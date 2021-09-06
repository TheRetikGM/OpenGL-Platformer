#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D tilesetImage;
in vec4 backColor;

void main()
{
	if (backColor.x != 1.0)	
		FragColor = texture(tilesetImage, TexCoords);	
	else
		FragColor = vec4(backColor.yzw, 1.0);
	//FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}