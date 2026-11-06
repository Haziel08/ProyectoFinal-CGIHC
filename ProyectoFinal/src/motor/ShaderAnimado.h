#pragma once
// ShaderAnimado: carga shaders/shader_animado.vert y .frag para agua y fuego
// y permite mandarles valores por nombre: SetFloat("amplitud", 0.3f), etc.
#include <string>
#include <glew.h>
#include <glm.hpp>

class ShaderAnimado
{
public:
	bool Crear(const char* rutaVert, const char* rutaFrag);
	void Usar() const { glUseProgram(programa); }

	// Mandar valores a los uniform del shader
	void SetInt(const char* nombre, int valor) const;
	void SetFloat(const char* nombre, float valor) const;
	void SetVec2(const char* nombre, const glm::vec2& valor) const;
	void SetVec3(const char* nombre, const glm::vec3& valor) const;
	void SetMat4(const char* nombre, const glm::mat4& valor) const;

private:
	GLuint programa = 0;
	static std::string LeerArchivo(const char* ruta);
	static GLuint Compilar(GLenum tipo, const std::string& codigo, const char* ruta);
};
