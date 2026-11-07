#include "ShaderAnimado.h"
#include <cstdio>
#include <fstream>
#include <sstream>

//>>>>>>>>>>>>>>>>>>>>>> ShaderAnimado >>>>>>>>>>>>>>>>>>>>>>


GLuint ShaderAnimado::Compilar(GLenum tipo, const std::string& codigo, const char* ruta)
{
	GLuint s = glCreateShader(tipo);
	const char* fuente = codigo.c_str();
	glShaderSource(s, 1, &fuente, nullptr);
	glCompileShader(s);

	GLint ok = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[1024];
		glGetShaderInfoLog(s, sizeof(log), nullptr, log);
		printf("Error compilando %s:\n%s\n", ruta, log);
	}
	return s;
}

// Lee todo el texto de un archivo
static std::string LeerArchivo(const char* ruta)
{
	std::ifstream archivo(ruta);
	if (!archivo)
		printf("No se encontro el shader %s\n", ruta);
	std::stringstream texto;
	texto << archivo.rdbuf();
	return texto.str();
}

bool ShaderAnimado::Crear(const char* ruta_vert, const char* ruta_frag)
{
	GLuint vs = Compilar(GL_VERTEX_SHADER, LeerArchivo(ruta_vert), ruta_vert);
	GLuint fs = Compilar(GL_FRAGMENT_SHADER, LeerArchivo(ruta_frag), ruta_frag);

	programa = glCreateProgram();
	glAttachShader(programa, vs);
	glAttachShader(programa, fs);
	glLinkProgram(programa);
	glDeleteShader(vs);
	glDeleteShader(fs);

	GLint ok = 0;
	glGetProgramiv(programa, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[1024];
		glGetProgramInfoLog(programa, sizeof(log), nullptr, log);
		printf("Error enlazando shader animado:\n%s\n", log);
		return false;
	}
	return true;
}

void ShaderAnimado::SetInt(const char* nombre, int valor) const
{
	glUniform1i(glGetUniformLocation(programa, nombre), valor);
}

void ShaderAnimado::SetFloat(const char* nombre, float valor) const
{
	glUniform1f(glGetUniformLocation(programa, nombre), valor);
}

void ShaderAnimado::SetVec2(const char* nombre, const glm::vec2& valor) const
{
	glUniform2fv(glGetUniformLocation(programa, nombre), 1, glm::value_ptr(valor));
}

void ShaderAnimado::SetVec3(const char* nombre, const glm::vec3& valor) const
{
	glUniform3fv(glGetUniformLocation(programa, nombre), 1, glm::value_ptr(valor));
}

void ShaderAnimado::SetMat4(const char* nombre, const glm::mat4& valor) const
{
	glUniformMatrix4fv(glGetUniformLocation(programa, nombre), 1, GL_FALSE, glm::value_ptr(valor));
}
