//
//  file:       tracking.cpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Defintions of the Wave tracking functions.  Associated header
//              file is tracking.hpp.  Tracking routine defines a search region
//              of interest for a Wave object and identifies its representation
//              in future frames.  Updates Wave data as necessary.  Includes
//              several clean-up functions.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#include "tracking.hpp"


// ---INTERNAL LINKAGE---
namespace {
    
// Args:
//   a: const ref to a wave
//   b: const ref to a different wave
// Operation:
//   Comparison function for sorting waves by descending birth.
// TODO:
//   This could be made a lambda in a std::sort().
bool CompareAgeDesc(const wave_obj::Wave& a, const wave_obj::Wave& b)
{
    return a.birth_ >= b.birth_;
}

// Args:
//   a: const ref to a wave
//   b: const ref to a different wave
// Operation:
//   Comparison function for sorting waves by ascending birth.
// TODO:
//   This could be made a lambda in a std::sort().
bool CompareAgeAsc(const wave_obj::Wave& a, const wave_obj::Wave& b)
{
    return a.birth_ <= b.birth_;
}
    
// Args:
//   wave: a const reference to a Wave object
//   waves: a const reference to a vector of Wave objects
// Operation:
//   Checks to see if the search regoin of interest of the input wave overlaps
//   with the search ROI of any wave in the vector of waves.
bool WillBeMerged(const wave_obj::Wave& wave,
                  const std::vector<wave_obj::Wave>& waves)
{
    // Using the wave's center-of-mass y coordinate and major axis angle,
    // find the wave's project on the y-axis.  Set to 'left_y'.
    int delta_y_left = wave.centroid_.x *
                       tan(wave.axis_angle_*3.14159265/180.0);
    int left_y = wave.centroid_.y + delta_y_left;
        
    // For every wave in the vector of waves, check if left_y falls inside
    // a wave's search ROI.  If yes, set return to true and break.
    for (std::vector<wave_obj::Wave>::size_type i = 0; i != waves.size(); ++i)
    {
        if (left_y >= waves[i].searchroi_coors_[0][0].y &&
            left_y <= waves[i].searchroi_coors_[0][3].y)
            return true;
    }
    return false;
}

}  // namespace


// ---EXTERNAL LINKAGE---
namespace tracking {

// Args:
//   sections: a reference to a vector of Wave objects
//   frame: a const reference to a binary image
//   frame_number: a frame number as an integer
//   number_of_frames: number of frames in the video sequence as an integer
// Operation:
//   Tracks a wave through a sequence of frames up calling all Wave member
//   functions that begin with "update*".  Automatically "kills" waves if the
//   analysis frame is the last frame in the sequence.
void TrackWaves(std::vector<wave_obj::Wave>& sections, const cv::Mat& frame,
                int frame_number, int number_of_frames)
{
    for (std::vector<wave_obj::Wave>::size_type i = 0; i != sections.size(); ++i)
    {
        // Update the ROI for finding wave's points in the frame.
        sections[i].update_searchroi_coors();

        // Find all the points in the ROI.
        sections[i].update_points(frame);
        
        // Check if the wave is "dead" (i.e. no points found).
        sections[i].update_death(frame_number);
        
        // If we are in the last frame, we kill all the waves prematurely.
        if (frame_number == number_of_frames)
            sections[i].death_ = frame_number;
        
        // Update the center of mass of the wave.
        sections[i].update_centroid();
        
        // Update the bounding box of the wave for display purposes.
        sections[i].update_boundingbox_coors();
        
        // Update instantaneous displacement of the wave, max displacement of
        // the wave, and the deque of displacement history of the wave.
        sections[i].update_displacement();
        
        // Update instantaneous mass of the wave, max mass of the wave.
        sections[i].update_mass();
        
        // Check the wave dynamics to see if the wave has become "recognized".
        // In this case we check max_mass and max_displacement.
        sections[i].update_recognized();
    }    
}

// Args:
//   tracked_waves: a reference to a vector of tracked waves
//   recognized_waves: a reference to a vector of dead but recognized waves
// Operation:
//   Checks to see if waves are dead and removes them from tracked_waves if so.
//   Moves recognized waves to recognized_waves, and destroys unrecognized
//   waves.
void RemoveDeadWaves(std::vector<wave_obj::Wave>& tracked_waves,
                     std::vector<wave_obj::Wave>& recognized_waves)
{
    std::vector<wave_obj::Wave>::size_type i = 0;
    
    while (i != tracked_waves.size())
    {
        if (tracked_waves[i].death_ != -1)
        {
            if (tracked_waves[i].recognized_ == true)
                recognized_waves.push_back(tracked_waves[i]);
            
            tracked_waves.erase(tracked_waves.begin() + i);
        } else {
            ++i;
        }
    }
}

// Args:
//   waves: a reference to a vector of waves
// Operation:
//   Checks each wave in the vector, starting with the oldest first, and
//   destroys it if it is determined to be a duplicate of an existing wave in
//   the vector.
// TODO(@citrusvanilla):
//   Double sort is probably unncessary here.
void RemoveDuplicateWaves(std::vector<wave_obj::Wave>& waves)
{
    // Sort waves by descending birth.
    std::sort(waves.begin(), waves.end(), CompareAgeDesc);
    
    // Remove duplicates.
    std::vector<wave_obj::Wave>::size_type i = 0;
    std::vector<wave_obj::Wave>::size_type j = 0;
    
    while (i != waves.size())
    {
        int delta_y_left = waves[i].centroid_.x *
                           tan(waves[i].axis_angle_*3.14159265/180.0);
        int left_y = waves[i].centroid_.y + delta_y_left;
        j = i+1;
        
        while (j != waves.size())
        {
            // check to see if waves[i] is in waves[j] shadow,
            // and erase if it it is.
            if (left_y >= waves[j].searchroi_coors_[0][0].y &&
                left_y <= waves[j].searchroi_coors_[0][3].y)
            {
                waves.erase(waves.begin() + i);
                break;
            } else {
                ++j;
            }
        }
        if (j == waves.size()) {++i;}
    }
    // Resort the waves by ascending birth.
    std::sort(waves.begin(), waves.end(), CompareAgeAsc);
};

// Args:
//   sections: a const reference to a vector of Wave objects
//   tracked_waves: a reference to a vector of waves that are being tracked.
// Operation:
//   Checks to see if each wave in sections may be a section of an existing wave
//   in tracked_waves.  If so, we ignore it (to be destroyed later).  If it is
//   a new wave, we add it to the list of waves to be tracked.
void AddNewSectionsToTrackedWaves(const std::vector<wave_obj::Wave>& sections,
                                  std::vector<wave_obj::Wave>& tracked_waves)
{
    for (std::vector<wave_obj::Wave>::size_type i = 0; i != sections.size(); ++i)
    {
        if (!WillBeMerged(sections[i], tracked_waves))
            tracked_waves.push_back(sections[i]);
    }
}

}   // namespace tracking

