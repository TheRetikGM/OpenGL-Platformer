#version 330 core

layout(location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec2 ScreenPos;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    ScreenPos = vertex.xy;
    gl_Position = vec4(vertex.xy, 0.0, 1.0);
}