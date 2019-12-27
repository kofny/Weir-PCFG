cmake_minimum_required(VERSION 3.15)
project(WeirPCFG)

set(CMAKE_CXX_STANDARD 14)

include_directories(guesser)
include_directories(trainer)

add_executable(WeirPCFG
        guesser/pcfg_manager.cpp
        guesser/pcfg_manager.h
        trainer/train.py)
