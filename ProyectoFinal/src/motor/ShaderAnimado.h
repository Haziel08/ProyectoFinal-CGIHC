#pragma once
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include <string>
#include <vector>

//>>>>>>>>>>>>>>>>>>>>>> ShaderAnimado >>>>>>>>>>>>>>>>>>>>>>
// Uniforms del agua y de las llamas.

class ShaderAnimado
{
public:
	bool Crear(const char* ruta_vert, const char* ruta_frag);   // archivos de shaders/
    ~ShaderAnimado() { if (programa) glDeleteProgram(programa); }
	void Usar() const { glUseProgram(programa); }

	// Mandar valores a los uniform del shader
	void SetInt(const char* nombre, int valor) const;
	void SetFloat(const char* nombre, float valor) const;
	void SetVec2(const char* nombre, const glm::vec2& valor) const;
	void SetVec3(const char* nombre, const glm::vec3& valor) const;
	void SetMat4(const char* nombre, const glm::mat4& valor) const;

private:
	GLuint programa = 0;
	static GLuint Compilar(GLenum tipo, const std::string& codigo, const char* ruta);
};
