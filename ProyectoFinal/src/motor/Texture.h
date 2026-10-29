#pragma once
#include <glew.h>
#include <string>
class Texture
{
public:
	Texture();
	Texture(const char* FileLoc);
	bool LoadTexture();
	bool LoadTextureA();
	bool LoadMemory(const unsigned char* bytes, int length);
	bool LoadColor(float red, float green, float blue);
	void UseTexture();
	void ClearTexture();
	~Texture();
private: 
	GLuint textureID;
	int width, height, bitDepth;
	std::string fileLocation;

};

