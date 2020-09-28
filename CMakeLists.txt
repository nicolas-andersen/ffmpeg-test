cmake_minimum_required(VERSION 3.16)
project(media_player C)

set(CMAKE_C_STANDARD 99)

add_executable(media_player main.c glad/glad.c glad/glad.h opengl.c opengl.h)

find_package(PkgConfig REQUIRED)

# ffmpeg
pkg_check_modules(AVCODEC REQUIRED libavcodec)
include_directories(${AVCODEC_INCLUDE_DIRS})
target_link_libraries(media_player ${AVCODEC_LIBRARIES})

pkg_check_modules(AVFORMAT REQUIRED libavformat)
include_directories(${AVFORMAT_INCLUDE_DIRS})
target_link_libraries(media_player ${AVFORMAT_LIBRARIES})

pkg_check_modules(AVFILTER REQUIRED libavfilter)
include_directories(${AVFILTER_INCLUDE_DIRS})
target_link_libraries(media_player ${AVFILTER_LIBRARIES})

pkg_check_modules(AVUTIL REQUIRED libavutil)
include_directories(${AVUTIL_INCLUDE_DIRS})
target_link_libraries(media_player ${AVUTIL_LIBRARIES})

pkg_check_modules(SWSCALE REQUIRED libswscale)
include_directories(${SWSCALE_INCLUDE_DIRS})
target_link_libraries(media_player ${SWSCALE_LIBRARIES})

#SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(media_player ${SDL2_LIBRARIES})