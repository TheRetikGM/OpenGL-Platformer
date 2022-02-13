#version 330 core

layout(location = 0) in vec4 aVertex;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform vec4 offset_scale;  // XY offset, ZW scale
uniform vec4 color;

void main()
{
    TexCoords = aVertex.zw;
    ParticleColor = color;
    gl_Position = projection * vec4((aVertex.xy * offset_scale.zw) + offset_scale.xy, 0.0, 1.0);
}