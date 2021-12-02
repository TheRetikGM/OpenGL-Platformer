cmake_minimum_required(VERSION 3.13.4)

project("main")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX "d")

# DOWNLOAD SUBMODULES
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	option(GIT_SUBMODULE "Check submodules during build" ON)
	if (GIT_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if (NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMODULE_RESULT}.")
		endif()
	endif()
endif()

# CHECK ALL THE SUBMODULES
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/Physics2D/CMakeLists.txt")
	message(FATAL_ERROR "The Physics2D submodule was not downloaded!")
endif()

add_subdirectory(json-develop)
add_subdirectory("src")
add_subdirectory("external/Physics2D")

target_link_directories(${PROJECT_NAME} PRIVATE lib)
target_include_directories(${PROJECT_NAME} 
	PRIVATE include 
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
	PRIVATE external/Physics2D/include
)
target_link_libraries(${PROJECT_NAME}	
	Texture
	Shader
	Glad
	Physics2D
)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

if (MSVC)
	target_link_libraries(${PROJECT_NAME}
		opengl32
		debug Debug/glfw3d optimized Release/glfw3
		debug Debug/zlibstaticd optimized Release/zlibstatic
		debug Debug/tinyxml2d optimized Release/tinyxml2
		debug Debug/tmxparser optimized Release/tmxparser
		debug Debug/freetyped optimized Release/freetype
	)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME}
        glfw
        GL
        X11
        pthread
        Xrandr
        Xi
        dl
        m
        tinyxml2
        z
        debug tmxparserd optimized tmxparser
	freetype
    )
endif()
