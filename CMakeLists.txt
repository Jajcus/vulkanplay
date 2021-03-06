cmake_minimum_required (VERSION 3.1)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCCompilerFlag)

project(vulkanplay)
add_executable(vulkanplay
		src/main.c
		src/model.c
		src/models/plane.c
		src/models/sphere.c
		src/models/tetrahedron.c
		src/models/terrain.c
		src/renderer.c
		src/scene.c
		src/surface.c
		src/vkapi.c
		src/world.c
		)
set(SHADERS src/shaders/main.vert src/shaders/main.frag)

include_directories(${CMAKE_SOURCE_DIR}/src)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

find_package(Threads)

# 'gnu99' is needed instead of c99 for sigaction()
CHECK_C_COMPILER_FLAG("-std=gnu99" STD_GNU99_SUPPORTED)
if(STD_GNU99_SUPPORTED)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif()

CHECK_C_COMPILER_FLAG("-Wall -pedantic" WALL_PEDANTIC_SUPPORTED)
if(WALL_PEDANTIC_SUPPORTED)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
endif()

CHECK_FUNCTION_EXISTS(sinf SINF_IN_STDLIB)
if(NOT SINF_IN_STDLIB)
	unset(RESULT CACHE)
	set(CMAKE_REQUIRED_LIBRARIES -lm)
	CHECK_FUNCTION_EXISTS(sinf SINF_IN_LIBM)
	if(SINF_IN_LIBM)
		set(MATH_LIB m)
	else()
		message(FATAL_ERROR "No sinf() found")
	endif()
	unset(CMAKE_REQUIRED_LIBRARIES)
endif()

find_library(VULKAN_LIB NAMES vulkan vulkan-1 DOC "Vulkan loader library")
if(NOT VULKAN_LIB)
	message(FATAL_ERROR "Vulkan loader not found. In can be found in the Vulkan SDK.")
endif()
set(CMAKE_REQUIRED_LIBRARIES ${VULKAN_LIB})
CHECK_FUNCTION_EXISTS(vkGetInstanceProcAddr VULKAN_LIB_FOUND)
if(NOT VULKAN_LIB_FOUND)
	message(FATAL_ERROR "No vkGetInstanceProcAddr() in ${VULKAN_LIB}")
endif()
CHECK_SYMBOL_EXISTS(vkGetInstanceProcAddr vulkan/vulkan.h VULKAN_HDR_FOUND)
if(NOT VULKAN_HDR_FOUND)
	message(FATAL_ERROR "No or invalid vulkan/vulkan.h")
endif()

find_program(XXD xxd DOC "xxd hexdump generator")
if(XXD)
	message(STATUS "Found xxd: ${XXD}")
else()
	message(FATAL_ERROR "xxd not found. You can find it distributed with VIM.")
endif()
find_program(GLSLANG_VALIDATOR glslangValidator)
if(GLSLANG_VALIDATOR)
	message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")
else()
	message(FATAL_ERROR "glslangValidator not found. It is a part of the Vulkan SDK, or the 'glslang' package.")
endif()

set(PLAT_LIBS)

find_package(XCB COMPONENTS XCB)
if(${XCB_FOUND})
	add_definitions(-DHAVE_XCB=1)
	target_sources(vulkanplay PRIVATE src/platform/plat_xcb.c)
	include_directories(${XCB_XCB_INCLUDE_DIR})
	set(PLAT_LIBS ${PLAT_LIBS} ${XCB_XCB_LIBRARY})
endif()

find_package(Wayland COMPONENTS Client)
if(${Wayland_FOUND})
	add_definitions(-DHAVE_WAYLAND=1)
	target_sources(vulkanplay PRIVATE src/platform/plat_wl.c)
	include_directories(${Wayland_Client_INCLUDE_DIR})
	set(PLAT_LIBS ${PLAT_LIBS} ${Wayland_Client_LIBRARY})
endif()


set(SHADER_C_FILES)
foreach(_file ${SHADERS})
  string(REPLACE "." "_" basename ${_file})
  get_filename_component(basename ${basename} NAME)
  set(spv_file "${basename}.spv")
  set(spv_c_file "${basename}.spv.c")
  unset(basename)
  add_custom_command(
    OUTPUT ${spv_c_file} 
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${_file} -o ${spv_file}
    COMMAND ${XXD} -i ${spv_file} > ${spv_c_file}
    DEPENDS ${_file}
    WORKING_DIRECTORY ${CURRENT_CMAKE_BINARY_DIR}
  ) 
  target_sources(vulkanplay PRIVATE ${spv_c_file})
  #set(SHADER_C_FILES ${SHADER_C_FILES} ${CMAKE_BINARY_DIR}/${spv_c_file})
  unset(spv_file)
  unset(spv_c_file)
endforeach()

target_link_libraries(vulkanplay ${VULKAN_LIB} ${PLAT_LIBS} ${MATH_LIB} ${CMAKE_THREAD_LIBS_INIT})
