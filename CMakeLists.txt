cmake_minimum_required (VERSION 3.5)

project (vulkan_forward_plus)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(EXTERNAL "lib")
include_directories("${EXTERNAL}/tinyobjloader")
include_directories("${EXTERNAL}/glfw-3.2.1.bin.WIN64/include")
include_directories("${EXTERNAL}/glm")
include_directories("${EXTERNAL}/stb")
include_directories("$ENV{VULKAN_SDK}/Include")

# Configure GLFW
# set(GLFW_ROOT_DIR "${EXTERNAL}/glfw-3.2.1")
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# add_subdirectory(${GLFW_ROOT_DIR})
# include_directories("${GLFW_ROOT_DIR}/include")

# Find our Vulkan!
# find_package(Vulkan REQUIRED)

# Note: trying to write files in dependency order
set(SOURCE_FILES
    "src/main.cpp"
    "src/VDeleter.h"
    "src/camera.h"
    "src/VulkanBaseApplication.h"
    "src/VulkanTools.cpp"
    "src/VulkanBaseApplication.cpp"
    )

# IF(MSVC)
#     # make it looks better in msvc
#     foreach(source IN LISTS SOURCE_FILES)
#         get_filename_component(source_path "${source}" PATH)
#         string(REPLACE "/" "\\" source_path_msvc "${source_path}")
#         source_group("${source_path_msvc}" FILES "${source}")
#     endforeach()
# ENDIF(MSVC)

set(LINK_DIRECTORIES
	"${EXTERNAL}/glfw-3.2.1.bin.WIN64/lib-vc2015/"
	"$ENV{VULKAN_SDK}/Bin/"
)

link_directories(${LINK_DIRECTORIES})

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

set(LINK_LIBRARIES
	"glfw3"
	"vulkan-1"
)

target_link_libraries(${CMAKE_PROJECT_NAME} ${LINK_LIBRARIES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/")
