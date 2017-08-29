# Vision-Based Near-Shore Wave Tracking and Recognition for High Elevation and Aerial Video Cameras, using C++ and OpenCV

<!---
![Surfer Detection Example](http://i.imgur.com/QaWJIU3.jpg?1)
--->
This repository contains a program for modeling, detecting, tracking, and recognizing near-shore ocean waves, 
written in C++ with use of OpenCV 3+ library.

The program is demoed on scenes from several Southern California locations in the video here.


### Software and Library Requirements
* OpenCV 3.2.0
* a C++ compiler
* CMake 3.8.1 or higher if you are generating build files with the CMakeLists.txt script.


## Goals
This program implements a common Computer Vision "recognition" workflow for videos 
through application to near-shore ocean wave recognition, and is fast enough to run in realtime.

Wave recognition has uses in higher-level objectives such as automatic wave period, frequency, and size determination, as well as region-of-interest definition for human activity recognition.


## Key Processes
The general vision-based workflow proceeds from detection, to tracking, and then recognition<sup>[1](#myfootnote1)</sup>.
The process in this program is thus:
1. Preprocessing of video: background modeling of the maritime environment and foreground extraction of near-shore waves.
2. Detection of objects: Identification and localization of waves in the scene.
3. Tracking of objects: Identity-preserving localization of detected waves through successive video frames for the capture of wave dynamics.
4. Recognition of objects: Classification of waves based on their temporal dynamics.


## Program Architecture
In accordance with the general comupter vision recognition workflow for objects in videos, the program is split into four modules (preprocessing, detection, tracking, and the 'Wave' class) in addition to main().  Module functions are declared and described in their header files (\*.hpp), with implementation and usage in the associated source files (\*.cpp).


## Code Organization
File | Purpose
------------ | -------------
include/wave_objects.hpp |	Declaration of the Wave class and associated data members and member functions.
include/preprocessing.hpp |	Declaration of preprocessing functions for frames of an OpenCV VideoReader object.
include/detection.hpp |	Declaration of wave detection functions from preprocessed frames of an OpenCV VideoWriter object.
include/tracking.hpp | Declaration of wave tracking functions from preprocessed frames of an OpenCV VideoWriter object.
src/wave_objects.cpp |	Definition and construction of the Wave object, and Wave get/set methods.
src/preprocessing.cpp |	Defintions of the frame preprocessing functions. Preprocessing downsizes full frames, applys Mixture of Gaussians mask, and denoises with morphological operators.
src/detection.cpp	| Defintions of the Wave detection functions. Detection routine search for contours, filters contours, and returns Wave objects.
src/tracking.cpp |	Defintions of the Wave tracking functions. Tracking routine defines a search region of interest for a Wave object and identifies its representation in future frames.  Updates Wave data as necessary.  Includes several clean-up functions.
main.cpp |	Main implementation of the Multiple Wave Tracking program. Implements preprocessing, detection, and tracking functions, as well as input and output handling.
scenes/ | A directory of sample videos for the Multiple Wave Tracking program.
CMakeLists.txt | Helper CMake script to generate build files for compilation.


## Multiple Wave Tracking Model

Main() implements the recognition workflow above. The following briefly outlines model choices.  Details can be found in ["Model Details"](##ModelDetails) below.

* **Preprocessing**: Input frames are downsized by a factor of four for analysis.  Background modeling is performed using a Mixture-of-Gaussians model with 5 Gaussians per pixels and a background history of 300 frames, resulting in a binary image in which background is represented by values of 255 and foreground as 0.  A square denoising kernel of 5x5 pixels is applied pixel-wise to the binary image to remove features that are too small to be considered.
* **Detection**: Contour-finding is applied to the denoised image to identify all forground objects.  These contours are filtered for both area and shape using contour moments, resulting in the positive identification of large, oblong shapes in the scene.  These contours are converted to Wave objects and passed to the tracking routine.
* **Tracking**: A region-of-interest is defined for each wave in which we expect the wave to exist in successive frames.  The wave's representation is captured and its dynamics are calculated.  We use two dynamics to determine whether or not the tracked object is indeed a wave: mass and displacement.  Mass is calculated by weighting pixels equally and performing a simple count.  Displacement is measured by calculating the orthogonal displacement of the wave's center-of-mass relative to its original major axis.
* **Recognition**: We accept an object as a wave if its mass and orthogonal displacement exceed user-defined thresholds.  In this manner, we can eliminate objects that would otherwise be identified as waves in static frames.


## Data and Assumptions

In order to use tracking inference in the classification of waves, we must use static postion videos as input to the program.  Included in the scene directory are three videos from different scenes that can be used to test the Multiple Wave Tracking program.  These videos are 
1280 x 720 pixels and encoded with the mp4 codec.  Please note that if you use your own videos, you may have to re-encode your videos
to play nice with the OpenCV library.  A common tool for handling video codecs is the [FFMEG library](https://www.ffmpeg.org/).

As a vision-based project, this program performs best on scenes in which the object of interest (the wave) is sufficiently separated from other objects (i.e. there is no occlusion or superimposition).  This assumption is fair for the wave object as ocean physics dictate that near-shore waves generally have consistent periods of separation, from the processes of assimilation, imposition, and interference that take place a great distance from shore.

The inherent assumption in the program of delineated waves is best achieved with high elevation or aerial cameras.


## Compiling and Launching the Model

The source files for this project must be compiled prior to execution.  A script is provided for generating build files using CMake as CMakeLists.txt, though it is not necessary to compile with this method.  The dependency for compiling the Multiple Wave Tracking program is the OpenCV library, which can be obtained [here](http://opencv.org/releases.html).  This project uses OpenCV version 3.2.0.  You will need to place the OpenCV header and library directories in your compiler's search path.  You may reference [this documentation](http://docs.opencv.org/3.0-last-rst/doc/tutorials/introduction/linux_gcc_cmake/linux_gcc_cmake.html) for a refresher on compiling OpenCV projects with G++ and CMake.

After compiling the Multiple Wave Tracking executable, you can launch the program from the command line after navigating to the directory containing the executable.  For example:

> citrusvanila build $ ./mwt_cpp some_video_with_waves.mp4

You should see output like this:

    Starting analysis of 840 frames.
    100 frames complete. (122 frames/sec; 0 sec/frame)
    200 frames complete. (131 frames/sec; 0.005 sec/frame)
    
The program reports its status every 100 frames, as well as the performance of the program.

The program will report simple statistics at the conclusion of analysis, like the following:

    Program complete.
    Program took 5950 milliseconds.
    Program speed: 168 frames per second.
    2 wave(s) found.

A log of the tracking routine is written to "wave_log.json" for a frame-by-frame breakdown of the program.

<!---

**Visualizing Recognition**

The Jupyter Notebook file 'surferdetection_predictscene.ipynb' has been provided to help you visualize prediction on unseen scenes.
This notebook will utilize the surferdetection_predict.py file that makes use of a fully-trained surfer detector model in the surferdetection_restore directory.
Launch the notebook from the commandline with the following:

    ipython notebook surferdetection_predictscene.ipynb

And that's it!  Please contact the author for gaining access to source data, troubleshooting or general comments.

--->

## Model Discussion

**Preprocessing**: In our videos of maritime environments, waves are surely the largest objects present and thus our downsizing of input videos by a factor of 4 (or greater) is acceptable. Background modeling however for maritime environments is very difficult endeavor even with static cameras due to the nature of liquids.  We employ a Gaussian Mixture Model with 5 Gaussians per pixel in order to give us the greatest flexibility in accounting for light and surface refractions.  The Gaussians that constitute a pixel's representation are determined using Maximum Likelihood Estimation (MLE), solved in our case with an Expectation Maximization (EM) algorithm.  This modeling classifies as foreground and pixel whose value is calculated to exceed 2.5x the standard deviation of any one of these Gaussians.  The EM algorithm that is performed on a per-pixel basis contributes to the expense of background modeling in the program, estimated to be about 50% of the total CPU resources in the program.  GMM is appropriate for extracting waves from our videos as our reprsentation of a broken wave is denoted by the presence of sea foam, and as foam as a physical objects is the trapping of air inside bubbles whose refractive properties give the foam a holistic color approaching white.  This is generally contrasted with the ocean surface, that despite being dynamic does not have such refractive properties and traps light such that its intensity is much lower than that of foam.  A background history of 300 frames (equivalent to 10 seconds in our sample videos) ensures that a wave passing through is a very infrequent even and thus esnuring its detection inthe foreground.  The denoising operation in the model employs a mathematical morphology known as 'opening', which is a sequence of erosion followed by dilation.  This has the effect of sending residual foreground pixels from the GMM model to the background.

**Detection**: The resultant frame from the preprocessing module is a binary image in which the background is represented by one value while the foreground is represented by another.  In our case, we are left with an waves represented as sea foam in the foreground.  Detection is intended to localize these waves, and to add a level of discernment between ocean waves and similar objects that may be whitecapping waves (waves that are broken by the wind) and shorebreak (waves that are broken on the shore).  We use a contour finding algorithm of Suzuki and Abe's Fast Contour Finding that identifies shapes by tracing connected pixels of a shapes border, and returns these shapes and locations.  These contours are filtered for area (to eliminate foreground objects that are too small to be considered) and inertia (to eliminate foreground objects whose shape does not match that of a breaking wave.  We are left with contours that are likely waves.

**Tracking**: We can take advantage of two assumptions about waves that eliminates our reliance on traditional sample-based tracking methologies and their associated probabilistic components.  The first is that waves are highly periodic in arrival and therefore will not exhibit occlusion or superimposition.  The second assumption is about the wave's dynamics; specifically, that a wave's movement can be desribed by its displacement orthogonal to the axis along which the wave was first identified in the sequence.  These two assumptions allow us to confidently define a search region in the next frame using its center-of-mass estimates in the current frame, and reduces our search space for the wave's position in successive frames to a search along one dimension.  The reduction in dimensionality of the search space allows us to cheaply and exhaustively search for a global maximum position that describes our tracked wave in successive frames.  We do not need to rely on sample-based tracking methods that are susceptible to drift and/or local maxima.

**Recognition**: By introducing tracking, we are able to simply represent waves by their sea-foam signatures, and to confidently classify waves in videos by using tracked dynamics.  As is demonstrated in the sample video, ocean waves have similar contour representation to "shorebreak"-type waves as well as "whitecapping"-type waves, yet our use of tracking allows us to correctly classifies these similar objects as being of the non-wave class.

#### Footnotes
<a name="myfootnote1">1</a>: [Bodor, Robert, Bennett Jackson, and Nikolaos Papanikolopoulos. "Vision-based human tracking and activity recognition." Proc. of the 11th Mediterranean Conf. on Control and Automation. Vol. 1. 2003.](http://mha.cs.umn.edu/Papers/Vision_Tracking_Recognition.pdf)
