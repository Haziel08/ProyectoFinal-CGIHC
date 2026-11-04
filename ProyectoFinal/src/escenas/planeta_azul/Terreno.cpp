#include "Terreno.h"

#include <vector>
#include <cmath>
#include <gtc/type_ptr.hpp>

Terreno::~Terreno()
{
	delete tierra;
	delete fondo;
	delete playa;
}

void Terreno::CrearPlaya()
{
	// Anillo alrededor del lago (de 0.97 a 1.12 veces el radio), pegado al suelo.
	// Tapa el borde "en escalera" entre los cuadritos de pasto y de arena.
	const int vueltas = 360, anillos = 6;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i <= vueltas; i++)
	{
		float ang = 2.0f * 3.14159265f * i / vueltas;
		for (int j = 0; j <= anillos; j++)
		{
			float d = 0.97f + 0.15f * j / anillos;
			float x = centroLago.x + radioLago.x * d * cosf(ang);
			float z = centroLago.y + radioLago.y * d * sinf(ang);
			glm::vec3 n = NormalEn(x, z);
			v.insert(v.end(), { x, Altura(x, z) + 0.02f, z,
								x / metrosPorTextura, z / metrosPorTextura,
								-n.x, -n.y, -n.z });
		}
	}
	for (int i = 0; i < vueltas; i++)
	{
		for (int j = 0; j < anillos; j++)
		{
			unsigned int a = i * (anillos + 1) + j, b = a + anillos + 1;
			idx.insert(idx.end(), { a, b, a + 1,  a + 1, b, b + 1 });
		}
	}
	playa = new MeshModel();
	playa->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());
}

float Terreno::DistanciaLago(float x, float z) const
{
	// Distancia "ovalada": vale 1 justo en la orilla del lago
	float dx = (x - centroLago.x) / radioLago.x;
	float dz = (z - centroLago.y) / radioLago.y;
	return sqrtf(dx * dx + dz * dz);
}

bool Terreno::EsLago(float x, float z) const
{
	return DistanciaLago(x, z) < 1.06f;   // un poquito mas que la orilla = playa de arena
}

float Terreno::Altura(float x, float z) const
{
	float d = DistanciaLago(x, z);

	if (d < 1.0f)
	{
		// Dentro del lago: baja suave desde la orilla y luego queda un fondo casi plano
		float t = glm::smoothstep(1.0f, 0.55f, d);
		float ondulacionFondo = 0.8f * sinf(x * 0.15f) * sinf(z * 0.12f);
		return -profundidad * t + ondulacionFondo * t;
	}

	// Tierra: colinas suaves que crecen al alejarse del lago (cerca de la orilla es plano)
	float colinas = 2.0f + 1.5f * sinf(x * 0.035f) * sinf(z * 0.03f + 1.0f) + 0.5f * sinf(x * 0.11f + z * 0.07f);
	return colinas * glm::smoothstep(1.0f, 1.6f, d);
}

glm::vec3 Terreno::NormalEn(float x, float z) const
{
	// Pendiente por diferencias: cuanto cambia la altura al moverse un poco en X y Z
	float e = 0.5f;
	float hx = (Altura(x + e, z) - Altura(x - e, z)) / (2.0f * e);
	float hz = (Altura(x, z + e) - Altura(x, z - e)) / (2.0f * e);
	return glm::normalize(glm::vec3(-hx, 1.0f, -hz));
}

void Terreno::Crear()
{
	std::vector<GLfloat> vTierra, vFondo;
	std::vector<unsigned int> iTierra, iFondo;

	float lado = tamano / celdas;
	float inicio = -tamano / 2.0f;

	// Agrega un vertice: posicion, UV (segun metros) y normal INVERTIDA (convencion del shader)
	auto agregarVertice = [&](std::vector<GLfloat>& v, float x, float z)
	{
		glm::vec3 n = NormalEn(x, z);
		v.insert(v.end(), { x, Altura(x, z), z,
							x / metrosPorTextura, z / metrosPorTextura,
							-n.x, -n.y, -n.z });
	};

	for (int i = 0; i < celdas; i++)
	{
		for (int j = 0; j < celdas; j++)
		{
			float x0 = inicio + i * lado, z0 = inicio + j * lado;
			float x1 = x0 + lado, z1 = z0 + lado;

			// Cada cuadrito va a la malla de pasto o a la de arena segun donde cae
			bool lago = EsLago((x0 + x1) * 0.5f, (z0 + z1) * 0.5f);
			std::vector<GLfloat>& v = lago ? vFondo : vTierra;
			std::vector<unsigned int>& idx = lago ? iFondo : iTierra;

			unsigned int b = (unsigned int)(v.size() / 8);
			agregarVertice(v, x0, z0);
			agregarVertice(v, x1, z0);
			agregarVertice(v, x1, z1);
			agregarVertice(v, x0, z1);
			idx.insert(idx.end(), { b, b + 1, b + 2,  b + 2, b + 3, b });
		}
	}

	tierra = new MeshModel();
	tierra->CreateMeshModel(vTierra.data(), iTierra.data(), (unsigned int)vTierra.size(), (unsigned int)iTierra.size());
	fondo = new MeshModel();
	fondo->CreateMeshModel(vFondo.data(), iFondo.data(), (unsigned int)vFondo.size(), (unsigned int)iFondo.size());
	CrearPlaya();

	texturaPasto = Texture("assets/planeta_azul/texturas/pasto.jpg");
	texturaPasto.LoadTextureA();
	texturaArena = Texture("assets/planeta_azul/texturas/arena.jpg");
	texturaArena.LoadTextureA();
}

void Terreno::Dibujar(const ContextoRender& ctx)
{
	glm::mat4 model(1.0f);                        // el terreno ya esta en coordenadas del mundo
	glm::vec3 blanco(1.0f);
	glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.uColor, 1, glm::value_ptr(blanco));
	materialSuelo.UseMaterial(ctx.uSpecularIntensity, ctx.uShininess);

	texturaPasto.UseTexture();
	tierra->RenderMeshModel();

	texturaArena.UseTexture();
	fondo->RenderMeshModel();

	// La playa va encima del suelo; el "offset" evita que parpadee con el pasto
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	playa->RenderMeshModel();
	glDisable(GL_POLYGON_OFFSET_FILL);
}
