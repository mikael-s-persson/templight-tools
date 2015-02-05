# This cmake module sets up compiler flags and custom target macros
# for use when building templight-tools.
# 
# 

#     Copyright 2015 Sven Mikael Persson
# 
#     THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE v3 (GPLv3).
# 
#     This file is part of templight-tools.
# 
#     Templight-tools is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     Templight-tools is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with templight-tools (as LICENSE in the root folder).
#     If not, see <http://www.gnu.org/licenses/>.


if(${CMAKE_VERSION} VERSION_GREATER 3.1)
  cmake_policy(SET CMP0054 OLD)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Configured for build-type: ${CMAKE_BUILD_TYPE}")


enable_testing()

include(CheckCXXCompilerFlag)

# Check and enable C++11 or C++0x features:  Can be called as: enable_cpp11( REQUIRED )
macro(enable_cpp11 level)
  if( level STREQUAL "REQUIRED")
    set(_ENABLE_CPP11_POSSIBLE_ERROR FATAL_ERROR)
  else()
    set(_ENABLE_CPP11_POSSIBLE_ERROR WARNING)
  endif()
  if(MSVC)
    if(MSVC_VERSION GREATER 1700)
      message(STATUS "This MSVC compiler version (>1700) has C++11 support enabled by default.")
    elseif(MSVC_VERSION GREATER 1600)
      message(${_ENABLE_CPP11_POSSIBLE_ERROR} "This MSVC compiler version (1700) has limited C++11 support. Consider using a newer version with better C++11 support, such as Visual Studio 2013 and above.")
    else()
      message(${_ENABLE_CPP11_POSSIBLE_ERROR} "This MSVC compiler version (<1700) has no C++11 support. Consider using a newer version with C++11 support, such as Visual Studio 2013 and above.")
    endif()
  else()
    CHECK_CXX_COMPILER_FLAG("-std=c++11" _COMPILER_SUPPORTS_CXX11)
    if(_COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      message(STATUS "The compiler has C++11 support. C++11 mode was enabled.")
    else()
      CHECK_CXX_COMPILER_FLAG("-std=c++0x" _COMPILER_SUPPORTS_CXX0X)
      if(_COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        message(${_ENABLE_CPP11_POSSIBLE_ERROR} "The compiler only has experimental C++11 support. Consider using a newer compiler with full C++11 support.")
      else()
        message(${_ENABLE_CPP11_POSSIBLE_ERROR} "The compiler has no detectable C++11 support. Consider using a newer compiler with C++11 support.")
      endif()
      mark_as_advanced(_COMPILER_SUPPORTS_CXX0X)
    endif()
    mark_as_advanced(_COMPILER_SUPPORTS_CXX11)
  endif()
  mark_as_advanced(_ENABLE_CPP11_POSSIBLE_ERROR)
endmacro(enable_cpp11)


if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /bigobj -D_SCL_SECURE_NO_WARNINGS")
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -ftemplate-depth=2000 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  if (WIN32)
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} --enable-stdcall-fixup")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # This is a hack because the major-minor version variables (${CLANG_VERSION_MAJOR}.${CLANG_VERSION_MINOR}) from cmake seem unreliable 
    execute_process( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})
    
    # TODO This is just a temporary hack because of a version of clang failing to compile with a experimental 4.9 version of libstdc++.
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -isystem /usr/include/c++/4.8 -isystem /usr/include/x86_64-linux-gnu/c++/4.8")
    message(STATUS "Detected Clang version: ${CLANG_VERSION_STRING}")
    if( CLANG_VERSION_STRING VERSION_LESS 3.6 )
#     if( "${CLANG_VERSION_MAJOR}.${CLANG_VERSION_MINOR}" STRLESS "3.6") 
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef -Wno-missing-braces")
    endif()
  endif()
endif()
message(STATUS "Configured compiler options for ${CMAKE_SYSTEM_NAME} system with ${CMAKE_CXX_COMPILER_ID} toolset.")

# Look for Boost:

# set(Boost_DEBUG TRUE)

set(Boost_ADDITIONAL_VERSIONS "1.45" "1.45.0" "1.46" "1.46.0" "1.46.1" "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0" "1.55" "1.55.0" "1.56" "1.56.0" "1.57" "1.57.0")
if(NOT DEFINED Boost_USE_STATIC_LIBS)
  # Some bug in some versions of FindBoost requires that this be set to OFF manually first.
  set(Boost_USE_STATIC_LIBS OFF)
endif()
set(Boost_USE_MULTITHREADED ON)

# if a custom path for boost is provided, than use that (and suppress system paths).
if(EXISTS "${CUSTOM_BOOST_PATH}/include/boost")
  set(Boost_INCLUDE_DIR "${CUSTOM_BOOST_PATH}/include")
  set(Boost_LIBRARY_DIR "${CUSTOM_BOOST_PATH}/lib")
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

if (NOT WIN32)
  # make sure that the *nix suffixes and prefixes are correct (some cmake installs of findBoost.cmake are wrong with this).
  set(_ORIGINAL_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(_ORIGINAL_CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES})
  if( Boost_USE_STATIC_LIBS )
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .so ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif()
  set(CMAKE_FIND_LIBRARY_PREFIXES lib ${CMAKE_FIND_LIBRARY_PREFIXES})
endif()

find_package(Boost 1.48 COMPONENTS system program_options unit_test_framework filesystem REQUIRED)
if(Boost_FOUND)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  link_directories(${Boost_LIBRARY_DIRS})
  message(STATUS "Boost library version ${Boost_LIB_VERSION} found, with headers at '${Boost_INCLUDE_DIR}' and libraries at '${Boost_LIBRARY_DIRS}' for libraries: \n${Boost_LIBRARIES}")
  if(MSVC)
    # Disable the libraries, since it uses automatic linking:
    set(Boost_LIBRARIES "")
  endif()
endif()

if( NOT WIN32 )
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_ORIGINAL_CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_PREFIXES ${_ORIGINAL_CMAKE_FIND_LIBRARY_PREFIXES})
endif()




macro(add_subdirectory_if_cmake subdir)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${subdir}")
  endif()
endmacro(add_subdirectory_if_cmake)


# Now set the global macros for setting up targets.
macro(templight_setup_tool_program target_name)
  install(TARGETS ${target_name} RUNTIME DESTINATION bin COMPONENT templight-tools)
  message(STATUS "Registered templight-tools tool program ${target_name}.")
endmacro(templight_setup_tool_program)

macro(templight_setup_static_library target_name)
  install(TARGETS ${target_name} ARCHIVE DESTINATION lib COMPONENT libtemplight)
  message(STATUS "Registered templight-tools static library ${target_name}.")
endmacro(templight_setup_static_library)

macro(templight_setup_shared_library target_name)
  install(TARGETS ${target_name} LIBRARY DESTINATION lib COMPONENT libtemplight)
  message(STATUS "Registered templight-tools shared library ${target_name}.")
endmacro(templight_setup_shared_library)

macro(templight_setup_target target_name)
  set_property(TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/examples")
  message(STATUS "Registered templight-tools example program ${target_name}.")
endmacro(templight_setup_target)

macro(templight_setup_test_program target_name)
  set_property(TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_tests")
  add_test(NAME "${target_name}" WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_tests/" COMMAND "$<TARGET_FILE:${target_name}>")
  message(STATUS "Registered templight-tools test program ${target_name}.")
endmacro(templight_setup_test_program)




