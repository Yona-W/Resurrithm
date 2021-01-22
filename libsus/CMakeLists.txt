set(PROJECT_NAME libsus)

################################################################################
# Source groups
################################################################################
set(_
    "exports.def"
    "main.cpp"
)
source_group("Source files" FILES ${_})

set(_
    "libsus.hpp"
)
source_group("Header files" FILES ${_})

set(ALL_FILES
    ${_}
    ${_}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE libsus)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_DIRECTORY_RELEASE   "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/")

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

