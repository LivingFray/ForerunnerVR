
# CMake rules for building the payload DLL

# Collect payload sources
file(GLOB_RECURSE PAYLOAD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/../common/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../common/*.c")

add_library(ForerunnerPayload SHARED ${PAYLOAD_SRC})

# Output the DLL next to the executable
set(PAYLOAD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(ForerunnerPayload PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PAYLOAD_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PAYLOAD_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${PAYLOAD_OUTPUT_DIR}"
    OUTPUT_NAME "Forerunner"
)

set_property(TARGET ForerunnerPayload PROPERTY CXX_STANDARD 20)

target_include_directories(ForerunnerPayload PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
