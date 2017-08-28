//
//  file:       preprocessing.hpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Declaration of preprocessing functions for frames of an OpenCV
//              VideoReader object.  Uses OpenCV3+ library.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#ifndef preprocessing_hpp
#define preprocessing_hpp

#include <stdio.h>
#include "opencv2/opencv.hpp"

namespace preprocessing {

// Initializes two objects needed for preprocessing frames of an OpenCV
// VideoReader object: 1. A background subtractor object for modeling scene
// background and extracting foreground, and 2. A kernel represented as a
// matrix for "denoising" the foreground.  Takes references to both objects and
// initializes according to constants defined in the accompanying source file.
void InitializePreprocessing(cv::Ptr<cv::BackgroundSubtractor>&,
                             cv::Mat&);

// Function for processing fullsize input frames, implementing the two objects
// described above in processing, and returning downsized binarized images to
// main for detection routine.  Meant to be called on every successive frame of
// an OpenCV VideoReader object.  The BG subtractor object is necessarily
// static.  This function has been measured to consume about 50% of CPU
// processing time in main, due to heavy requirements of the Mixture of Gaussian
// modeling.
void Preprocess(const cv::Mat&,
                cv::Mat&,
                cv::Ptr<cv::BackgroundSubtractor>&,
                const cv::Mat&);

}  // name space preprocessing

#endif /* preprocessing_hpp */
