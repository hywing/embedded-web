cmake_minimum_required(VERSION 3.5)

project(tcp-client LANGUAGES C)

add_executable(${PROJECT_NAME} main.c mongoose.c)
