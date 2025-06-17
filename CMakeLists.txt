cmake_minimum_required(VERSION 3.31)
project(ele_ds_server C)

set(CMAKE_C_STANDARD 11)

# 递归查找所有.c和.h文件
file(GLOB_RECURSE SOURCES "*.c")
file(GLOB_RECURSE HEADERS "*.h")

# 添加头文件目录，确保CLion能找到头文件
foreach(header ${HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    include_directories(${dir})
endforeach()

# 创建静态库，包含所有找到的源文件
add_library(ele_ds_server STATIC ${SOURCES} ${HEADERS})

# 打印找到的文件列表（可选，用于调试）
message(STATUS "Sources: ${SOURCES}")
message(STATUS "Headers: ${HEADERS}")
