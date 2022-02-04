#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D spriteImage;
uniform vec3 spriteColor;
uniform vec4 spriteScaleOffset;
uniform bool force_color;

void main()
{
	vec2 texCoords = TexCoords * spriteScaleOffset.xy + spriteScaleOffset.zw;
	vec4 t = texture(spriteImage, texCoords);

	if (force_color)
		FragColor = vec4(spriteColor, texture(spriteImage, texCoords).a);
	else
		FragColor = vec4(spriteColor, 1.0) * texture(spriteImage, texCoords);
}