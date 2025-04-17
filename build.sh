#!/bin/bash

# Source files
SRC_FILES="main.cpp resources.cpp"

# Custom raylib path
RAYLIB_PATH="/home/kobedb/raylib"
INCLUDE_DIR="$RAYLIB_PATH/include"
LIB_DIR="$RAYLIB_PATH/lib"

# Compiler and flags
CXX=g++
CXXFLAGS="-O2 -std=c++14"
INCLUDES="-I$INCLUDE_DIR"

# Static raylib, dynamic everything else
LIBS="-L$LIB_DIR -Wl,-Bstatic -lraylib -Wl,-Bdynamic -lGL -lm -lpthread -ldl -lrt -lX11"

# Compile
$CXX $CXXFLAGS $SRC_FILES $INCLUDES $LIBS -o game