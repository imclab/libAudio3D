cmake_minimum_required (VERSION 2.6) 
project (kissfft) 

add_library(${PROJECT_NAME} ${kissfft_SOURCE_DIR}/kiss_fft.c 
                            ${kissfft_SOURCE_DIR}/kiss_fftr.c)