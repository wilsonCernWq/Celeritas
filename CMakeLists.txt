cmake_minimum_required(VERSION 3.5.0)
project(Celeritas)
add_subdirectory(src)
install(DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_PREFIX})
