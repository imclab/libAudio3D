cmake_minimum_required (VERSION 2.6) 
project (Audio3D) 

set(CMAKE_MODULE_PATH "${Audio3D_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

SET(MIT_KEMAR_DATASET_FLAG    "-DMIT_KEMAR")
option(USE_MIT_KEMAR_DATASET  "Use MIT KEMAR HRTF dataset" ON)

find_package(Libsamplerate REQUIRED) 
find_package(FLANN 1.7.0 REQUIRED)

include_directories(SYSTEM 
                    ${FLANN_INCLUDE_DIRS}
                    ${LIBSAMPLERATE_INCLUDE_DIRS}
                    ${Audio3D_SOURCE_DIR}/kissfft
                    ${Audio3D_SOURCE_DIR}/include 
                    ${Audio3D_SOURCE_DIR}/data 
                   )

add_definitions( ${LIBRESAMPLE_DEFINITIONS} )

add_library(kissfft kissfft/kiss_fft.c kissfft/kiss_fftr.c)
add_library (fft_filter src/fft_filter_impl.cpp src/fft_filter.cpp)
target_link_libraries (fft_filter kissfft)  

add_library (resampler src/resampler.cpp) 
target_link_libraries(resampler ${LIBSAMPLERATE_LIBRARIES})

add_library (hrtf src/hrtf.cpp) 
if(USE_MIT_KEMAR_DATASET)
   set_target_properties(hrtf PROPERTIES COMPILE_FLAGS ${MIT_KEMAR_DATASET_FLAG} )
endif(USE_MIT_KEMAR_DATASET)
target_link_libraries(hrtf resampler)

add_library (${PROJECT_NAME} src/audio_3d.cpp
                             src/reberation.cpp
            ) 
target_link_libraries(${PROJECT_NAME} hrtf fft_filter)

ADD_SUBDIRECTORY (test)