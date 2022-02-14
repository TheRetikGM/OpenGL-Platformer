#version 330 core

layout(location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec2 ScreenPos;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    ScreenPos = vec2(model * vec4(vertex.xy, 0.0f, 1.0f));
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}