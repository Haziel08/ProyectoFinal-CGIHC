#include "Agua.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Agua >>>>>>>>>>>>>>>>>>>>>>


Agua::~Agua()
{
	delete malla;
}

void Agua::Crear(glm::vec2 centro, glm::vec2 tamano, float nivel_y, const char* ruta_textura)
{
	// Malla de muchos cuadritos (las olas mueven cada vertice)
	const int div = 160;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i <= div; i++)
	{
		for (int j = 0; j <= div; j++)
		{
			float u = (float)i / div, w = (float)j / div;
			float x = centro.x + (u - 0.5f) * tamano.x;
			float z = centro.y + (w - 0.5f) * tamano.y;
			v.insert(v.end(), { x, nivel_y, z,  u, w,  0.0f, 1.0f, 0.0f });
		}
	}
	for (int i = 0; i < div; i++)
	{
		for (int j = 0; j < div; j++)
		{
			unsigned int a = i * (div + 1) + j, b = a + div + 1;
			idx.insert(idx.end(), { a, b, a + 1,  a + 1, b, b + 1 });
		}
	}
	malla = new MeshModel();
	malla->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());

	repeticion = tamano.x / 20.0f;    // la textura se repite cada 20 unidades

	textura = Texture(ruta_textura);
	textura.LoadTextureA();
	shader.Crear("shaders/shader_animado.vert", "shaders/shader_animado.frag");
}

void Agua::Dibujar(const ContextoRender& ctx, float tiempo, const DatosLuz& luz)
{
	shader.Usar();

	// Camara
	shader.SetMat4("model", glm::mat4(1.0f));
	shader.SetMat4("view", ctx.vista);
	shader.SetMat4("projection", ctx.proyeccion);
	shader.SetVec3("pos_camara", ctx.pos_camara);

	// Movimiento
	shader.SetFloat("tiempo", tiempo);
	shader.SetFloat("amplitud", amplitud);
	shader.SetFloat("frecuencia", frecuencia);
	shader.SetFloat("velocidad", velocidad);
	shader.SetVec2("velocidad_u_v", velocidad_u_v);
	shader.SetFloat("repeticion_u_v", repeticion);

	// Color y luz
	shader.SetVec3("color_base", color_base);
	shader.SetFloat("transparencia", transparencia);
	shader.SetFloat("desvanecer", 0.0f);             
	shader.SetInt("usar_luz", 1);
	shader.SetVec3("luz_direccion", luz.direccion);
	shader.SetVec3("luz_color", luz.color);
	shader.SetFloat("luz_ambiental", luz.ambiental);
	shader.SetFloat("luz_difusa", luz.difusa);
	shader.SetFloat("brillo", 64.0f);

	// Transparencia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);      // el agua no tapa lo que se dibuje despues

	textura.UseTexture();
	malla->RenderMeshModel();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
}
