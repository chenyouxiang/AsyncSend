cmake_minimum_required(VERSION 2.6)

project(async-send)

set(SRC src/main.cc src/Connection.cc src/FIFOConnection.cc src/Poller.cc src/http/parser.cc src/http/http_parser.cc)

add_compile_options(-std=c++11 -g)

add_executable(async-send ${SRC})


