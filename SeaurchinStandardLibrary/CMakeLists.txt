set(PROJECT_NAME SeaurchinStandardLibrary)

################################################################################
# Source groups
################################################################################
set(
    "Random.cpp"
    "Random.h"
)
source_group("Class files" FILES ${})

set(_
    "exports.def"
    "Main.cpp"
)
source_group("Source files" FILES ${_})

set(_
    "../Seaurchin/SeaurchinExtension.h"
    "Main.h"
)
source_group("Header files" FILES ${_})

set(ALL_FILES
    ${}
    ${_}
    ${_}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(ROOT_NAMESPACE SeaurchinStandardLibrary)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_DIRECTORY_RELEASE   "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/")
################################################################################
# Include directories
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    "../library/angelscript/angelscript/include"
)

################################################################################
# Compile and link options
################################################################################

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        -Og
    >
    $<$<CONFIG:Release>:
        /O3;
    >
    ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
    ${DEFAULT_CXX_EXCEPTION_HANDLING}
)