#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection, model;
uniform bool inverse_tex_x, inverse_tex_y;

void main()
{
	TexCoords = vertex.zw;

	if (inverse_tex_x)
		TexCoords.x = 1.0 - TexCoords.x;
	if (inverse_tex_y)
		TexCoords.y = 1.0 - TexCoords.y;

	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}