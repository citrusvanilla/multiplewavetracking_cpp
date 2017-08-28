//
//  file:       detection.cpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Defintions of the Wave detection functions.  Associated header
//              file is detection.hpp.  Detection routine search for contours,
//              filters contours, and returns Wave objects.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#include "detection.hpp"


// ---INTERNAL LINKAGE---
namespace {

// Minimum area threshold for contour detection.
const int kMinArea = 100;

// Inertia thresholds for contour detection.
const double kMinInertiaRatio = 0.0;
const double kMaxInertiaRatio = 0.1;

// Args:
//   contour: a const reference to a contour,
//   min_area: a minimum area threshold
//   min_inertia_ratio: a minimum inertia ratio
//   max_inertia_ratio: and a maximum inertia ratio
// Operation:
//   Returns true if contour meets threshold requirements, and false if it does
//   not.  "Inertia" measures the oblong shape of a contour.  In our case,
//   we are looking for long and narrow contours.
bool KeepContour(const std::vector<cv::Point>& contour, int min_area,
                 double min_inertia_ratio, double max_inertia_ratio)
{
    bool ret = true;
    
    // Get the moments.
    cv::Moments moms = moments(cv::Mat(contour));
    
    // Filter by area:
    if (ret == true)
    {
        double area = moms.m00;
        if (area < kMinArea)
            ret = false;
    }
    
    // Filter by inertia:
    if (ret == true)
    {
        double denom = std::sqrt(pow(2*moms.mu11, 2) +
                                 pow(moms.mu20 - moms.mu02, 2));
        const double eps = 1e-2;
        double ratio = 0.0;
        
        if (denom > eps)
        {
            double cosmin = (moms.mu20 - moms.mu02) / denom;
            double sinmin = 2 * moms.mu11 / denom;
            double cosmax = -cosmin;
            double sinmax = -sinmin;
            
            double imin = 0.5*(moms.mu20 + moms.mu02) -
                          0.5*(moms.mu20 - moms.mu02)*cosmin -
                          moms.mu11*sinmin;
            double imax = 0.5*(moms.mu20 + moms.mu02) -
                          0.5*(moms.mu20 - moms.mu02)*cosmax -
                          moms.mu11*sinmax;
            
            ratio = imin / imax;
        } else {
            ratio = 1;
        }
        
        if (ratio < kMinInertiaRatio || ratio >= kMaxInertiaRatio)
            ret = false;
    }
    return ret;
}

}   // namespace


// ---EXTERNAL LINKAGE---
namespace detection {
    
// Args:
//   contours: a reference to a vector of opencv contours
//   waves: a reference to a vector of Wave objects
//   frame_number: an integer representing the frame in a video sequence
// Operation:
//   Filters the contours, converts accepted contours, and appends them to the
//   vector of Waves.  Destroys unacceptable contours.
void FilterAndConvert(std::vector<std::vector<cv::Point> >& contours,
                      std::vector<wave_obj::Wave>& waves, int frame_number)
{    
    // Init a counter for iterator.
    std::vector<std::vector<cv::Point> >::size_type i =0;
    
    // Filter the contours and make vector of Wave objects
    while (i != contours.size()){
        if (KeepContour(contours[i], kMinArea, kMinInertiaRatio,
                        kMaxInertiaRatio))
        {
            waves.push_back(wave_obj::Wave(contours[i], frame_number));
            ++i;
        } else {
            contours.erase(contours.begin() + i);
        }
    }
}

// Args::
//   contours: a reference to empty vector of contours
//   binary_img: a const reference to a binary image.
// Operation:
//   Appends found contours in the binary image to the vector of contours.
void FindContoursBasic(std::vector<std::vector<cv::Point> >& contours,
                       const cv::Mat& binary_img)
{
    // Return detected shapes to contours container.
    cv::findContours(binary_img, contours, cv::RETR_LIST, CV_CHAIN_APPROX_NONE);
}

// Args:
//   binary_image: a const reference to a binary image
//   frame_number: a frame number as an int
// Operations:
//   Returns a vector of Wave objects.
std::vector<wave_obj::Wave> DetectSections(const cv::Mat& binary_image,
                                           int frame_number)
{
    // Init a vector that will hold OpenCV contour objects.
    std::vector<std::vector<cv::Point> > contours;
    
    // Find contours in binary image and return to contours.
    FindContoursBasic(contours, binary_image);
    
    // Init a vector that will hold sections.
    std::vector<wave_obj::Wave> sections;
    
    // Filter the contours, converting the ones we keep to sections.
    FilterAndConvert(contours, sections, frame_number);
    
    // Return the sections.
    return sections;
};
    
}  // namespace detection
