cmake_minimum_required (VERSION 2.6) 
project (Audio3D) 

ADD_SUBDIRECTORY (libFFTFilter)

ADD_SUBDIRECTORY (libHRTF)

include_directories(${Audio3D_SOURCE_DIR}/include 
                    ${FFTFilter_SOURCE_DIR}/include
                    ${HRTF_SOURCE_DIR}/include
                   )

add_library (${PROJECT_NAME} src/audio_3d.cpp) 
target_link_libraries(${PROJECT_NAME} HRTF FFTFilter)

ADD_SUBDIRECTORY (test)