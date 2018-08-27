add_library(SON STATIC
  SON_bigbrother.cpp
  SON_celeritasClient.cpp
  SON_celeritasServer.cpp
  SON_iovec.cpp
  SON_paralleltcp.cpp
  SON_perf.cpp
  SON_tcp.cpp)
target_include_directories(SON PUBLIC
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Celeritas>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include;>")
find_package(Threads QUIET)
if (TARGET Threads::Threads)
  target_link_libraries(SON Threads::Threads)
endif ()
set_property(TARGET SON APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS USE_CELERITAS)
install(TARGETS SON
  EXPORT CeleritasTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(EXPORT CeleritasTargets
  FILE CeleritasTargets.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/vlutil)
export(EXPORT CeleritasTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/CeleritasTargets.cmake)
