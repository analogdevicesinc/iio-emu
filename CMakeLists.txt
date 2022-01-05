#  ADIBSD License
#
#  Copyright (c) 2021 Analog Devices Inc.
#  All rights reserved.
#
#  This file is part of iio-emu
#  (see http://www.github.com/analogdevicesinc/iio-emu).
#
#  Redistribution and use in source and binary forms, with or without modification,
#  are permitted provided that the following conditions are met:
#      - Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      - Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in
#        the documentation and/or other materials provided with the
#        distribution.
#      - Neither the name of Analog Devices, Inc. nor the names of its
#        contributors may be used to endorse or promote products derived
#        from this software without specific prior written permission.
#      - The use of this software may or may not infringe the patent rights
#        of one or more patent holders.  This license does not release you
#        from the requirement that you obtain separate licenses from these
#        patent holders to use this software.
#      - Use of the software either in source or binary form, must be run
#        on or directly connected to an Analog Devices Inc. component.
#
#  THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#  INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
#  PARTICULAR PURPOSE ARE DISCLAIMED.
#
#  IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
#  RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
#  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


cmake_minimum_required(VERSION 3.1.3)
project(iio-emu LANGUAGES CXX)

include(cmake/compileResources.cmake)
include(cmake/compilerWarnings.cmake)
include(cmake/staticAnalyzers.cmake)

set(CMAKE_CXX_STANDARD 11)

set(IIO_EMU_VERSION_MAJOR 0)
set(IIO_EMU_VERSION_MINOR 1)
set(IIO_EMU_VERSION ${IIO_EMU_VERSION_MAJOR}.${IIO_EMU_VERSION_MINOR})

option(BUILD_TOOLS "Build the tools" OFF)

# Set the default install path to /usr/local
if (NOT WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE)
endif()

include(GNUInstallDirs)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

find_package(tinyiiod REQUIRED)
find_package(LibXml2 REQUIRED)

FILE(GLOB_RECURSE SRC_LIST
        iiod/*.cpp
        networking/*.cpp
        utils/*.cpp)

FILE(GLOB_RECURSE RESOURCES_LIST
        iiod/*.xml)

get_resources(${CMAKE_CURRENT_SOURCE_DIR}/iiod/*.xml COMPILED_RESOURCES)

add_executable(${PROJECT_NAME}
        main.cpp
        ${SRC_LIST}
        ${COMPILED_RESOURCES})

if (NOT DEFINED IIO_EMU_LOG_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE IIO_EMU_LOG_LEVEL=1)
    message(STATUS "Logging level: 1")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE IIO_EMU_LOG_LEVEL=${IIO_EMU_LOG_LEVEL})
    message(STATUS "Logging level: ${IIO_EMU_LOG_LEVEL}")
endif()

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/resources
        )

if (WIN32 OR CYGWIN OR MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE wsock32 ws2_32)
endif()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        tinyiiod::tinyiiod
        ${LIBXML2_LIBRARIES}
        )

set_warnings(${PROJECT_NAME})

if (NOT WIN32)
    find_library(PTHREAD_LIBRARIES pthread)
    if (PTHREAD_LIBRARIES)
        target_link_libraries(${PROJECT_NAME}
                PRIVATE
                ${PTHREAD_LIBRARIES}
                )
    endif()
endif()

if (BUILD_TOOLS)
    message(STATUS "Building tools")
    add_subdirectory(tools)
endif()

if (NOT OSX_PACKAGE)
	# Support creating some basic binpkgs via `make package`.
	# Disabled if OSX_PACKAGE is enabled, as tarballs would end up empty otherwise.
	option(ENABLE_PACKAGING "Create .deb/.rpm or .tar.gz packages via 'make package'" OFF)

	if(ENABLE_PACKAGING)
		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			include(cmake/DarwinPackaging.cmake)
		endif()
		if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			include(cmake/LinuxPackaging.cmake)
		endif()
	endif()
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
