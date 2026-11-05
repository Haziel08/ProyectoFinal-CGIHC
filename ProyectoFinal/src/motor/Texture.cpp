//>>>>>>>>>>>>>>>>>>>>>> Texture >>>>>>>>>>>>>>>>>>>>>>
#include "Texture.h"
#include "CommonValues.h"

Texture::Texture()
{
	texture_id = 0;
	width = 0;
	height = 0;
	bit_depth = 0;
	file_location = "";
}
Texture::Texture(const char *ruta_archivo)
{
	texture_id = 0;
	width = 0;
	height = 0;
	bit_depth = 0;
	file_location = ruta_archivo;
}

bool Texture::LoadTextureA()
{
	//para cambiar el origen a la esquina inferior izquierda como necesitamos
	stbi_set_flip_vertically_on_load(true);
	unsigned char *tex_data = stbi_load(file_location.c_str(), &width, &height, &bit_depth,STBI_rgb_alpha); //el tipo unsigned char es para un array de bytes de la imagen, obtener datos de la imagen 
	if (!tex_data)
	{
		printf("No se encontró el archivo: %s\n", file_location.c_str());
		return false;
	}
	glGenTextures(1, &texture_id); //parecido al vao: crear una textura y asignarle un índice
	glBindTexture(GL_TEXTURE_2D, texture_id);//se indica que la textura es de tipo 2D, para superficies planas es suficiente esta textura
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// eje S paralelo a X, repetir sobre el eje
/*	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);// eje S paralelo a X, repetir sobre el eje pero rotando con forme a un centro
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// eje S paralelo a X, envolver toda la superficie
*/
	//>>>>>>>>>>>>>>>>>>>>>> Repeticion de paneles de pista >>>>>>>>>>>>>>
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		file_location.find("pista_recta.png") != std::string::npos ? GL_MIRRORED_REPEAT : GL_REPEAT);
	/*GL_TEXTURE_MIN_FILTER: Para más cerca o textura se escala a menor tamaño. GL_TEXTURE_MAG_FILTER: Para más lejos o textura se escala a mayor tamaño. 
	GL_LINEAR  aplica sampling y blending de texels más cercanos. GL_NEAREST aplica sample de texel más cercano
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		(file_location.find("pista_") != std::string::npos || file_location.find("paneles_solares.png") != std::string::npos || file_location.find("crater_luna.png") != std::string::npos || file_location.find("roca_piso_") != std::string::npos) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);//para hacer un unbind de la textura
	stbi_image_free(tex_data); //para liberar la información de la imagen
	return true;
}
bool Texture::LoadTexture()
{
	//para cambiar el origen a la esquina inferior izquierda como necesitamos
	stbi_set_flip_vertically_on_load(true);
	unsigned char *tex_data = stbi_load(file_location.c_str(), &width, &height, &bit_depth, STBI_rgb); //el tipo unsigned char es para un array de bytes de la imagen, obtener datos de la imagen 
	if (!tex_data)
	{
		printf("No se encontró el archivo: %s\n", file_location.c_str());
		return false;
	}
	glGenTextures(1, &texture_id); //parecido al vao: crear una textura y asignarle un índice
	glBindTexture(GL_TEXTURE_2D, texture_id);//se indica que la textura es de tipo 2D, para superficies planas es suficiente esta textura

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// eje S paralelo a X, repetir sobre el eje
/*	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);// eje S paralelo a X, repetir sobre el eje pero rotando con forme a un centro
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// eje S paralelo a X, envolver toda la superficie
*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// eje T paralelo a Y, repetir sobre el eje
	/*GL_TEXTURE_MIN_FILTER: Para más cerca o textura se escala a menor tamaño. GL_TEXTURE_MAG_FILTER: Para más lejos o textura se escala a mayor tamaño.
	GL_LINEAR  aplica sampling y blending de texels más cercanos. GL_NEAREST aplica sample de texel más cercano
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		file_location.find("suelo_naranja.jpg") != std::string::npos ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// stb_image entrega filas RGB consecutivas, sin relleno entre ellas.
	GLint alineacion_anterior;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alineacion_anterior);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, alineacion_anterior);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);//para hacer un unbind de la textura
	stbi_image_free(tex_data); //para liberar la información de la imagen
	return true;
}
bool Texture::LoadMemory(const unsigned char* bytes, int length)
{
	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixels = stbi_load_from_memory(bytes, length, &width, &height, &bit_depth, STBI_rgb_alpha);
	if (!pixels) return false;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(pixels);
	return true;
}

bool Texture::LoadColor(float red, float green, float blue)
{
	unsigned char pixel[] = { static_cast<unsigned char>(red * 255.0f),
		static_cast<unsigned char>(green * 255.0f), static_cast<unsigned char>(blue * 255.0f), 255 };
	width = height = 1;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	return true;
}

void Texture::ClearTexture()
{

	glDeleteTextures(1, &texture_id);
	texture_id = 0;
	width = 0;
	height = 0;
	bit_depth = 0;
	file_location = "";
}
void Texture::UseTexture()
{	//UnitTexture
	glActiveTexture(GL_TEXTURE0); //para crear un sampler que es lo que necesitan los shaders para poder acceder a la textura: 16 a 32 texturas pueden ser declaradas
	//si hay mas de 1 unittexture se tiene que crear una unifromvariable que haga cambio entre la unit texture a utilizar
	glBindTexture(GL_TEXTURE_2D, texture_id);

}

Texture::~Texture()
{
	ClearTexture();
}
