cmake_minimum_required(VERSION 3.23)
project(Week9 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Week9
        client.c
        config.h
        http.c
        http.h
        server.c
        serverHelper.c)
