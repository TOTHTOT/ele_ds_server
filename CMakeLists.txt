cmake_minimum_required(VERSION 3.31)
project(ele_ds_server C)

set(CMAKE_C_STANDARD 11)

add_library(ele_ds_server STATIC library.c)
