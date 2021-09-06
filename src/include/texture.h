#pragma once

class Texture2D
{
public:
	unsigned int ID;
	unsigned int Width, Height;
	unsigned int Internal_format;
	unsigned int Image_format;

	unsigned int Wrap_S, Wrap_T;
	unsigned int Filter_min, Filter_mag;

	Texture2D();
	~Texture2D();

	void Generate(unsigned int width, unsigned int height, unsigned char* data);

	void Bind() const;
};