# CMakeList.txt : CMake project for Forerunner, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Forerunner")

# Project Source
file(GLOB_RECURSE FORERUNNER_SRC src/*.cpp)

# IMGUI
file(GLOB IMGUI_SRC 
	lib/imgui/*.h
	lib/imgui/*.cpp
	lib/imgui/backends/imgui_impl_dx11.*
	lib/imgui/backends/imgui_impl_win32.*
)


# Add source to this project's executable.
#add_executable (Forerunner WIN32 "src/launcher/main.cpp" resources/forerunner.rc "src/launcher/launcher.cpp"  "src/launcher/launcher.h")
add_executable (Forerunner WIN32 resources/forerunner.rc ${FORERUNNER_SRC} ${IMGUI_SRC})

set_property(TARGET Forerunner PROPERTY CXX_STANDARD 20)

target_link_libraries(Forerunner PRIVATE 
	d3d11.lib
)

target_include_directories(Forerunner PRIVATE
	lib/imgui
)
