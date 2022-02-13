#version 330 core

layout(location = 0) in vec4 aVertex;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 model;
uniform vec4 color;

void main()
{
    TexCoords = aVertex.zw;
    ParticleColor = color;
    gl_Position = projection * model * vec4(aVertex.xy, 0.0, 1.0);
}