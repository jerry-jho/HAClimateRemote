
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
CXXFLAGS = -g -O2 -std=c++11 -I$(mkfile_dir)/nativelibs -Wall -Wextra -pthread -Wno-unused-parameter -DARDUINO
NATIVE_SRC = $(mkfile_dir)/nativelibs/arduino.cpp $(mkfile_dir)/nativelibs/HTTPClient.cpp $(mkfile_dir)/nativelibs/WiFi.cpp
 
default:
	g++ main.cpp $(CXXFLAGS) $(NATIVE_SRC) -o main.exe -lwsock32 -lws2_32