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

cmake_minimum_required(VERSION 2.8)

project(Templight-Tools)
set(CMAKE_PROJECT_NAME templight-tools)
set(MAJOR_VERSION 1)
set(MINOR_VERSION 9)
set(PATCH_VERSION 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_mods")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Use this line to force static boost libs (or pass -DBoost_USE_STATIC_LIBS=ON to cmake command).
# set(Boost_USE_STATIC_LIBS ON)

# This BoilerplateMacros module sets most compiler flags and includes boost libraries.
include(BoilerplateMacros)

enable_cpp11( REQUIRED )


if((${CMAKE_VERSION} VERSION_GREATER "2.8.7") AND (EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake"))
  include(InstallRequiredSystemLibraries)
  
  if(WIN32)
    set(
      CPACK_NSIS_MENU_LINKS
      "https://github.com/mikael-s-persson/templight-tools" "Templight-tools website"
    )
  else()
    set(CPACK_SET_DESTDIR "on")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  endif()
  
  if (DEFINED PLATFORM_CPACK_GENERATOR)
    set(CPACK_GENERATOR "${PLATFORM_CPACK_GENERATOR}")
  endif ()
  
  if (PLATFORM_NAME AND NOT WIN32)
    set(TEMPLIGHT_PACKAGE_PLATFORM "_${PLATFORM_NAME}${PLATFORM_VERSION}")
  else()
    set(TEMPLIGHT_PACKAGE_PLATFORM "")
  endif()
  
  set(CPACK_DEB_COMPONENT_INSTALL 1)
  
  set(CPACK_PACKAGE_NAME "templight-tools")
  set(CPACK_PACKAGE_VENDOR "Sven Mikael Persson")
  set(CPACK_PACKAGE_CONTACT "Sven Mikael Persson <mikael.s.persson@gmail.com>")
  set(CPACK_PACKAGE_DESCRIPTION "Templight-tools")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tools for dealing with templight trace files.")
  set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}${TEMPLIGHT_PACKAGE_PLATFORM}_${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
  seT(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
  set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
  
  # This is not used right now (until things are finalized), but will come in handy:
  if( 0 )
    if (SUSE_LINUX)
      set(CPACK_RPM_PACKAGE_REQUIRES "libstdc++-devel, glibc-devel")
    elseif (FEDORA_LINUX)
      set(CPACK_RPM_PACKAGE_REQUIRES "libstdc++-devel, glibc-headers, glibc-devel")
    elseif (UBUNTU_LINUX)
      if ("${PLATFORM_VERSION}" STREQUAL "14.04")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++-4.8-dev")
      elseif ("${PLATFORM_VERSION}" STREQUAL "14.10")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++-4.9-dev")
      else()
        message(WARNING "Unsupported Ubuntu version. Package dependencies will be invalid.")
      endif()
    elseif (DEBIAN_LINUX)
      if ( ("${PLATFORM_VERSION}" STREQUAL "7.6") OR ("${PLATFORM_VERSION}" STREQUAL "7.7") )
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6-4.7-dev")
      else()
        message(WARNING "Unsupported Debian version. Package dependencies will be invalid.")
      endif()
    endif()
  endif()
  
  
  include(CPack)
endif()


if(Boost_FOUND)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  link_directories(${Boost_LIBRARY_DIRS})
endif()


# Include templight-tools' own include directory:
include_directories(AFTER "include")

add_subdirectory_if_cmake("src")
add_subdirectory_if_cmake("convert")
add_subdirectory_if_cmake("test")
add_subdirectory_if_cmake("perf")
add_subdirectory_if_cmake("example")

install(DIRECTORY include/ DESTINATION include COMPONENT libtemplight-dev FILES_MATCHING PATTERN "*.hpp")



if((${CMAKE_VERSION} VERSION_GREATER "2.8.7") AND (EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake"))
  
  cpack_add_component(templight-tools
    DISPLAY_NAME "templight-tools"
    DESCRIPTION "Tools for dealing with templight trace files."
  )
  
  cpack_add_component(libtemplight
    DISPLAY_NAME "libtemplight"
    DESCRIPTION "A library to deal with templight trace files."
  )
  
  cpack_add_component(libtemplight-dev
    DISPLAY_NAME "libtemplight-dev"
    DESCRIPTION "Development files, mainly headers, for libtemplight, a library to deal with templight trace files."
  )
  
endif()



