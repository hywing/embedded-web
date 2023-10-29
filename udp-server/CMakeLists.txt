cmake_minimum_required(VERSION 3.5)

project(udp-server LANGUAGES C)

add_executable(${PROJECT_NAME} main.c mongoose.c)
