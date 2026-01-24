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

# Add source to this project's executable.
add_executable (Forerunner WIN32 "src/launcher/main.cpp" resources/forerunner.rc "src/launcher/launcher.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Forerunner PROPERTY CXX_STANDARD 20)
endif()


target_link_libraries(Forerunner PRIVATE 
	d3d11.lib
)

# TODO: Add tests and install targets if needed.
