set(CMAKE_MODULE_PATH "${Audio3D_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

enable_testing()
find_package(GTest REQUIRED)
find_package (Threads)
find_package(PortAudio REQUIRED)
MESSAGE( STATUS "PORTAUDIO_INCLUDE_DIRS:         " ${PORTAUDIO_INCLUDE_DIRS} )

include_directories(${PORTAUDIO_INCLUDE_DIRS}
                    ${GTEST_INCLUDE_DIR})
LINK_DIRECTORIES(/opt/local/lib)

add_executable(test_fft_filter test_fft_filter.cpp)
target_link_libraries(test_fft_filter ${PROJECT_NAME} ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_test(
    NAME test_fft_filter
    COMMAND test_fft_filter
)

add_executable(test_fft test_fft.cpp)
target_link_libraries(test_fft kissfft ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_test(
    NAME test_fft
    COMMAND test_fft
)

add_executable(test_room_model test_room_model.cpp)
target_link_libraries(test_room_model room_model ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_test(
    NAME test_room_model
    COMMAND test_room_model
)


#add_executable(pa_sample pa_sample.cpp)
#target_link_libraries(pa_sample ${PROJECT_NAME} ${PORTAUDIO_LIBRARIES})
