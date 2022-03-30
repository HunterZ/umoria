cmake_minimum_required(VERSION 3.22.3)
project(umoria)
# try to build whatever seems appropriate for the current environment
if (MINGW)
  add_subdirectory(windows)
else()
  error("Sorry, no supported platform has been detected")
endif()
