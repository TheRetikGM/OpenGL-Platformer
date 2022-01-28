#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D spriteImage;
uniform vec3 spriteColor;
uniform vec4 spriteScaleOffset;

void main()
{
	vec2 texCoords = TexCoords * spriteScaleOffset.xy + spriteScaleOffset.zw;
	vec4 t = texture(spriteImage, texCoords);
	if (spriteColor.rgb == vec3(1.0, 1.0, 1.0))
		FragColor = vec4(spriteColor, 1.0) * texture(spriteImage, texCoords);
	else
		FragColor = vec4(spriteColor, t.a);
	// FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}