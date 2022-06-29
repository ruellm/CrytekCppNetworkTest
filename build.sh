#!/bin/bash

#build essential to make sure CMake is installed
sudo apt-get install build-essential

#make build folder
mkdir build

#build project
cd build
cmake ..
make

#copy the binaries to demo directories
cp Client/Client ../Demo/Linux
cp Server/Server ../Demo/Linux
