# **CarND Kidnapped Vehicle** 

---

**CarND Kidnapped Vehicle Project**

The goals / steps of this project are the following:
* Implement 2 dimensional particle filter in C++ to find the kidnapped car location
* Test on simulator that the filter successfully gets car location
* Summarize the results with a written report


[//]: # (Image References)

[image1]: ./kidnappedcar.jpg "Result"


---
### Files Submitted 

The following files were changed during the implementation:
* particle_filter.cpp contains filter initialization, prediction, weight updates and particle resampling


#### 1. Project Basics

In this project the C++ was used to program. According the project the car was kidnapped and transported to a new location but it has a map of this location, a noisy GPS estimates of its initial location and lots of noisy sensor and control data. The code were tested using Udacities simulator.



#### 3. Accuracy

Accuracy was tested using Udacity simulator

![alt text][image1]

