//
//  file:       preprocessing.cpp
//
//  project:    Multiple Wave Tracking
//
//  contents:   Defintions of the frame preprocessing functions.  Associated
//              header file is preprocessing.hpp.  Preprocessing downsizes
//              full frames, applys Mixture of Gaussians mask, and denoises with
//              morphological operators.
//
//  use:        see readme.txt
//
//  author:     Created by Justin Fung on 9/1/17.
//
//  copyright:  © 2017 Justin Fung. All rights reserved.
//

#include "preprocessing.hpp"

#include "opencv2/bgsegm.hpp"


// ---INTERNAL LINKAGE---
namespace {

// Resizing input constants:
int kAnalysisWidth = 320;
int kAnalysisHeight = 180;

// Background Subtractor constants:
const int kMogHistory = 300;
const int kNumMixtures = 5;
const double kBgRatio = 0.7;
const double kNoiseSig = 0.0;

// Morphological Operator constants:
const int kKernelSize = 5;

}   // namespace


// ---EXTERNAL LINKAGE---
namespace preprocessing {
    
// Args:
//   dec_pBS: a reference to a declared Pointer-to-a-BackgroundSubtractor object
//   dec_denoising_kernel: a reference to a declared denoising kernel matrix
// Operation:
//   Initializes BackgroundSubtractor object to a Mixture of Gaussians model.
//   Initializes the denoising kernel to a square shape of size kKernelSize.
void InitializePreprocessing(cv::Ptr<cv::BackgroundSubtractor>& dec_pBS,
                             cv::Mat& dec_denoising_kernel)
{
    // Set the Background Subtractor object to a Mixture of Gaussains.
    dec_pBS = cv::bgsegm::createBackgroundSubtractorMOG(kMogHistory,
                                                        kNumMixtures,
                                                        kBgRatio, kNoiseSig);
    
    // Create a 5x5 square structuring element kernel for morphological operations.
    dec_denoising_kernel = cv::getStructuringElement(cv::MORPH_RECT,
                                            cv::Size(kKernelSize, kKernelSize));
}

// Args:
//   frame: a reference to an input frame as an opencv matrix
//   binary_image: a reference to an initialized binary_image container
//   init_pBS: a reference to an initialized Pointer-to-a-BS object
//   init_denoising_kernel: a reference to an initialized denoising kernel
// Operation:
//   Downsizes the frame to a temporary frame, on which the BS object is
//   applied. Output is returned to binary_image matrix, and morphological
//   operations are applied.
void Preprocess(const cv::Mat& frame,
                cv::Mat& binary_image,
                cv::Ptr<cv::BackgroundSubtractor>& init_pBS,
                const cv::Mat& init_denoising_kernel)
{
    cv::Mat resized_frame;
    
    // Resize input frames here using OpenCV function 'resize'.
    cv::resize(frame, resized_frame, cv::Size(kAnalysisWidth, kAnalysisHeight),
               0, 0, cv::INTER_LINEAR);
    
    // Background Modeling:  Apply MOG mask to the frame.
    init_pBS->apply(resized_frame, binary_image);
    
    // Apply morphological operations
    cv::morphologyEx(binary_image, binary_image, cv::MORPH_OPEN,
                     init_denoising_kernel);
}

} // namespace preprocessing
