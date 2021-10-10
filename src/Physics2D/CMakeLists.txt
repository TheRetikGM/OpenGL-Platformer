file(GLOB Physics2D_src_files "*.cpp")
file(GLOB Physics2D_include_files "include/*.h" "include/*.hpp")

add_library(Physics2D ${Physics2D_src_files} ${Physics2D_include_files})

target_include_directories(Physics2D
	PRIVATE "${CMAKE_SOURCE_DIR}/include"
	"include"
)
