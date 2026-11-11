#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Recursos y componentes del planeta azul >>>>>>>>>>>>>>>>>>>>>>
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>
namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> ShaderAnimado >>>>>>>>>>>>>>>>>>>>>>
// Uniforms del agua y de las llamas.

class ShaderAnimado
{
public:
	bool Crear(const char* codigo_vert, const char* codigo_frag);
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
//>>>>>>>>>>>>>>>>>>>>>> ModeloAnimado >>>>>>>>>>>>>>>>>>>>>>
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


extern const char* esqueleto_vert;

class ModeloAnimado
{
public:
	static const int MAX_HUESOS = 100;          // igual que en shader_esqueleto.vert

	// Malla + esqueleto + textura. La animacion que traiga el archivo queda como la numero 0.
	bool Cargar(const std::string& archivo, const std::string& ruta_textura, bool normalizar = false);
    glm::mat4 MarcoHueso(const std::string& nombre) const;
    const glm::mat4& Normalizacion() const { return normalizacion; }
	// Otra animacion del MISMO esqueleto (otro FBX de Mixamo). Regresa su numero.
	int AgregarAnimacion(const std::string& archivo);

	// Si la animacion avanza sola (en Mixamo no se marco "In Place"), deja ese hueso
	// en su lugar en X y Z; asi el avance lo controla nuestro codigo y no "brinca"
	// al repetirse. Para Mixamo el hueso es "mixamorig:Hips" (la cadera).
	void QuitarAvance(int animacion, const std::string& hueso);

	// Calcula la pose de todos los huesos en el segundo indicado (la animacion se repite)
	void CalcularPose(int animacion, float segundos);
	// Manda los huesos al shader activo y dibuja la malla
	void Dibujar(GLuint programa_shader);

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
		float duracion_ticks = 0.0f;
		float ticks_por_segundo = 30.0f;
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
	std::map<std::string, int> indice_hueso;     // nombre -> numero de hueso
	std::vector<glm::mat4> offsets;             // de la malla al espacio de cada hueso
	std::vector<glm::mat4> matrices_huesos;      // resultado que se manda al shader
	glm::mat4 normalizacion{1};
    glm::mat4 inversa_raiz = glm::mat4(1.0f);
	std::vector<Animacion> animaciones;

	GLuint VAO = 0, VBO = 0, IBO = 0;
	GLsizei num_indices = 0;
	Texture textura;
};
//>>>>>>>>>>>>>>>>>>>>>> CicloDiaNoche >>>>>>>>>>>>>>>>>>>>>>
// CicloDiaNoche

// Valores de la luz para mandarlos a cualquier shader (tambien al del agua)
struct DatosLuz
{
	glm::vec3 direccion;
	glm::vec3 color;
	float ambiental;
	float difusa;
};

class CicloDiaNoche
{
public:
	float duracion = IluminacionAmbiental::duracion_dia;        // segundos de un dia completo
	float inclinacion_sol = 22.0f;   // grados sobre el horizonte

	void Actualizar(float dt);

	float FactorDia() const;        // 1 = mediodia, 0 = medianoche
	bool EsDeNoche() const { return FactorDia() < 0.5f; }   // para prender lamparas

	DatosLuz Luz() const;
	DirectionalLight LuzDireccional() const;   // lista para shader.SetDirectionalLight

private:
	float tiempo = 0.0f;
};
//>>>>>>>>>>>>>>>>>>>>>> Terreno >>>>>>>>>>>>>>>>>>>>>>
// Terreno

class Terreno
{
public:
	//Medidas
	float tamano = 400.0f;                        // lado del terreno cuadrado
	int celdas = 400;                             // cuadritos por lado
	glm::vec2 centro_lago = glm::vec2(0.0f, 70.0f);
	glm::vec2 radio_lago = glm::vec2(80.0f, 55.0f); // radio en X y en Z
	float profundidad = 30.0f;                    // que tan hondo es el lago
	float metros_por_textura = 12.0f;               // cada cuanto se repite la textura

	void Crear();
	void Dibujar(const ContextoRender& ctx);
	~Terreno();

	float Altura(float x, float z) const;         // altura del suelo en (x, z)
	bool EsLago(float x, float z) const;          // true si (x, z) cae en el lago o la playa
	float DistanciaLago(float x, float z) const;  // 0 en el centro, 1 en la orilla, >1 tierra

private:
	glm::vec3 NormalEn(float x, float z) const;
	void AgregarCuadro(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float x0, float z0, float lado);

	MeshModel* tierra = nullptr;
	MeshModel* fondo = nullptr;
	Texture textura_pasto;
	Texture textura_arena;
	Material material_suelo = Material(0.2f, 8);
};
//>>>>>>>>>>>>>>>>>>>>>> Agua >>>>>>>>>>>>>>>>>>>>>>
// Agua superficie semitransparente con olas, dibujada con ShaderAnimado.
// IMPORTANTE: dibujarla AL FINAL de la escena

class Agua
{
public:
	//Aspecto
	float amplitud = 0.25f;                         // altura de las olas
	float frecuencia = 0.15f;                       // olas mas juntas o separadas
	float velocidad = 1.2f;                         // rapidez de las olas
	glm::vec2 velocidad_u_v = glm::vec2(0.02f, 0.01f); // como fluye la textura
	float transparencia = 0.6f;                     // 0 invisible, 1 opaca
	glm::vec3 color_base = glm::vec3(0.55f, 0.8f, 1.0f);

	// Rectangulo de agua (centro, tamano) a la altura nivel_y
	void Crear(glm::vec2 centro, glm::vec2 tamano, float nivel_y, const char* ruta_textura);
	void Dibujar(const ContextoRender& ctx, float tiempo, const DatosLuz& luz,
                 const glm::vec3& luz_sumergida, bool encendida);
	~Agua();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
	float repeticion = 1.0f;
};
//>>>>>>>>>>>>>>>>>>>>>> Fuego >>>>>>>>>>>>>>>>>>>>>>
// Fuego

class Fuego
{
public:
	void Crear();
	// posicion = base de la llama, ancho y alto
	void Dibujar(const ContextoRender& ctx, glm::vec3 posicion, float ancho, float alto, float tiempo);
	~Fuego();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
};
//>>>>>>>>>>>>>>>>>>>>>> Vegetacion >>>>>>>>>>>>>>>>>>>>>>
// Vegetacion: arboles, arbustos, pasto, flores y rocas del planeta azul.
// Modelos: Stylized Nature MegaKit de Quaternius (licencia CC0).
//
// Igual que en las practicas: cada modelo se carga una vez (Model) y se dibuja
// varias veces cambiando su matriz model (trasladar, rotar, escalar).

class Vegetacion
{
public:
	void Crear(const Terreno& terreno);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces);

	// Zonas sin vegetacion para casas y NPCs (true si (x, z) cae dentro de una)
	bool EnZonaLibre(float x, float z) const;

private:
	// Dibuja un modelo parado sobre el terreno en (x, z)
	void DibujarEn(Model& modelo, float x, float z, float escala, float giro, glm::vec3 color = glm::vec3(1.0f));

	const Terreno* terreno = nullptr;
	Shader shader;                          // shader_vegetacion (recorta lo transparente de las hojas)
	Material material_plantas = Material(0.3f, 8);
	GLuint u_model = 0, u_color = 0;

	Model arbol1;
	Model arbol2;
	Model pino1;
	Model pino2;
	Model arbusto;
	Model arbusto_flores;
	Model pasto;
	Model flores;
	Model roca1;
	Model roca2;
	Model piedra;
	Model alga;
};
// Pez abisal y luces del fondo del lago.
class PezAbisal
{
public:
	void Crear(const Terreno& terreno);
	void Actualizar(float dt, Window& ventana);
	void Dibujar(const ContextoRender& ctx);
	void DibujarHalo(const ContextoRender& ctx);   // brillo del farol (transparente, va al final)

	SpotLight foco;              // reflector que sale del farol
	bool linterna = true;        // empieza prendida

private:
	glm::mat4 MatrizCuerpo() const;

	const Terreno* terreno = nullptr;
	float tiempo = 0.0f;
	float angulo = 0.0f;
	glm::vec3 posicion{ 0.0f };
	float giro = 0.0f;

	Model cuerpo;
	Model aleta_izq;
	Model aleta_der;
	Model farol;
	Texture halo;
	MeshModel plano_halo;
};

class ArbolesLuminosos
{
public:
	static const int num_arboles = 8;

	void Crear(const Terreno& terreno);
	void Actualizar(float dt, bool es_de_noche, Window& ventana);
	void Dibujar(const ContextoRender& ctx);

	// Una luz por arbol (solo cuando estan prendidos)
	PointLight luces[num_arboles];
	glm::vec3 pos_luces[num_arboles];
	unsigned int num_luces = 0;

	bool encendidos = false;

private:
	const Terreno* terreno = nullptr;
	Model arbol;
	float brillo = 0.0f;          // 0 apagado, 1 prendido (cambia poco a poco)
	float tiempo = 0.0f;
	bool era_de_noche = false;
};

//>>>>>>>>>>>>>>>>>>>>>> Aldea >>>>>>>>>>>>>>>>>>>>>>
// Aldea

class Aldea
{
public:
	glm::vec2 centro = glm::vec2(-110.0f, -40.0f);   // centro de la aldea = la fogata

	void Crear(const Terreno& terreno);
	void Actualizar(float tiempo, bool es_de_noche, Window& ventana);
	void Dibujar(const ContextoRender& ctx);                 // chozas, antorchas y fogata
	void DibujarLlamas(const ContextoRender& ctx, float tiempo);   // fuego

	// Luces puntuales encendidas en este momento
	PointLight luces[max_point_lights];
    glm::vec3 pos_luces[max_point_lights];
	unsigned int num_luces = 0;

	// Donde esta cada choza, para poner a sus habitantes
	int NumChozas() const;
	glm::vec2 PosicionChoza(int i) const;

	bool fogata_encendida = false;      // empieza apagada; se prende con U
	bool era_de_noche = false;
    bool antorchas_encendidas = false;   // las prende el ciclo de dia y noche

private:
	glm::vec3 SobreElSuelo(float x, float z) const;   //x, altura del terreno, z

	const Terreno* terreno = nullptr;
	Fuego fuego;

	Model choza_muu;
	Model choza_mantah;
	Model choza_krakee;
	Model choza_antuk;
	Model antorcha;
	Model fogata;
};
//>>>>>>>>>>>>>>>>>>>>>> Peces >>>>>>>>>>>>>>>>>>>>>>
// Peces Air fish y Peepers

class Peces
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

private:
	float tiempo = 0.0f;
	Model air_fish;
	Model peeper;
};
//>>>>>>>>>>>>>>>>>>>>>> Necromorfos >>>>>>>>>>>>>>>>>>>>>>
// Necromorfos NPCs con animacion esqueletal

class Necromorfos
{
public:
    glm::vec3 PosicionPatrulla() const;
    glm::vec3 RumboPatrulla() const;
	void Crear(const Terreno& terreno, const Aldea& aldea);
	void Actualizar(float dt, bool fogata_encendida);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces);

private:
	// Un necromorfo que da vueltas a un circulo
	struct Caminante
	{
		glm::vec2 centro;
		float radio;
		float angulo;        // donde va en el circulo (radianes)
		float velocidad;     // radianes por segundo (negativa = al reves)
		bool baila_con_fogata;
	};

	void DibujarUno(const Caminante& c, int animacion, float tiempo_animacion, GLuint u_model);

	const Terreno* terreno = nullptr;
	glm::vec2 fogata;
	bool bailando = false;
	float tiempo = 0.0f;

	ModeloAnimado necromorfo;
	int anim_caminar = 0;
	int anim_bailar = 1;
	Shader shader;              // shader_esqueleto.vert + shader_light.frag

	static const int max_caminantes = 10;
	Caminante caminantes[max_caminantes];
	int num_caminantes = 0;
};
//>>>>>>>>>>>>>>>>>>>>>> Stalker >>>>>>>>>>>>>>>>>>>>>>
// Stalker

class Stalker
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	glm::vec3 posicion = glm::vec3(0.0f);   // donde esta ahora
	float giro = 0.0f;                       // hacia donde mira

private:
	float tiempo = 0.0f;
	float angulo = 0.0f;    // donde va en la elipse

	Model cuerpo;
	Model boca_sup;
	Model boca_inf;
	Model aleta_del_izq;
	Model aleta_del_der;
	Model aleta_tras_izq;
	Model aleta_tras_der;
};
//>>>>>>>>>>>>>>>>>>>>>> Pidgeys >>>>>>>>>>>>>>>>>>>>>>
// Pidgeys

class Pidgeys
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	//La presa (la usa el Reaper para saber donde saltar)
	glm::vec2 centro_presa = glm::vec2(0.0f, 70.0f);   // circulo sobre el lago
	float radio_presa = 22.0f;
	float altura_presa = 12.0f;
	float velocidad_presa = 0.25f;                     // radianes por segundo

	float AnguloPresa() const { return angulo_presa; }
	glm::vec3 PosicionPresa() const;
	bool PresaViva() const { return presa_viva; }
	void ComerPresa();                                // la llama el Reaper al morder

private:
	void DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala);

	float tiempo = 0.0f;
	float angulo_presa = 0.0f;
	bool presa_viva = true;
	float tiempo_comida = 0.0f;     // segundos desde que se la comieron

	Model cuerpo;
	Model ala_izq;
	Model ala_der;
};
//>>>>>>>>>>>>>>>>>>>>>> Reaper >>>>>>>>>>>>>>>>>>>>>>
// Reaper

class Reaper
{
public:
	void Crear();
	void Actualizar(float dt, Window& ventana, Pidgeys& pidgeys);
	void Dibujar(const ContextoRender& ctx);

private:
	float tiempo = 0.0f;

	glm::vec3 posicion = glm::vec3(0.0f);   // posicion de la cabeza
	float giro = 0.0f;                       // hacia donde mira
	float inclinacion = 0.0f;                // nariz arriba (+) o abajo (-)

	bool saltando = false;
	float s = 0.0f;                          // avance del salto (0 a 1)
	bool ya_mordio = false;
	float abre_boca = 0.0f;                   // 0 cerrada, 1 abierta

	Model cuerpo;
	Model boca;
	Model tent_sup_izq;
	Model tent_sup_der;
	Model tent_inf_izq;
	Model tent_inf_der;
	Model aleta_izq;
	Model aleta_der;
	Model cola;
};
//>>>>>>>>>>>>>>>>>>>>>> EscenaPlanetaAzul >>>>>>>>>>>>>>>>>>>>>>

class EscenaPlanetaAzul : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Azul"; }
    ~EscenaPlanetaAzul() override { lampara_suelo.ClearModel(); }
    float SueloPatrulla(float x, float z) const override;
    PointLight LuzLocal() const override;
    void PrepararLuces(const glm::vec3& ojo) override;
    SpotLight* ObtenerFocos() override { return &pez_abisal.foco; }
    unsigned int NumFocos() override { return pez_abisal.linterna ? 1u : 0u; }
    glm::vec2 CentroAldea() const { return aldea.centro; }
    bool ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const override;
    void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

	// main.cpp usa esta luz cuando el planeta azul esta activo
	DirectionalLight* ObtenerLuzDireccional() override { return &luz_sol; }

	// main.cpp usa las luces de la aldea fogata y antorchas
	PointLight* ObtenerLucesPuntuales() override { return luces_visibles; }
	unsigned int NumLucesPuntuales() override { return num_luces_visibles; }

private:
	float tiempo = 0.0f;          // reloj de la escena

	CicloDiaNoche ciclo;
	DirectionalLight luz_sol;      // la unica luz direccional del planeta
	Skybox skybox_dia;
	Skybox skybox_noche;

    ArbolesLuminosos arboles_sumergidos;
    PezAbisal pez_abisal;
    PointLight luces_visibles[max_point_lights];
    unsigned int num_luces_visibles = 0;
	Terreno terreno;
	Vegetacion vegetacion;
	Aldea aldea;
	Peces peces;
	Necromorfos necromorfos;
	Stalker stalker;             // avatar
	Pidgeys pidgeys;
	Reaper reaper;
	Agua agua;
    Model lampara_suelo;
    Texture halo_suelo;
    MeshModel malla_halo_suelo, malla_halo_tejado;
    void CargarLamparas();
    void DibujarLamparas(const ContextoRender& ctx);
    bool luz_lago_encendida = true;
    float nivel_agua = -1.0f;      // altura de la superficie del lago

};
}
using EscenaPlanetaAzul = PlanetaAzul::EscenaPlanetaAzul;
