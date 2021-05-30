#cmake最低版本号要求
cmake_minimum_required(VERSION 3.0.0)
#设置PROJECT_NAME变量
set(PROJECT_NAME led)
#程序版本号
project(${PROJECT_NAME})
#查找当前目录下的所有源文件并存入DIR_SRCS
#aux_source_directory(src DIR_SRCS)
# 递归列出所有源文件
#file (GLOB_RECURSE SOURCE_FILES *.cc)
include_directories(include)
aux_source_directory(lib SRC_LIST)
#动态库位置
#LINK_DIRECTORIES(/usr/local/lib/)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")#多进程配置

# 保证glfw dir被编译
#add_subdirectory(${wiringPi_DIR})
# 添加头文件搜索路径
#include_directories(${wiringPi_DIR} /usr/include)
# 添加链接库
#find_path(HIREDIS_INCLUDE_DIR wiringSerial.h /usr/include/hiredis)
#find_library(HIREDIS_LIBRARY NAMES libwiringPi.so PATH /usr/lib/)
#enable_testing()
# 添加一个可编译的目标到工程
add_executable(${PROJECT_NAME} main.cpp ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} libwiringPi.so) # 链接动态库


