cmake_minimum_required(VERSION 3.5.0)
project(Celeritas)
option(CELERITAS_INSTALL_HEADER "Install Celeritas's headers files" ON)
add_subdirectory(src)
if (CELERITAS_INSTALL_HEADER)
install(DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
