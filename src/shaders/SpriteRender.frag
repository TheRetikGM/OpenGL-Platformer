#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D spriteImage;
uniform vec3 spriteColor;
uniform vec4 spriteScaleOffset;

void main()
{
	vec2 texCoords = TexCoords * spriteScaleOffset.xy + spriteScaleOffset.zw;
	FragColor = vec4(spriteColor, 1.0) * texture(spriteImage, texCoords);
	// FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}