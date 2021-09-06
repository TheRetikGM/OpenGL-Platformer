cmake_minimum_required(VERSION 3.13.4)

project("main")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(json-develop)
add_subdirectory("src")

target_link_directories(${PROJECT_NAME} PRIVATE lib)
target_include_directories(${PROJECT_NAME} PRIVATE include PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}	
	Texture
	Shader
	Glad
)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

if (MSVC)
	target_link_libraries(${PROJECT_NAME}
		opengl32
		debug Debug/glfw3d optimized Release/glfw3
		debug Debug/zlibstaticd optimized Release/zlibstatic
		debug Debug/tinyxml2d optimized Release/tinyxml2
		debug Debug/tmxparser optimized Release/tmxparser
	)
endif()