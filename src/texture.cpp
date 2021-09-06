#include "Texture.h"
#include <exception>
#include <glad/glad.h>
#include <cstdio>

Texture2D::Texture2D()
	: Width(0), Height(0), Internal_format(GL_RGB), Image_format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_min(GL_LINEAR), Filter_mag(GL_LINEAR)
{
	glGenTextures(1, &this->ID);
}
Texture2D::~Texture2D()
{	
}
unsigned int get_nearest_multiple_two(unsigned int n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char* data)
{
	this->Width = get_nearest_multiple_two(width);
	this->Height = get_nearest_multiple_two(height);

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, Internal_format, width, height, 0, Image_format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_mag);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}