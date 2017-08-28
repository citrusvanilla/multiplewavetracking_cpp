//
//  file:       tracking.hpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Declaration of wave tracking functions from preprocessed frames
//              of an OpenCV VideoWriter object.  Uses OpenCV3+ library.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#ifndef tracking_hpp
#define tracking_hpp

#include <stdio.h>
#include <vector>

#include "opencv2/opencv.hpp"
#include "wave_objects.hpp"


namespace tracking {

// Main function for tracking Wave objects through successive, preprocessed
// frames of an OpenCV VideoReader object.  Calls all member functions of the
// Wave object that begin with 'update*'.  These member functions define a new
// region of interest in which to search for the wave's representation in
// successive frames, identifies this representation, and updates its data
// accordingly.  TrackWaves and associated functions below have been measured
// to consume about 4% of CPU processing time in execution.
void TrackWaves(std::vector<wave_obj::Wave>&,
                const cv::Mat&,
                int, int);

// Identifies when a wave no longer exists by checking its .death_ member, and
// either destroys the object if it never became recognized, or moves it to a
// separate vector that holds the final representations of waves that became
// recognized in the tracking routine.
void RemoveDeadWaves(std::vector<wave_obj::Wave>&,
                     std::vector<wave_obj::Wave>&);

// Physical waves in the real world may have several different 'sections' that
// are actually part of one wave.  This function checks to see if main() is
// tracking separate wave objects that actually represent the same wave. If the
// function finds this to be the case, it destroys the younger object, keeping
// the oldest wave object for further tracking.
void RemoveDuplicateWaves(std::vector<wave_obj::Wave>&);

// This function determines when a new wave object has entered the scene that
// is not actually a wave that is already being tracked.  It takes the output
// of the detection routine and checks each wave's data to see if it is already
// being tracked.  If it is not, it adds the Wave object to the vector of waves
// that are to be tracked by the TrackWaves() function above.  Waves that are
// already being tracked are destroyed in successive calls to DetectSections().
void AddNewSectionsToTrackedWaves(const std::vector<wave_obj::Wave>&,
                                  std::vector<wave_obj::Wave>&);

}   // namespace tracking

#endif /* tracking_hpp */
