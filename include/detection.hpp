//
//  file:       detection.hpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Declaration of wave detection functions from preprocessed frames
//              of an OpenCV VideoWriter object.  Uses OpenCV3+ library.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#ifndef detection_hpp
#define detection_hpp

#include <stdio.h>
#include <vector>

#include "wave_objects.hpp"
#include "opencv2/opencv.hpp"

namespace detection {

// Accepts a binarized, preprocessed image from the preprocessing routine and
// detects waves.  Utilizes OpenCV functions for finding contours of a certain
// size and shape as defined by constants in associated source file, and creates
// wave objects from contours that meet the thresholds.  Returns a vector of
// these waves.  Wave objects are handled (destroyed) in the tracking routine.
// Detection has been measured to consume about 2% of CPU processing time in
// execution.
std::vector<wave_obj::Wave> DetectSections(const cv::Mat&, int);

} // namespace detection

#endif /* detection_hpp */
