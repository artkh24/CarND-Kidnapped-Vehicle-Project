/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

default_random_engine gen;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
  
	num_particles=100;
    
   // Create a normal (Gaussian) distribution for x, y and theta.
	normal_distribution<double> dist_x(x, std[0]);
	normal_distribution<double> dist_y(y, std[1]);
	normal_distribution<double> dist_theta(theta, std[2]);
    
    //Initialize particles
    for (int i = 0; i < num_particles; ++i){
        Particle particle;
        particle.id=float(i);
        particle.x =dist_x(gen);
		particle.y =dist_y(gen);
		particle.theta =dist_theta(gen);
        particle.weight=1.0;
      
       particles.push_back(particle);
    }
    weights.resize(num_particles);
    is_initialized=true;
    
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
    
  
    // Create a normal (Gaussian) distribution for noise
	normal_distribution<double> dist_x(0, std_pos[0]);
	normal_distribution<double> dist_y(0, std_pos[1]);
	normal_distribution<double> dist_theta(0, std_pos[2]);
    
    for (int i = 0; i < num_particles; ++i){
      if(fabs(yaw_rate)<0.00001){
      	particles[i].x+=velocity*delta_t*cos(particles[i].theta)+dist_x(gen);
      	particles[i].y += velocity*delta_t*sin(particles[i].theta)+dist_y(gen);
      } 
      else {
      	particles[i].x+=(velocity / yaw_rate) * (sin(particles[i].theta + yaw_rate*delta_t) - sin(particles[i].theta))+dist_x(gen);
     	particles[i].y += (velocity / yaw_rate) * (cos(particles[i].theta) - cos(particles[i].theta + yaw_rate*delta_t))+dist_y(gen);
      	particles[i].theta += yaw_rate * delta_t+dist_theta(gen);  
      }
    }

}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
	
    for (unsigned int i=0; i < observations.size(); i++){
      
      //Get current observation
      LandmarkObs observation = observations[i];
      
      double minDistance=numeric_limits<double>::max();
      int map=-1;
      for (unsigned int j=0; j<predicted.size(); j++){
        //Get current prediction
        LandmarkObs prediction = predicted[j];
        
        double distance=dist(observation.x,observation.y,prediction.x,prediction.y);
        if(distance<minDistance){
           minDistance=distance;
           map=prediction.id;
        }
      }
      observations[i].id=map;
    }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
  
  
    double sig_x = std_landmark[0];
    double sig_y = std_landmark[1];
   // calculate normalization term
   double gauss_norm= 1.0/(2.0 * M_PI * sig_x * sig_y);
  
    for (int i=0; i < num_particles; i++){
      
      //Get current particle coordinates
      double particle_x = particles[i].x;
      double particle_y = particles[i].y;
      double particle_theta = particles[i].theta;
      
      //Make a vector to get the landmarks within sensor range
      vector<LandmarkObs> landmarksInRange;
      
      //Iterate through landmarks
      for (unsigned int j=0; j<map_landmarks.landmark_list.size(); j++){
        
        //Get current landmark coordinates
        float x_f = map_landmarks.landmark_list[j].x_f;
        float y_f = map_landmarks.landmark_list[j].y_f;
        int id_i = map_landmarks.landmark_list[j].id_i;
        
        //Check if landmark is in sensor range
        if(dist(particle_x,particle_y,x_f,y_f)<=sensor_range){
          
          //Push to in range landmarks
           landmarksInRange.push_back(LandmarkObs{id_i,x_f,y_f});
        } 
      }
	
      //Transform observations from car coordinate system to map coordinate system
      vector<LandmarkObs> tr_observations;
      for (unsigned int j=0; j<observations.size(); j++){
        double x_m=particle_x+cos(particle_theta)*observations[j].x-sin(particle_theta)*observations[j].y;
        double y_m=particle_y+sin(particle_theta)*observations[j].x+cos(particle_theta)*observations[j].y;
        tr_observations.push_back(LandmarkObs{observations[j].id, x_m, y_m});
      }
      
      //Accociate observations with landmarks in range
      dataAssociation(landmarksInRange, tr_observations);
      
      // weight reset
      particles[i].weight=1.0;
      
      //Weight calculation
      
      for(unsigned int j=0; j<tr_observations.size(); j++){
        
        double x_obs=tr_observations[j].x;
        double y_obs=tr_observations[j].y;
        int prediction_id=tr_observations[j].id;
        
        double mu_x, mu_y;
        
        unsigned int k=0;
        while(k<landmarksInRange.size()){
          if(prediction_id==landmarksInRange[k].id){
            mu_x=landmarksInRange[k].x;
            mu_y=landmarksInRange[k].y;
            break;
          }
          k++;
        }
       
        
        // calculate exponent
        double exponent= pow(x_obs - mu_x,2)/(2 * pow(sig_x,2)) + pow(y_obs - mu_y,2)/(2 * pow(sig_y,2));
        particles[i].weight*=(gauss_norm*exp(-exponent));
        
      }
      
      weights[i]=particles[i].weight;
    }
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
  
    discrete_distribution<> dist(weights.begin(),weights.end());
    vector<Particle> resampled;
    for (int i=0; i<num_particles; i++){
      resampled.push_back(particles[dist(gen)]);
    }
	particles=resampled;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates
    
    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
    
    return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
