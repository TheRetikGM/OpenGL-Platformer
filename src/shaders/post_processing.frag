#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec2 ScreenPos;

uniform float radius;
uniform vec2 center_pos;
uniform sampler2D scene;

void main()
{
    // Center to fragment.
    vec2 ctf = ScreenPos - center_pos;
    float dist = ctf.x * ctf.x  + ctf.y * ctf.y;
    if (dist < radius * radius)
        FragColor = texture(scene, TexCoords);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}