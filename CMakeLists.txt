cmake_minimum_required(VERSION 3.7)

project(jfyspot)

add_executable(jfyspot jfycommon.cpp jfydata.cpp jfyserial.cpp jfypacketdata.cpp jfyexception.cpp jfyconnection.cpp main.cpp)

install(TARGETS jfyspot DESTINATION bin)
