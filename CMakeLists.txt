###############################################################################
## Name:        CMakeLists.txt
## Purpose:     To build wxTestSVG2 application
## Author:      PB
## Created:     2024-01-18
## Copyright:   (c) 2024 PB
## Licence:     wxWindows licence
###############################################################################

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(wxTestSVG2)

find_package(wxWidgets 3.2 COMPONENTS webview core base REQUIRED)

add_subdirectory(lunasvg)

set(SOURCES
  bmpbndl_lunasvg.h
  bmpbndl_lunasvg.cpp
  svgapp.cpp
  svgbench.h
  svgbench.cpp
  svgframe.h
  svgframe.cpp
)
if (WIN32)
  list(APPEND SOURCES "${wxWidgets_ROOT_DIR}/include/wx/msw/wx.rc")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_executable(${PROJECT_NAME} ${SOURCES})

include(${wxWidgets_USE_FILE})
target_include_directories(${PROJECT_NAME} PRIVATE lunasvg/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

if (WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE wxUSE_RC_MANIFEST wxUSE_DPI_AWARE_MANIFEST=2)
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE YES)
  
  if(MSVC)
      target_compile_options(${PROJECT_NAME} PRIVATE /W4)
      add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_NON_CONFORMING_SWPRINTFS _SCL_SECURE_NO_WARNINGS)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    else() # GCC or clang
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} lunasvg)