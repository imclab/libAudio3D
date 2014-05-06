set(CMAKE_MODULE_PATH "${Audio3D_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

find_package(PortAudio REQUIRED)

include_directories(${PORTAUDIO_INCLUDE_DIRS}
                   )
LINK_DIRECTORIES(/opt/local/lib)

MESSAGE( STATUS "PORTAUDIO_INCLUDE_DIRS:         " ${PORTAUDIO_INCLUDE_DIRS} )

add_executable(pa_sample pa_sample.cpp)
target_link_libraries(pa_sample ${PROJECT_NAME} ${PORTAUDIO_LIBRARIES})
