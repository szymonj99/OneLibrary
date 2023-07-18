# Check if the library is already configured
if(NOT TARGET ${LIBRARY_NAME})

    # Create an imported target for the library
    add_library(${LIBRARY_NAME} INTERFACE)

    # Set properties for the library
    target_include_directories(${LIBRARY_NAME} INTERFACE
        "$<BUILD_INTERFACE:${OneLibrary_INCLUDE_DIRS}>"
        "$<INSTALL_INTERFACE:include>"
    )

    target_link_libraries(${LIBRARY_NAME} INTERFACE
        ${OneLibrary_LIBRARIES}
    )

    # Add dependencies, if any
    foreach(dep ${OneLibrary_DEPENDENCIES})
        target_link_libraries(${LIBRARY_NAME} INTERFACE ${dep})
    endforeach()

endif()

# Provide version information
set_property(TARGET ${LIBRARY_NAME} PROPERTY VERSION ${LIBRARY_VERSION})

# Create an imported target for the library configuration
add_library(${LIBRARY_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})
