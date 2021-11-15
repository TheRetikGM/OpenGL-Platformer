file(GLOB Physics2D_src_files "*.cpp")
file(GLOB Physics2D_include_files "include/*.h" "include/*.hpp")
file(GLOB Physics2D_src_colliders "colliders/*.cpp")
file(GLOB Physics2D_include_colliders "include/colliders/*.h")

add_library(Physics2D 
	${Physics2D_src_files} ${Physics2D_include_files}
	${Physics2D_src_colliders} ${Physics2D_include_colliders}
)

target_include_directories(Physics2D
	PRIVATE "${CMAKE_SOURCE_DIR}/include"
	"include"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "Header Files" FILES ${Physics2D_include_colliders})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${Physics2D_src_colliders})
