#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 iTexCoords1;
layout (location = 2) in vec4 iTexCoords2;
layout (location = 3) in vec4 instanceBackColor;
layout (location = 4) in mat4 instancePVMs;

out vec2 TexCoords;
out vec4 backColor;

void main()
{
	vec2 c[4] = vec2[](iTexCoords1.xy, iTexCoords1.zw, iTexCoords2.xy, iTexCoords2.zw);
	TexCoords = c[gl_VertexID];
	backColor = instanceBackColor;
	gl_Position = instancePVMs * vec4(pos, 0.0, 1.0);
}