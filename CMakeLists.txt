# CMakeList.txt : CMake project for pavuk_engine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("pavuk_engine")

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include_directories(${Vulkan_INCLUDE_DIRS})
#include_directories(${PROJECT_SOURCE_DIR}/include)
#include_directories(${PROJECT_SOURCE_DIR}/glfw/include)
#include_directories(${PROJECT_SOURCE_DIR})

IF(WIN32)
	# Nothing here (yet)
ELSEIF(APPLE)
	link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(WIN32)
	link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)

# Create an empty list to store source files
set(SOURCE_FILES "")

# Add all .cpp files in the src directory to the list
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Add all .hpp files in the include directory to the list
file(GLOB_RECURSE HDR_FILES "src/*.h")
file(GLOB_RECURSE INC_FILES "include/*.h")

# Create the executable or library target
add_executable(pavuk_engine ${SRC_FILES} ${HDR_FILES} ${INC_FILES})

# Include the 'include' directory
target_include_directories(pavuk_engine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(pavuk_engine PRIVATE ${PROJECT_SOURCE_DIR}/glfw/include)
target_include_directories(pavuk_engine PRIVATE ${PROJECT_SOURCE_DIR}/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

target_link_libraries (pavuk_engine glfw)
target_link_libraries(pavuk_engine Vulkan::Vulkan)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET pavuk_engine PROPERTY CXX_STANDARD 17)
endif()

set_property(TARGET pavuk_engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
message(${CMAKE_SOURCE_DIR})
