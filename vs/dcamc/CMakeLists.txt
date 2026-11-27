# dcamc: Windows-only camera client (DirectShow capture + LLAH retrieval).
#
# Replaces the Visual Studio 2005 project (dcamc.sln / dcamc.vcproj), which current Visual Studio
# and Windows SDKs cannot build. The source list, libraries and preprocessor definitions are
# the same as in that project.
#
#   cmake -S vs/dcamc -B build -A x64 -DOpenCV_DIR=<OpenCV install or build dir>
#   cmake --build build --config Release
#
# Works with OpenCV 4 and OpenCV 5. Use -A Win32 for a 32-bit build like the original project.

cmake_minimum_required(VERSION 3.16)
project(dcamc CXX)

if(NOT WIN32 OR NOT MSVC)
  message(FATAL_ERROR "dcamc uses DirectShow and Winsock; build it on Windows with Visual Studio.")
endif()

find_package(OpenCV REQUIRED)
if(OpenCV_VERSION VERSION_LESS 4.0)
  message(FATAL_ERROR "dcamc needs OpenCV 4 or later (found ${OpenCV_VERSION}).")
endif()
set(DCAMC_OPENCV_MODULES core imgproc imgcodecs highgui videoio)
if(OpenCV_VERSION VERSION_GREATER_EQUAL 5.0)
  # OpenCV 5 moved goodFeaturesToTrack (camharris.cpp) from imgproc to the features module.
  list(APPEND DCAMC_OPENCV_MODULES features)
endif()
find_package(OpenCV REQUIRED COMPONENTS ${DCAMC_OPENCV_MODULES})
message(STATUS "dcamc: OpenCV ${OpenCV_VERSION}")

# Same files as the <Files> section of the former dcamc.vcproj.
set(DCAMC_SOURCES
  ar.cpp auto_connect.cpp block.cpp camharris.cpp cFpTuneCap.cpp cFpTuneReg.cpp cr.cpp
  dcamc.cpp disc.cpp dscap.cpp fptune.cpp gencomb.cpp gj.cpp hash.cpp hist.cpp init.cpp
  nimg.cpp nn5dcamc.cpp proctime.cpp proj4p.cpp projrecov.cpp ws_cl.cpp)
list(TRANSFORM DCAMC_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/dcamc/")

add_executable(dcamc ${DCAMC_SOURCES})
target_compile_features(dcamc PRIVATE cxx_std_17)

# compat/ provides qedit.h, which current Windows SDKs no longer include.
target_include_directories(dcamc PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/compat"
  "${CMAKE_CURRENT_SOURCE_DIR}/dcamc")

# As in the former dcamc.vcproj: console application, no UNICODE (character set "Not Set").
target_compile_definitions(dcamc PRIVATE
  WIN32 _CONSOLE
  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

# The sources are Shift_JIS without a BOM. Read and run them as code page 932 on every
# machine, as Visual Studio 2005 did on Japanese Windows. Without this, an English-locale
# compiler misreads comments whose last byte is 0x5C ('\') as line continuations.
target_compile_options(dcamc PRIVATE /W3 /source-charset:.932 /execution-charset:.932)

target_link_libraries(dcamc PRIVATE ${OpenCV_LIBS} ws2_32 strmiids winmm)

# The program reads client.ini (init.h: kIniFileName) from the working directory.
set_property(TARGET dcamc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dcamc")
