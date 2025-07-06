cmake_minimum_required(VERSION 3.10)
project(GenderDetectionNoOpenCV)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    # Add Windows-specific libraries
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    
    # Add executable
    add_executable(GenderDetection main.cpp)
    
    # Link Windows libraries
    target_link_libraries(GenderDetection 
        vfw32    # Video for Windows
        winmm    # Windows Multimedia
        user32   # User interface
        gdi32    # Graphics Device Interface
    )
    
else()
    # For Linux/Unix systems - basic version without camera
    add_executable(GenderDetection main.cpp)
    
    # Link standard libraries
    target_link_libraries(GenderDetection 
        pthread  # Threading support
    )
    
    # Add compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")