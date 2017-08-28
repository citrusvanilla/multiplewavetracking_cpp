//
//  file:       wave_objects.cpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Definition and construction of the Wave object, and Wave
//              get/set methods.  Associated header file is wave_objects.hpp.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#include "wave_objects.hpp"


// ---INTERNAL LINKAGE---
namespace {

// Wave object constants.
const int kDisplacementThreshold = 10;
const int kMassThreshold = 1000;
const int kSearchRegionBuffer = 15;
const int kAnalysisFrameWidth = 320;
const int kAnalysisFrameHeight = 180;
const double kWaveAngle = 5.0;
const int kTrackingHistory = 20;

}   // namespace


// ---EXTERNAL LINKAGE---
namespace wave_obj {

// Object that represents a wave in a video frame.  Initiated from a
// filtered contour object (See: 'detection.cpp').
//
// Args:
//   contour: must be initialized with an opencv contour
//   frame_number: must be initialized with a frame number
// Example:
//   std::vector<wave_obj::Wave> vector_of_waves;
//   wave_obj::Wave one_wave(contour, frame_number);
//   vector_of_waves.push_back(one_wave);
Wave::Wave(std::vector<cv::Point> contour, int frame_number):
    name_(),
    points_(contour),
    birth_(frame_number),
    axis_angle_(kWaveAngle),
    centroid_(),
    centroid_vec_(),
    original_axis_(),
    searchroi_coors_(),
    boundingbox_coors_(),
    displacement_(0),
    max_displacement_(0),
    displacement_vec_(),
    mass_(),
    max_mass_(),
    recognized_(false),
    death_(-1)
{
    set_wave_name();
    update_centroid();
    set_original_axis();
    update_searchroi_coors();
    update_boundingbox_coors();
    update_mass();
};


//---METHODS (10)---

// Operation:
//   Sets the name of the wave using an incremented static int var.
void Wave::set_wave_name()
{
    static int id = 0;
    id += 1;
    name_ = id;
}

// Operation:
//   Sets the major axis of the wave from its origin in standard form.
void Wave::set_original_axis()
{
    original_axis_[0] = std::tan(-axis_angle_*3.14159265/180.0);
    original_axis_[1] = -1;
    original_axis_[2] = (centroid_.y -
                         std::tan(-axis_angle_*3.14159265/180.0)*centroid_.x);
}

// Operation:
//   Sets the four coordinates of the search ROI.
void Wave::update_searchroi_coors()
{
    searchroi_coors_.clear();
    
    // Get the left and right y-axis buffer region deltas.
    int delta_y_left = centroid_.x*std::tan(axis_angle_*3.14159265/180.0);
    int delta_y_right = (kAnalysisFrameWidth - centroid_.x) *
                         std::tan(axis_angle_*3.14159265/180.0);
    
    // These coordinates MUST be in order!
    cv::Point upper_left(0,int(centroid_.y + delta_y_left -
                               kSearchRegionBuffer));
    cv::Point upper_right(kAnalysisFrameWidth, int(centroid_.y - delta_y_right -
                                                   kSearchRegionBuffer));
    cv::Point lower_right(kAnalysisFrameWidth, int(centroid_.y - delta_y_right +
                                                   kSearchRegionBuffer));
    cv::Point lower_left(0, int(centroid_.y + delta_y_left +
                                kSearchRegionBuffer));

    // Push all the points into one vector representing the polygon of the ROI.
    std::vector<cv::Point> ROI = {upper_left, upper_right, lower_right,
                                  lower_left};

    // Update the search region coordinates.
    searchroi_coors_.push_back(ROI);
}

// Operation:
//   Sets the death to the current frame if the wave as disappeared from a frame.
void Wave::update_death(int frame_number)
{
    if (points_.empty())
        death_ = frame_number;
}

// Operation:
//   Updates points by masking input frame and measuring representaiton.
void Wave::update_points(const cv::Mat& frame)
{
    points_.clear();
    
    // Init empty images to hold our points, and the mask
    cv::Mat points_img = cv::Mat::zeros(kAnalysisFrameHeight,
                                        kAnalysisFrameWidth, CV_8UC1);
    cv::Mat mask_img = cv::Mat::zeros(kAnalysisFrameHeight,
                                      kAnalysisFrameWidth, CV_8UC1);
    
    // Fill the polygon in the mask of the search region.
    cv::fillPoly(mask_img, searchroi_coors_, cv::Scalar(255));
    
    // AND the binary image with the mask and store result into points image.
    cv::bitwise_and(frame, mask_img, points_img);
    
    // Our points are non-zero; store them back to the points attribute.
    cv::findNonZero(points_img, points_);
}

// Operation:
//   Sets center-of-mass coordinate using points_ attribute.
void Wave::update_centroid()
{
    centroid_ = {-1,-1};

    if (!points_.empty()){
        int sum_x = 0;
        int sum_y = 0;
        
        for (std::vector<cv::Point>::size_type i = 0; i != points_.size(); ++i)
        {
            sum_x += points_[i].x;
            sum_y += points_[i].y;
        }
        
        centroid_.x = int(sum_x / points_.size());
        centroid_.y = int(sum_y / points_.size());
    }
    
    // Update wave.centroid_vec.
    centroid_vec_.push_back(centroid_);
    
    // Pop and discard if the deque exceeds the TrackingHistory constant.
    if (centroid_vec_.size() > kTrackingHistory)
         centroid_vec_.erase(centroid_vec_.begin());
}

// Operation:
//   Sets the four coordinates of a polygon bounding a wave's points_.
void Wave::update_boundingbox_coors()
{
    if (!points_.empty())
    {
        // Calculate means.
        int sum_x = 0, sum_y = 0;
        
        for (std::vector<cv::Point>::size_type i = 0; i != points_.size(); ++i)
        {
            sum_x += points_[i].x;
            sum_y += points_[i].y;
        }
        
        double mean_x = sum_x / points_.size();
        double mean_y = sum_y / points_.size();
        
        // Calculate Standard Deviations.
        double e_x = 0, e_y = 0;

        for (std::vector<cv::Point>::size_type i = 0; i != points_.size(); ++i)
        {
            e_x += std::pow(points_[i].x - mean_x, 2);
            e_y += std::pow(points_[i].y - mean_y, 2);
        }
        double inv = 1.0 / static_cast<double>(points_.size());

        double std_x = std::sqrt(inv*e_x);
        double std_y = std::sqrt(inv*e_y);
        
        // Keep non-outliers (i.e. discard the outliers).
        std::vector<cv::Point> points_wo_outliers;
        
        for (std::vector<cv::Point>::size_type i = 0; i != points_.size(); ++i)
        {
            if (std::abs(points_[i].x - mean_x) <= 3*std_x &&
                std::abs(points_[i].y - mean_y) <= 3*std_y)
                points_wo_outliers.push_back(points_[i]);
        }
        
        // Finds the rectangle that encloses these points.
        cv::RotatedRect rect = cv::minAreaRect(points_wo_outliers);
        
        // Returns the four coordinates of this bounding rectangle.
        cv::boxPoints(rect, boundingbox_coors_);
    }
}

// Operation:
//   Updates displacement_, max_displacement_, and displacement_vec_.
void Wave::update_displacement()
{
    if (centroid_.x > -1 && centroid_.y > -1)
    {
        // Evaluate displacement from original axis.
        displacement_ = std::abs(original_axis_[0]*centroid_.x +
                                 original_axis_[1]*centroid_.y +
                                 original_axis_[2]) /
                       (std::sqrt(std::pow(original_axis_[0],2) +
                                  std::pow(original_axis_[1],2)));
    }
    
    // Update max displacement.
    if (displacement_ > max_displacement_)
        max_displacement_ = displacement_;
    
    // Update displacement vector.
    displacement_vec_.push_back(displacement_);
    
    // Pop and discard if the deque exceeds the TrackingHistory constant,
    if (displacement_vec_.size() > kTrackingHistory)
        displacement_vec_.erase(displacement_vec_.begin());
}

// Operation:
//   Updates mass_ and max_mass_ by evaluating points_.
void Wave::update_mass()
{
    // Update instantaneous mass.
    if (!points_.empty())
        mass_ = static_cast<int>(points_.size());
    else
        mass_ = 0;
    
    // Update maximum mass.
    if (mass_ > max_mass_)
        max_mass_ = mass_;
}

// Operation:
//   Updates recognized_ by evaluating max_displacement_ and max_mass_.
void Wave::update_recognized()
{
    if (!recognized_ &&
        max_displacement_ >= kDisplacementThreshold &&
        max_mass_ >= kMassThreshold)
        recognized_ = true;
};

}   //namespace wave_obj
