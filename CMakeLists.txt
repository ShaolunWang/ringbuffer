cmake_minimum_required(VERSION 3.15...4.0)

project(
  ringbuffer 
  VERSION 0.1
  DESCRIPTION "lock-free ringbuffer"
  LANGUAGES CXX)


option(RINGBUFFER_BUILD_TESTS "Build tests" OFF)

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    set(RINGBUFFER_BUILD_TESTS ON)
endif()


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(RINGBUFFER_BUILD_TESTS)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
  add_subdirectory(test)
endif()

add_library(ringbuffer INTERFACE)
add_library(ringbuffer::ringbuffer ALIAS ringbuffer)

target_include_directories(ringbuffer
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(ringbuffer INTERFACE cxx_std_23)

include(GNUInstallDirs)

install(
    TARGETS ringbuffer
    EXPORT ringbuffer_targets
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ringbuffer_targets
    FILE ringbufferTargets.cmake
    NAMESPACE ringbuffer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ringbuffer
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ringbuffer-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ringbuffer
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ringbuffer
)
