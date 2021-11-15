#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 TextColor;
uniform sampler2D textBitmap;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textBitmap, TexCoords).r);
	FragColor = vec4(TextColor, 1.0) * sampled;	
}