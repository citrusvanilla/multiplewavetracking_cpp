//
//  file:       main.hpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Main implementation of the Multiple Wave Tracking program.
//              Uses OpenCV3+ library.  Implements preprocessing, detection,
//              and tracking functions, as well as input and output handling.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <time.h>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"

#include "preprocessing.hpp"
#include "detection.hpp"
#include "wave_objects.hpp"
#include "tracking.hpp"

using namespace std::chrono;


// Declare file name of the video.
const std::string kInputVidName = "tstreet.mp4";
const std::string kOutputVidName = "output.mp4";

// Set output frame sizes
const int kOutputWidth = 320;
const int kOutputHeight = 180;


// Simple log for output.
// Args:
//   begin_time: time in milliseconds
//   end_time: time in milliseconds
//   rec_waves: a vector of Wave objects
//   num_frames: number of frames in a video sequence
// Opertion:
//   Simple log to report to stdio of program performance and waves identified.
void WriteLog(high_resolution_clock::time_point begin_time,
              high_resolution_clock::time_point end_time,
              std::vector<wave_obj::Wave> waves,
              int num_frames)
{
    std::cout << "------------" << std::endl;
    std::cout << "Program complete." << std::endl;
    std::cout << "Program took "
              << duration_cast<milliseconds>(end_time-begin_time).count()
              << " milliseconds." << std::endl;
    std::cout << "Program speed: " << static_cast<double>(num_frames) /
                        duration_cast<seconds>(end_time-begin_time).count()
              << " frames per second." << std::endl;
    std::cout << waves.size() << " wave(s) found." << std::endl;
    std::cout << "------------" << std::endl;
    //std::cout << "DEBUGGING" << std::endl;
    //std::cout << "Time spent in reading frames: " << sec << " milliseconds."
    //          << std::endl;
}


// Simple Debugger that outputs tracked wave statistics.
// Args:
//   waves: a vector of Wave obejcts
// Opertion:
//   Displays tracked waves statistics to stdio.  Useful for monitoring
//   behavior of the detection and tracking routines.
void WaveDebugger(std::vector<wave_obj::Wave> waves)
{
    std::cout << "Tracking " << waves.size() << " waves..." << std::endl;

    for (std::vector<wave_obj::Wave>::size_type i = 0; i != waves.size(); ++i)
    {
        std::cout << "id: " << waves[i].name_ << std::endl;
        //cout << "original axis: " << waves[i].original_axis[0]
        // << ", " << waves[i].original_axis[1]
        // << ", " << waves[i].original_axis[2] << endl;
        //cout << "centroid: " << waves[i].centroid << endl;
        std::cout << "centroid deque size: "
                  << waves[i].centroid_vec_.size() << std::endl;
        //cout << "disp: " << waves[i].displacement << endl;
        std::cout << "max_disp: " << waves[i].max_displacement_ << std::endl;
        std::cout << "mass: " << waves[i].mass_ << std::endl;
        std::cout << "max_mass: " << waves[i].max_mass_ << std::endl;
        std::cout << "recognized: " << waves[i].recognized_ << std::endl;
        std::cout << "death: " << waves[i].death_ << std::endl;
    }
}


// Simple status update to stdio.
// Args:
//   frame_num: frame being analyzed
//   tot_frames: total number of frames in the sequence
// Operation:
//   Outputs status to stdio based on frame count.
void status_update(int frame_num, int tot_frames,
                   high_resolution_clock::time_point begin_time,
                   high_resolution_clock::time_point curr_time)
{
    if (frame_num == 1)
        std::cout << "Starting analysis of " << tot_frames <<
        " frames." << std::endl;
    
    else if (frame_num % 100 == 0)
        std::cout << std::setprecision(3) << frame_num << " frames complete. ("
        << 1000 * static_cast<double>(frame_num) /
        duration_cast<milliseconds>(curr_time-begin_time).count() <<
        " frames/sec; " <<
        duration_cast<milliseconds>(curr_time-begin_time).count() / 1000 /
        static_cast<double>(frame_num) << " sec/frame)" <<
        std::endl;
    else if (frame_num == tot_frames)
        std::cout << "End of video reached successfully." << std::endl;
}


int main(int argc, const char** argv)
{
    // ---INPUT---
    // Init OpenCV VideoCapture object and check for errors.
    cv::VideoCapture cap(kInputVidName);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream or file." << std::endl;
        return -1;
    }
    int number_of_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);

    // ---OUTPUT---
    // Initialize a VideoWriter object and check for success.
    int fourcc = CV_FOURCC('M','P','4','V');
    double fps = cap.get(cv::CAP_PROP_FPS);
    cv::Size S = cv::Size(kOutputWidth, kOutputHeight);
    bool is_color = false;
    cv::VideoWriter writer;
    writer = cv::VideoWriter(kOutputVidName, fourcc, fps, S, is_color);
    if (!writer.isOpened()) {
        std::cerr << "Could not open the output video file for write\n";
        return -1;
    }

    // ---PREPROCESSING---
    // Init Background subtractor and morphological kernel objects.
    cv::Ptr<cv::BackgroundSubtractor> pMOG;
    cv::Mat morphological_kernel;
    preprocessing::InitializePreprocessing(pMOG, morphological_kernel);

    // ---ANALYSIS---
    // Init OpenCV frame, binary_image, and vector of Waves objects.
    cv::Mat frame;
    cv::Mat binary_image;
    std::vector<wave_obj::Wave> tracked_waves;
    std::vector<wave_obj::Wave> recognized_waves;

    // Init a frame number counter.
    int frame_number = 1;

    // Init a timer for program performance.
    auto t1 = high_resolution_clock::now();

    while(true)
    {
        // Read into frame and check for error.
        cap >> frame;
        if (frame.empty()) {break;}
        
        // Provide status update to stdio.
        status_update(frame_number, number_of_frames,
                      t1, high_resolution_clock::now());

        // ---PREPROCESS---
        preprocessing::Preprocess(frame, binary_image, pMOG,
                                  morphological_kernel);

        // ---DETECTION---
        std::vector<wave_obj::Wave> tmp_sections = detection::DetectSections(
                binary_image, frame_number);

        // ---TRACKING---
        tracking::TrackWaves(tracked_waves, binary_image, frame_number,
                             number_of_frames);
        tracking::RemoveDeadWaves(tracked_waves, recognized_waves);
        tracking::RemoveDuplicateWaves(tracked_waves);
        if (frame_number < number_of_frames)
            tracking::AddNewSectionsToTrackedWaves(tmp_sections, tracked_waves);

        // ---DEBUG---
        // WaveDebugger(tracked_waves);

        // Display the resulting binary mask.
        // imshow ("Frame", binary_image);

        // Write to output
        // writer.write(binary_image);

        // User event: Exit loop with ESC.
        // char c = (char)waitKey(1);
        // if (c==27) {break;}

        ++frame_number;
    }

    // Stop timer and write simple log to stdio.
    auto t2 = high_resolution_clock::now();
    WriteLog(t1, t2, recognized_waves, number_of_frames);

    // When main loop is complete, release video resource.
    cap.release();

    // Closes all the display frames.
    // cv::destroyAllWindows();

    return 0;
}
