#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Texture >>>>>>>>>>>>>>>>>>>>>>
#include <glew.h>
#include <string>
class Texture
{
public:
	Texture();
	Texture(const char* ruta_archivo);
	bool LoadTexture();
	bool LoadTextureA();
	bool LoadMemory(const unsigned char* bytes, int length);
	bool LoadColor(float red, float green, float blue);
	void UseTexture(GLenum unidad = GL_TEXTURE0);
	void ClearTexture();
	~Texture();
private: 
	GLuint texture_id;
	int width, height, bit_depth;
	std::string file_location;

};

