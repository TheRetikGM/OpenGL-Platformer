set(SRC_DIR ${CMAKE_SOURCE_DIR})
configure_file(config.h.in config.h @ONLY)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/src/Physics2D/include
)

# Texture
add_library(Texture texture.cpp include/texture.h)

# Shader
add_library(Shader shader.cpp include/shader.h)

# Glad
add_library(Glad glad.c ../include/glad/glad.h)

# Physics2D
add_subdirectory(Physics2D)

# Main executable
file(GLOB SHADERS
	"shaders/*.frag"
	"shaders/*.vert"
	"shaders/*.geom"
)
set (Camera	
	camera/tileCamera2D.cpp include/camera/tileCamera2D.h
)
set (INTERFACES
	include/camera/Icamera.h
	include/IAnimated.h
	include/Interfaces/ITileSpace.h
)
file(GLOB game_files "game/*.cpp")
file(GLOB game_includes "include/game/*.h" "include/game/*.hpp")

add_executable(${PROJECT_NAME}
	main.cpp
	stb_image.cpp ../include/stb_image.h
	stb_image_write.cpp ../include/stb_image_write.h
	resource_manager.cpp include/resource_manager.h
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	${game_files} ${game_includes}
	tilemap.cpp include/tilemap.h
	tilemap_renderer.cpp include/tilemap_renderer.h
	sprite_renderer.cpp include/sprite_renderer.h
	basic_renderer.cpp include/basic_renderer.h
	${Camera}
	"${SHADERS}"
	${INTERFACES}
	include/Helper.hpp
)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "" FILES ${SHADERS})

source_group(TREE ${CMAKE_SOURCE_DIR}/src/include PREFIX "Header Files" FILES 
	${game_includes}
	include/Helper.hpp)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files" FILES ${game_files})

source_group(TREE ${CMAKE_SOURCE_DIR}/src/include PREFIX "Header Files/Renderers" FILES include/tilemap_renderer.h include/sprite_renderer.h include/basic_renderer.h)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files/Renderers" FILES tilemap_renderer.cpp sprite_renderer.cpp basic_renderer.cpp)

source_group(TREE ${CMAKE_SOURCE_DIR}/src/include PREFIX "Header Files/Interfaces" FILES ${INTERFACES})

source_group(TREE ${CMAKE_SOURCE_DIR}/include PREFIX "Header Files/External" FILES ../include/stb_image.h ../include/stb_image_write.h)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files/External" FILES stb_image.cpp stb_image_write.cpp)
