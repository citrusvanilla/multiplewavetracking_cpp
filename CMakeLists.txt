# CMAKE VERSION 3.8.1
cmake_minimum_required(VERSION 3.8.1)

# PROJECT NAME
project(mwt_cpp)

# REQUIRE OPENCV
find_package(OpenCV 3.2.0 REQUIRED)
message("OpenCV version: ${OpenCV_VERSION}")

# HEADERS
include_directories(include)

# SOURCES with GLOBBING
file(GLOB SOURCES "src/*.cpp")

# REQUEST EXECUTABLE
add_executable(${PROJECT_NAME} ${SOURCES})

# LINK OPENCV LIBRARIES
target_link_libraries (${PROJECT_NAME} ${OpenCV_LIBS})