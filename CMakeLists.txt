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
# Only collect launcher / top-level application sources here (do not include payload sources)
file(GLOB_RECURSE FORERUNNER_SRC src/launcher/*.cpp src/common/*.cpp)

# IMGUI
file(GLOB IMGUI_SRC 
	lib/imgui/*.h
	lib/imgui/*.cpp
	lib/imgui/backends/imgui_impl_dx11.*
	lib/imgui/backends/imgui_impl_win32.*
)

# MinHook
file (GLOB MINHOOK_SRC
	lib/minhook/src/*.cpp
)


# Add source to this project's executable.
add_executable (Forerunner WIN32 resources/forerunner.rc ${FORERUNNER_SRC} ${IMGUI_SRC} ${MINHOOK_SRC})


# Put the executable output into the shared runtime folder
set_target_properties(Forerunner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# On Visual Studio, mark the launcher exe as the startup project so the IDE will launch it
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Forerunner)

# Build payload DLL (placed into the same output folder)
add_subdirectory(src/payload)

# Ensure the launcher executable will build the payload DLL first
add_dependencies(Forerunner ForerunnerPayload)

# Provide debugger command and working directory for Visual Studio
set_target_properties(Forerunner PROPERTIES
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:Forerunner>"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_property(TARGET Forerunner PROPERTY CXX_STANDARD 20)

target_link_libraries(Forerunner PRIVATE 
	d3d11.lib
	${CMAKE_SOURCE_DIR}/lib/openvr/openvr_api.lib
)

target_include_directories(Forerunner PRIVATE
	lib/imgui
	lib/minhook/include
	lib/openvr
	lib/json
)

target_include_directories(Forerunner PUBLIC
	src
)

# Copy openvr.dll to same directory as exe
add_custom_command(
        TARGET Forerunner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/openvr/openvr_api.dll
                ${CMAKE_BINARY_DIR}/bin/openvr_api.dll)

# Copy VR resources to same directory as exe
add_custom_command(
        TARGET Forerunner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${CMAKE_SOURCE_DIR}/resources/VR
                ${CMAKE_CURRENT_BINARY_DIR}/bin/VR)