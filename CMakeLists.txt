cmake_minimum_required(VERSION 3.21)

# Project name
project(GL-compute VERSION 0.0.1)

# Set the executable name as the project name and enforce lower case.
string(TOLOWER ${CMAKE_PROJECT_NAME} EXECUTABLE_NAME)

# Dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Add the sub-directories
#add_subdirectory(lib)
add_subdirectory(src)


message(STATUS "SRC : " ${SRC})
message(STATUS "GL_SRC : " ${GL_SRC})
message(STATUS "LIB_SRC : " ${LIB_SRC})
message(STATUS "UTILS_SRC : " ${UTILS_SRC})

# Add the executable
add_executable(${EXECUTABLE_NAME} 
    ${SRC}
)

# Set target C++ standard and runtime directory
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED True
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


target_compile_options(${EXECUTABLE_NAME} PRIVATE
    #-Wall
    #-Wextra
    #-Wpedantic
    #-Werror
)


target_link_libraries(${EXECUTABLE_NAME} PUBLIC
    OpenGL::GL
    glfw
    GLEW
    pthread
)


