//
//  file:       wave_objects.hpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Declaration of the Wave class and associated data members and
//              member functions.  Uses OpenCV3+ library.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#ifndef wave_objects_hpp
#define wave_objects_hpp

#include <vector>
#include "opencv2/opencv.hpp"

namespace wave_obj {

// Wave object is initiated with the following data members and contruction
// methods.  Waves are meant to be tracked through frames (See: tracking.cpp)
// and all methods prepended with 'update' are intended to be called in
// successive frames.  Methods prepended with 'set' are meant to be called
// during construction only.
class Wave {
  public:
    Wave(std::vector<cv::Point> contour, int frame_number);
    
    // ---  DATA MEMBERS ---
    
    // Name of the wave.
    int name_;
    
    // Frame of birth of the wave.
    int birth_;
    
    // Ex-ante angle of major axis of wave.
    double axis_angle_;
    
    // Center of mass of the wave in (x,y) coordinates.
    cv::Point centroid_;
    
    // Deque of centroids.
    std::deque<cv::Point> centroid_vec_;
    
    // Coordinates of polygon bounding a search ROI.
    std::vector<std::vector<cv::Point> > searchroi_coors_;
    
    // Equation of the wave's original axis in standard form.
    double original_axis_[3];
    
    // Coordinates of polygon bounding the wave points.
    cv::Mat boundingbox_coors_;
    
    // Instantaneous displacement of wave relative to its origin.
    int displacement_;
    
    // Maximum displacement of the wave through its existance.
    int max_displacement_;
    
    // Deque of displacements of the wave over time.
    std::deque<int> displacement_vec_;
    
    // Instantaneous mass of wave.
    int mass_;
    
    // Maximum mass of wave through its existance.
    int max_mass_;
    
    // Whether or not the wave is recognized as an actual wave.
    bool recognized_;
    
    // Frame of Death of the wave (-1 if still alive).
    int death_;
    
    
    // --- WAVE METHODS ---
    
    // Updates the search region of interest in which a wave will identify its
    // representation in successive frames by using its current center-of-mass
    // estimate and a user-defined constant kSearchRegionBuffer.
    void update_searchroi_coors();
    
    // Determines if a wave has disappeared from a video, in which case we would
    // no longer want to track it.
    void update_death(int frame_number);
    
    // The main representation of a wave is in the points_ attribute. This
    // function updates the points_ in current frame by masking the frame
    // according to the searchroi_coors_ attribute and measuring the result.
    void update_points(const cv::Mat& frame);
    
    // Centroid represents the center-of-mass of the wave's representation.
    // Updates this attribute by calculating the center-of-mass using
    // first-order moments. Updates the deque of centroids for temporal tracking
    // as well.  Centroid is used to calculate displacement of the wave.
    void update_centroid();
    
    // Bounding Box Coordinates bound the representation of the wave in a
    // quadrangle shape.  This is solely for display purposes if the user is
    // outputing a video with wave detection/tracking overlaid on the source
    // video.
    void update_boundingbox_coors();
    
    // Displacement is one of two wave dynamics used to determine if the wave is
    // a positive instance of a wave.  This measures the distance in pixels of
    // the displacement of the wave orthogonal to its original major axis.
    // Updates max_displacement_ and the deque of displacements accordingly.
    void update_displacement();
    
    // Mass is one of two wave dynamics used to determine if the wave is a
    // positive instance of a wave.  This measures the mass of the
    // representation of the wave in pixels.  Updates max_mass accordingly.
    void update_mass();
    
    // Evaluates the two wave dynamics of mass and displacement if and only if
    // the wave is not already recognized.  This determines whether or not a
    // wave is actually a wave by user definition.
    void update_recognized();
    
  private:

    // Representation of the wave.
    std::vector<cv::Point> points_;
    
    // Sets the name of the wave using an integer.
    void set_wave_name();
    
    // Sets the standard line equation for the wave's original axis.  Standard
    // form of a line is denoted as Ax + By = C.  This is used to determine
    // orthogonal displacement of the wave overtime through the
    // update_displacement() function below.
    void set_original_axis();
};

}   // namespace wave_obj

#endif /* wave_objects_hpp */
