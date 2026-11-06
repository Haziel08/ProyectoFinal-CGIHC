#pragma once
// ModeloAnimado: carga un modelo con huesos (por ejemplo de Mixamo) y reproduce
// sus animaciones (animacion esqueletal).
//
// Como funciona, en 4 pasos:
//   1. Cargar(): con Assimp se lee la malla, y para cada vertice se guardan
//      los 4 huesos que lo mueven y cuanto lo mueve cada uno (pesos).
//   2. Tambien se copia el arbol de huesos (cadera -> columna -> brazo -> mano...).
//   3. CalcularPose(): para el tiempo actual se busca la posicion/rotacion de
//      cada hueso entre dos keyframes y se interpola; luego se recorre el arbol
//      multiplicando por la matriz del padre (igual que el modelado jerarquico).
//   4. Dibujar(): se mandan las matrices de los huesos al shader
//      (shader_esqueleto.vert) y el shader mueve cada vertice segun sus pesos.
#include <string>
#include <vector>
#include <map>
#include <glew.h>
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include "Texture.h"

struct aiNode;

class ModeloAnimado
{
public:
	static const int MAX_HUESOS = 100;          // igual que en shader_esqueleto.vert

	// Malla + esqueleto + textura. La animacion que traiga el archivo queda como la numero 0.
	bool Cargar(const std::string& archivo, const std::string& rutaTextura);
	// Otra animacion del MISMO esqueleto (otro FBX de Mixamo). Regresa su numero.
	int AgregarAnimacion(const std::string& archivo);

	// Calcula la pose de todos los huesos en el segundo indicado (la animacion se repite)
	void CalcularPose(int animacion, float segundos);
	// Manda los huesos al shader activo y dibuja la malla
	void Dibujar(GLuint programaShader);

	~ModeloAnimado();

private:
	// Un cuadro clave (keyframe): en que tick y que valor
	struct ClavePos { float tick; glm::vec3 valor; };
	struct ClaveRot { float tick; glm::quat valor; };

	// Movimiento de un hueso durante la animacion
	struct Canal
	{
		std::vector<ClavePos> posiciones;
		std::vector<ClaveRot> rotaciones;
		std::vector<ClavePos> escalas;
	};

	struct Animacion
	{
		float duracionTicks = 0.0f;
		float ticksPorSegundo = 30.0f;
		std::map<std::string, Canal> canales;   // nombre del hueso -> su movimiento
	};

	// Nodo del arbol de huesos (copia del de Assimp)
	struct Nodo
	{
		std::string nombre;
		glm::mat4 transformacion;               // pose de reposo respecto al padre
		std::vector<int> hijos;
	};

	int CopiarNodo(const aiNode* nodo);
	void CalcularNodo(const Animacion& anim, float tick, int nodo, const glm::mat4& padre);
	glm::mat4 Interpolar(const Canal& canal, float tick) const;
	bool LeerAnimacion(const std::string& archivo);

	std::vector<Nodo> nodos;                    // nodos[0] = raiz
	std::map<std::string, int> indiceHueso;     // nombre -> numero de hueso
	std::vector<glm::mat4> offsets;             // de la malla al espacio de cada hueso
	std::vector<glm::mat4> matricesHuesos;      // resultado que se manda al shader
	glm::mat4 inversaRaiz = glm::mat4(1.0f);
	std::vector<Animacion> animaciones;

	GLuint VAO = 0, VBO = 0, IBO = 0;
	GLsizei numIndices = 0;
	Texture textura;
};
