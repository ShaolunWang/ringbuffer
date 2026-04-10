cmake_minimum_required(VERSION 3.15...4.0)

project(
  ringbuffer 
  VERSION 0.1
  DESCRIPTION "lock-free ringbuffer"
  LANGUAGES CXX)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)


set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	add_compile_definitions(Debug)
	add_compile_options(-fsanitize=address,undefined -g)
	add_link_options(-fsanitize=address)
endif()

if(CMAKE_BUILD_TYPE MATCHES THREAD)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	add_compile_definitions(Debug)
	add_compile_options(-fsanitize=thread -g)
	add_link_options(-fsanitize=thread)
endif()


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

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
endif()

add_subdirectory(test)
