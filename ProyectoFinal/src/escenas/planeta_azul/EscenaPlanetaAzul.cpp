#include "EscenaPlanetaAzul.h"

namespace PlanetaAzul
{
const char* animado_vert = R"glsl(#version 330
// Shader animado (agua, fuego, etc.). Mueve los vertices con ondas seno,
// como en el ejemplo de clase (anim.vs), y calcula su normal para la luz.

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float tiempo;       // segundos que lleva corriendo
uniform float amplitud;     // altura de la onda (0 = sin movimiento)
uniform float frecuencia;   // que tan juntas estan las ondas
uniform float velocidad;    // que tan rapido avanzan las ondas

void main()
{
	vec3 p = pos;

	// Dos ondas seno cruzadas: una avanza en X y otra en Z
	float onda1 = frecuencia * p.x + velocidad * tiempo;
	float onda2 = frecuencia * 0.8 * p.z + velocidad * 1.3 * tiempo;
	p.y += amplitud * 0.5 * (sin(onda1) + sin(onda2));

	// Normal de la superficie ondulada (derivada de la altura en X y en Z)
	float dx = amplitud * 0.5 * frecuencia * cos(onda1);
	float dz = amplitud * 0.5 * frecuencia * 0.8 * cos(onda2);
	vec3 n = normalize(vec3(-dx, 1.0, -dz));

	gl_Position = projection * view * model * vec4(p, 1.0);
	FragPos = (model * vec4(p, 1.0)).xyz;
	Normal = mat3(transpose(inverse(model))) * n;
	TexCoord = tex;
}
)glsl";
const char* animado_frag = R"glsl(#version 330
// Shader animado (agua, fuego...): textura que se mueve con el tiempo,
// transparencia y luz direccional.
//   Agua:  usar_luz = 1, transparencia 0.6, desvanecer 0
//   Fuego: usar_luz = 0, transparencia 1.0, desvanecer 1, textura subiendo
//          (en C++ se mezcla con glBlendFunc(GL_SRC_ALPHA, GL_ONE): lo negro no se ve)

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D textura;
uniform float tiempo;
uniform vec2  velocidad_u_v;     // cuanto se mueve la textura por segundo
uniform float repeticion_u_v;    // cuantas veces se repite la textura
uniform vec3  color_base;       // color que multiplica a la textura
uniform float transparencia;   // 0 = invisible, 1 = opaco
uniform float desvanecer;      // 1 = se apaga hacia arriba y a los lados (forma de llama)

uniform int   usar_luz;         // 1 = recibe la luz direccional, 0 = no
uniform vec3  luz_direccion;    // hacia donde viaja la luz (igual que DirectionalLight)
uniform vec3  luz_color;
uniform float luz_ambiental;
uniform float luz_difusa;
uniform vec3  pos_camara;
uniform float brillo;          // shininess del reflejo
uniform vec3 luz_sumergida;
uniform float intensidad_sumergida;

void main()
{
	// La textura se recorre con el tiempo: parece que el agua fluye
	vec2 uv = TexCoord * repeticion_u_v + velocidad_u_v * tiempo;
	vec3 rgb = texture(textura, uv).rgb * color_base;

	// Forma de llama: brillante abajo al centro, se apaga hacia arriba y hacia los lados
	float hacia_arriba = 1.0 - TexCoord.y;
	float hacia_lados = 1.0 - abs(TexCoord.x * 2.0 - 1.0);
	float forma = hacia_arriba * hacia_lados * 2.0;
	rgb = mix(rgb, rgb * forma, desvanecer);

	if (usar_luz == 1)
	{
		vec3 N = normalize(Normal);
		vec3 L = normalize(-luz_direccion);           // hacia la luz
		vec3 V = normalize(pos_camara - FragPos);     // hacia la camara

		// Igual que en shader_light: ambiental + difusa + especular
		float difusa = max(dot(N, L), 0.0);
		float especular = pow(max(dot(V, reflect(-L, N)), 0.0), brillo);
		rgb = rgb * luz_color * (luz_ambiental + luz_difusa * difusa)
			+ luz_color * especular * luz_difusa;
	}

	color = vec4(rgb, transparencia);
}
)glsl";
const char* esqueleto_vert = R"glsl(#version 330
// shader_esqueleto.vert
//>>>>>>>>>>>>>>>>>>>>>> Entradas y uniforms >>>>>>>>>>>>>>>>>>>>>>>>>

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;
layout (location = 3) in ivec4 hueso_ids;   // los 4 huesos que mueven a este vertice
layout (location = 4) in vec4 pesos;       // cuanto lo mueve cada uno (suman 1)

out vec4 v_col;
out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;
out vec4 v_color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;

const int MAX_HUESOS = 100;
uniform mat4 huesos[MAX_HUESOS];           // pose de cada hueso (la calcula ModeloAnimado)


//>>>>>>>>>>>>>>>>>>>>>> Funcion principal del shader >>>>>>>>>>>>>>>>
void main()
{
	// Mezcla de las matrices de los 4 huesos segun sus pesos
	mat4 piel = huesos[hueso_ids.x] * pesos.x
	          + huesos[hueso_ids.y] * pesos.y
	          + huesos[hueso_ids.z] * pesos.z
	          + huesos[hueso_ids.w] * pesos.w;
	vec4 pos_animada = piel * vec4(pos, 1.0);
	vec3 normal_animada = mat3(piel) * norm;

	gl_Position = projection * view * model * pos_animada;
	v_col = vec4(0.0, 1.0, 0.0, 1.0f);
	v_color=vec4(color,1.0f);
	tex_coord = tex;
	// Transformo las normales con la inversa transpuesta de la matriz del modelo.
	normal = mat3(transpose(inverse(model))) * normal_animada;
	
	frag_pos = (model * pos_animada).xyz; // Posicion en el espacio de la escena.
}
)glsl";
const char* vegetacion_frag = R"glsl(#version 330
// shader_vegetacion.frag: copia de shader_light.frag para hojas, pasto y flores.
//>>>>>>>>>>>>>>>>>>>>>> Entradas y uniforms >>>>>>>>>>>>>>>>>>>>>>>>>

in vec4 v_col;
in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;
in vec4 v_color;

out vec4 color;

const int max_point_lights = 16;
const int max_spot_lights = 5;

struct Light
{
	vec3 color;
	float ambient_intensity;
	float diffuse_intensity;
};

struct DirectionalLight 
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};


struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material
{
	float specular_intensity;
	float shininess;
};

uniform int point_light_count;
uniform int spot_light_count;

uniform DirectionalLight directional_light;
uniform PointLight point_lights[max_point_lights];
uniform SpotLight spot_lights[max_spot_lights];

uniform sampler2D the_texture;
uniform Material material;

uniform vec3 eye_position;
uniform bool normales_geometricas;
uniform float emision_material;
uniform bool relieve_vibrante;
uniform vec2 desplazamiento_textura = vec2(0.0);
uniform int modo_efecto = 0; // 0: material, 1: fuego, 2: humo de la practica 10.
uniform float opacidad_particula = 1.0;
uniform float opacidad_aerea = 1.0;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambient_color = vec4(light.color, 1.0f) * light.ambient_intensity;
	// El producto punto mide el angulo entre la luz y la normal.
	float diffuse_factor = max(dot(normalize(normal), normalize(direction)), 0.0f);
	vec4 diffuse_color = vec4(light.color * light.diffuse_intensity * diffuse_factor, 1.0f);
	
	vec4 specular_color = vec4(0, 0, 0, 0);
	
	if(diffuse_factor > 0.0f)
	{ // Calculo el brillo especular a partir de la posicion de la camara.
		vec3 frag_to_eye = normalize(eye_position - frag_pos);
		vec3 reflected_vertex = normalize(reflect(normales_geometricas ? -direction : direction, normalize(normal)));
		
		float specular_factor = dot(frag_to_eye, reflected_vertex);
		if(specular_factor > 0.0f)
		{
			specular_factor = pow(specular_factor, material.shininess);
			specular_color = vec4(light.color * material.specular_intensity * specular_factor, 1.0f);
		}
	}

	return (ambient_color + diffuse_color + specular_color);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directional_light.base, directional_light.direction);
}

//nuevo
vec4 CalcPointLight(PointLight p_light)
{
	vec3 direction = frag_pos - p_light.position;
	float distance = length(direction);
	direction = normalize(direction);
	if (normales_geometricas) direction = -direction;
	
	vec4 color = CalcLightByDirection(p_light.base, direction);
	float attenuation = p_light.exponent * distance * distance +
						p_light.linear * distance +
						p_light.constant;
	
	return (color / attenuation);
}




vec4 CalcSpotLight(SpotLight s_light)
{
	vec3 ray_direction = normalize(frag_pos - s_light.base.position);
	float sl_factor = dot(ray_direction, s_light.direction);
	
	if(sl_factor > s_light.edge)
	{
		vec4 color = CalcPointLight(s_light.base);
		
		return color * (1.0f - (1.0f - sl_factor)*(1.0f/(1.0f - s_light.edge)));
		
	} else {
		return vec4(0, 0, 0, 0);
	}
}










vec4 CalcPointLights()
{
	vec4 total_color = vec4(0, 0, 0, 0);
	for(int i = 0; i < point_light_count; i++)
	{		
		total_color += CalcPointLight(point_lights[i]);
	}
	
	return total_color;
}

vec4 CalcSpotLights()
{
	vec4 total_color = vec4(0, 0, 0, 0);
	for(int i = 0; i < spot_light_count; i++)
	{		
		total_color += CalcSpotLight(spot_lights[i]);
	}
	
	return total_color;
}


//>>>>>>>>>>>>>>>>>>>>>> Funcion principal del shader >>>>>>>>>>>>>>>>
void main()
{
	// UNICA diferencia con shader_light.frag: las hojas tienen partes
	// transparentes en su textura; esos pixeles no se dibujan.
	if (texture(the_texture, tex_coord).a < 0.5)
		discard;

	vec4 muestra = texture(the_texture, tex_coord + desplazamiento_textura);
	if (modo_efecto == 1)
	{
		float ancho = mix(0.49, 0.05, tex_coord.y);
		float borde = 1.0 - smoothstep(ancho * 0.60, ancho, abs(tex_coord.x - 0.5));
		float punta = 1.0 - smoothstep(0.40, 1.0, tex_coord.y);
		float raiz = smoothstep(0.0, 0.12, tex_coord.y);
		color = vec4(muestra.rgb * v_color.rgb * 2.2, muestra.a * borde * punta * raiz * opacidad_particula);
		return;
	}
	if (modo_efecto == 2)
	{
		color = vec4(muestra.rgb, muestra.a * opacidad_particula);
		return;
	}
	if (modo_efecto == 3)
	{
		float borde = pow(1.0 - clamp(dot(normalize(normal), normalize(eye_position - frag_pos)), 0.0, 1.0), 2.6);
		color = vec4(v_color.rgb, borde * opacidad_particula);
		return;
	}
	vec4 final_color = CalcDirectionalLight();
	final_color += CalcPointLights();
	final_color += CalcSpotLights();
	vec4 texel = texture(the_texture, tex_coord);
	if (relieve_vibrante)
	{
		float luminancia = dot(texel.rgb, vec3(0.213, 0.715, 0.072));
		texel.rgb = mix(vec3(luminancia), texel.rgb, 1.65);
		texel.rgb = max(vec3(0.0), (texel.rgb - vec3(0.18)) * 1.30 + vec3(0.18));
	}
	color = texel*v_color*final_color;
	color.rgb += texture(the_texture, tex_coord).rgb * emision_material;
	if (normales_geometricas) color.a = texture(the_texture, tex_coord).a * v_color.a;
	color.a *= opacidad_aerea;
	
}
)glsl";
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

bool ShaderAnimado::Crear(const char* codigo_vert, const char* codigo_frag)
{
	GLuint vs = Compilar(GL_VERTEX_SHADER, codigo_vert, "ondas y fuego: vertices");
	GLuint fs = Compilar(GL_FRAGMENT_SHADER, codigo_frag, "ondas y fuego: fragmentos");

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
//>>>>>>>>>>>>>>>>>>>>>> ModeloAnimado >>>>>>>>>>>>>>>>>>>>>>


// Vertice con huesos: posicion, UV, normal + 4 huesos y sus 4 pesos
struct VerticeAnimado
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	int huesos[4];
	float pesos[4];
};

// Assimp guarda las matrices por renglones y glm por columnas hay que transponer
static glm::mat4 AGlm(const aiMatrix4x4& m)
{
	return glm::transpose(glm::make_mat4(&m.a1));
}

static const unsigned int FLAGS_ASSIMP =
	aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices;

ModeloAnimado::~ModeloAnimado()
{
	if (IBO) glDeleteBuffers(1, &IBO);
	if (VBO) glDeleteBuffers(1, &VBO);
	if (VAO) glDeleteVertexArrays(1, &VAO);
}

int ModeloAnimado::CopiarNodo(const aiNode* nodo)
{
	int indice = (int)nodos.size();
	nodos.push_back({ nodo->mName.C_Str(), AGlm(nodo->mTransformation), {} });
	for (unsigned int i = 0; i < nodo->mNumChildren; i++)
	{
		int hijo = CopiarNodo(nodo->mChildren[i]);
		nodos[indice].hijos.push_back(hijo);
	}
	return indice;
}

bool ModeloAnimado::Cargar(const std::string& archivo, const std::string& ruta_textura, bool normalizar)
{
	Assimp::Importer importer;
	// Sin nodos auxiliares de FBX el arbol queda con un nodo por hueso
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(archivo, FLAGS_ASSIMP);
	if (!scene || !scene->mRootNode)
	{
		printf("No se pudo cargar el modelo animado %s: %s\n", archivo.c_str(), importer.GetErrorString());
		return false;
	}

	//Arbol de huesos
	nodos.clear();
	CopiarNodo(scene->mRootNode);
	inversa_raiz = glm::inverse(nodos[0].transformacion);

	//Malla vertices con sus huesos y pesos 
	std::vector<VerticeAnimado> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int m = 0; m < scene->mNumMeshes; m++)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		unsigned int base = (unsigned int)vertices.size();

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VerticeAnimado v;
			v.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			v.uv = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
			// Normal invertida misma convencion que Model y shader_light
			v.normal = -glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			for (int k = 0; k < 4; k++) { v.huesos[k] = 0; v.pesos[k] = 0.0f; }
			vertices.push_back(v);
		}

		for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			for (unsigned int k = 0; k < mesh->mFaces[f].mNumIndices; k++)
				indices.push_back(base + mesh->mFaces[f].mIndices[k]);

		// Cada hueso dice a que vertices mueve y con que peso
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			const aiBone* hueso = mesh->mBones[b];
			std::string nombre = hueso->mName.C_Str();
			if (indice_hueso.find(nombre) == indice_hueso.end())
			{
				indice_hueso[nombre] = (int)offsets.size();
				offsets.push_back(AGlm(hueso->mOffsetMatrix));
			}
			int id = indice_hueso[nombre];
			for (unsigned int w = 0; w < hueso->mNumWeights; w++)
			{
				VerticeAnimado& v = vertices[base + hueso->mWeights[w].mVertexId];
				for (int k = 0; k < 4; k++)          // se guarda en el primer lugar libre
				{
					if (v.pesos[k] == 0.0f)
					{
						v.huesos[k] = id;
						v.pesos[k] = hueso->mWeights[w].mWeight;
						break;
					}
				}
			}
		}
	}
	matrices_huesos.assign(offsets.size(), glm::mat4(1.0f));

	//Mandar la malla a la tarjeta de video
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VerticeAnimado), vertices.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	GLsizei paso = sizeof(VerticeAnimado);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 4, GL_INT, paso, (void*)offsetof(VerticeAnimado, huesos));   // enteros: "I"
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, pesos));
	glEnableVertexAttribArray(4);
	glBindVertexArray(0);
	num_indices = (GLsizei)indices.size();

	if (ruta_textura.empty())
    {
        const aiTexture* imagen = nullptr;
        for (unsigned i = 0; i < scene->mNumMaterials && !imagen; ++i)
        {
            aiString ruta;
            if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE,0,&ruta) == AI_SUCCESS)
                imagen = scene->GetEmbeddedTexture(ruta.C_Str());
        }
        if (!imagen || imagen->mHeight || !textura.LoadMemory(
            reinterpret_cast<const unsigned char*>(imagen->pcData),int(imagen->mWidth)))
            textura.LoadColor(.62f,.44f,.73f);
    }
    else
    {
        textura = Texture(ruta_textura.c_str());
        textura.LoadTextureA();
    }

	printf("Modelo animado %s: %d vertices, %d huesos\n", archivo.c_str(), (int)vertices.size(), (int)offsets.size());
    const bool animado = LeerAnimacion(archivo);
    if (normalizar && animado)
    {
        CalcularPose(0,0);
        glm::vec3 minimo(1e9f), maximo(-1e9f);
        for (const auto& v : vertices)
        {
            glm::vec4 punto(0);
            for (int i = 0; i < 4; ++i)
                if (v.pesos[i] > 0)
                    punto += v.pesos[i] * (matrices_huesos[v.huesos[i]] * glm::vec4(v.pos,1));
            minimo = glm::min(minimo,glm::vec3(punto));
            maximo = glm::max(maximo,glm::vec3(punto));
        }
        if (maximo.y > minimo.y)
            normalizacion = glm::scale(glm::mat4(1),glm::vec3(1.0f/(maximo.y-minimo.y))) *
                glm::translate(glm::mat4(1),-glm::vec3((minimo.x+maximo.x)*.5f,minimo.y,(minimo.z+maximo.z)*.5f));
    }
	return true;
}

bool ModeloAnimado::LeerAnimacion(const std::string& archivo)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(archivo, FLAGS_ASSIMP);
	if (!scene || scene->mNumAnimations == 0)
	{
		printf("%s no trae animacion\n", archivo.c_str());
		return false;
	}

	const aiAnimation* a = scene->mAnimations[0];
	Animacion anim;
	anim.duracion_ticks = (float)a->mDuration;
	anim.ticks_por_segundo = a->mTicksPerSecond > 0.0 ? (float)a->mTicksPerSecond : 30.0f;

	for (unsigned int c = 0; c < a->mNumChannels; c++)
	{
		const aiNodeAnim* ch = a->mChannels[c];
		Canal canal;
		for (unsigned int k = 0; k < ch->mNumPositionKeys; k++)
		{
			const aiVector3D& p = ch->mPositionKeys[k].mValue;
			canal.posiciones.push_back({ (float)ch->mPositionKeys[k].mTime, glm::vec3(p.x, p.y, p.z) });
		}
		for (unsigned int k = 0; k < ch->mNumRotationKeys; k++)
		{
			const aiQuaternion& q = ch->mRotationKeys[k].mValue;
			canal.rotaciones.push_back({ (float)ch->mRotationKeys[k].mTime, glm::quat(q.w, q.x, q.y, q.z) });
		}
		for (unsigned int k = 0; k < ch->mNumScalingKeys; k++)
		{
			const aiVector3D& s = ch->mScalingKeys[k].mValue;
			canal.escalas.push_back({ (float)ch->mScalingKeys[k].mTime, glm::vec3(s.x, s.y, s.z) });
		}
		anim.canales[ch->mNodeName.C_Str()] = canal;
	}
	animaciones.push_back(anim);
	printf("Animacion %d de %s: %.1f segundos\n", (int)animaciones.size() - 1, archivo.c_str(),
		anim.duracion_ticks / anim.ticks_por_segundo);
	return true;
}

int ModeloAnimado::AgregarAnimacion(const std::string& archivo)
{
	LeerAnimacion(archivo);
	return (int)animaciones.size() - 1;
}

void ModeloAnimado::QuitarAvance(int animacion, const std::string& hueso)
{
	if (animacion < 0 || animacion >= (int)animaciones.size())
		return;
	auto canal = animaciones[animacion].canales.find(hueso);
	if (canal == animaciones[animacion].canales.end() || canal->second.posiciones.empty())
		return;

	// Todas las posiciones toman la X y Z del primer keyframe (solo se conserva el sube y baja)
	std::vector<ClavePos>& claves = canal->second.posiciones;
	for (ClavePos& c : claves)
	{
		c.valor.x = claves[0].valor.x;
		c.valor.z = claves[0].valor.z;
	}
}

// Busca entre que dos keyframes cae "tick" y regresa el indice del primero
template <typename T>
static int BuscarClave(const std::vector<T>& claves, float tick)
{
	for (int i = 0; i + 1 < (int)claves.size(); i++)
		if (tick < claves[i + 1].tick)
			return i;
	return (int)claves.size() - 1;
}

glm::mat4 ModeloAnimado::Interpolar(const Canal& canal, float tick) const
{
	// Posicion: interpolacion lineal entre los dos keyframes
	glm::vec3 pos(0.0f);
	if (!canal.posiciones.empty())
	{
		int i = BuscarClave(canal.posiciones, tick);
		int j = glm::min(i + 1, (int)canal.posiciones.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.posiciones[i].tick) / (canal.posiciones[j].tick - canal.posiciones[i].tick);
		pos = glm::mix(canal.posiciones[i].valor, canal.posiciones[j].valor, t);
	}

	// Rotacion: "slerp", la interpolacion para rotaciones (cuaterniones)
	glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
	if (!canal.rotaciones.empty())
	{
		int i = BuscarClave(canal.rotaciones, tick);
		int j = glm::min(i + 1, (int)canal.rotaciones.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.rotaciones[i].tick) / (canal.rotaciones[j].tick - canal.rotaciones[i].tick);
		rot = glm::normalize(glm::slerp(canal.rotaciones[i].valor, canal.rotaciones[j].valor, t));
	}

	// Escala
	glm::vec3 esc(1.0f);
	if (!canal.escalas.empty())
	{
		int i = BuscarClave(canal.escalas, tick);
		int j = glm::min(i + 1, (int)canal.escalas.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.escalas[i].tick) / (canal.escalas[j].tick - canal.escalas[i].tick);
		esc = glm::mix(canal.escalas[i].valor, canal.escalas[j].valor, t);
	}

	// Igual que en clase: trasladar * rotar * escalar
	return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), esc);
}

void ModeloAnimado::CalcularNodo(const Animacion& anim, float tick, int indice, const glm::mat4& padre)
{
	const Nodo& nodo = nodos[indice];

	// Si el hueso tiene movimiento en la animacion se usa ese; si no, su pose de reposo
	glm::mat4 local = nodo.transformacion;
	auto canal = anim.canales.find(nodo.nombre);
	if (canal != anim.canales.end())
		local = Interpolar(canal->second, tick);

	// Jerarquia: la matriz del hijo se multiplica por la del padre
	glm::mat4 global = padre * local;

	auto hueso = indice_hueso.find(nodo.nombre);
	if (hueso != indice_hueso.end())
		matrices_huesos[hueso->second] = inversa_raiz * global * offsets[hueso->second];

	for (int hijo : nodo.hijos)
		CalcularNodo(anim, tick, hijo, global);
}

void ModeloAnimado::CalcularPose(int animacion, float segundos)
{
	if (animacion < 0 || animacion >= (int)animaciones.size() || nodos.empty())
		return;
	const Animacion& anim = animaciones[animacion];
	// Segundos -> ticks, y con fmod se repite al terminar
	float tick = fmodf(segundos * anim.ticks_por_segundo, anim.duracion_ticks);
	CalcularNodo(anim, tick, 0, glm::mat4(1.0f));
}

glm::mat4 ModeloAnimado::MarcoHueso(const std::string& nombre) const
{
    const auto hueso = indice_hueso.find(nombre);
    if (hueso == indice_hueso.end()) return glm::mat4(1);
    const int i = hueso->second;
    return normalizacion * matrices_huesos[i] * glm::inverse(offsets[i]);
}

void ModeloAnimado::Dibujar(GLuint programa_shader)
{
	GLint u_huesos = glGetUniformLocation(programa_shader, "huesos");
	int n = glm::min((int)matrices_huesos.size(), MAX_HUESOS);
	glUniformMatrix4fv(u_huesos, n, GL_FALSE, glm::value_ptr(matrices_huesos[0]));

	textura.UseTexture();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
//>>>>>>>>>>>>>>>>>>>>>> CicloDiaNoche >>>>>>>>>>>>>>>>>>>>>>


void CicloDiaNoche::Actualizar(float dt)
{
	tiempo += dt;
	if (tiempo > duracion)
		tiempo -= duracion;     // vuelve a empezar el dia
}

float CicloDiaNoche::FactorDia() const
{
	// Coseno empieza en 1 (mediodia), baja a 0 a la mitad (medianoche) y regresa
	return IluminacionAmbiental::FactorDia(tiempo, duracion);
}

DatosLuz CicloDiaNoche::Luz() const
{
	float f = FactorDia();

	// Direccion desde el sol del skybox (hacia +Z y arriba) hacia la escena
	float inc = glm::radians(inclinacion_sol);
	glm::vec3 hacia_el_sol(0.0f, sinf(inc), cosf(inc));

	DatosLuz luz;
	luz.direccion = -hacia_el_sol;
	luz.color = glm::mix(glm::vec3(0.45f, 0.55f, 0.85f), glm::vec3(1.0f), f);   // luna -> blanco
	luz.ambiental = glm::mix(0.12f, 0.40f, f);
	luz.difusa = glm::mix(0.15f, 0.85f, f);
	return luz;
}

DirectionalLight CicloDiaNoche::LuzDireccional() const
{
	DatosLuz l = Luz();
	return DirectionalLight(l.color.r, l.color.g, l.color.b,
		l.ambiental, l.difusa,
		l.direccion.x, l.direccion.y, l.direccion.z);
}
//>>>>>>>>>>>>>>>>>>>>>> Terreno >>>>>>>>>>>>>>>>>>>>>>


Terreno::~Terreno()
{
	delete tierra;
	delete fondo;
}

float Terreno::DistanciaLago(float x, float z) const
{
	// Distancia
	float dx = (x - centro_lago.x) / radio_lago.x;
	float dz = (z - centro_lago.y) / radio_lago.y;
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
		// Dentro del lago baja suave desde la orilla y luego queda un fondo casi plano
		float t = glm::smoothstep(1.0f, 0.55f, d);
		float ondulacion_fondo = 0.8f * sinf(x * 0.15f) * sinf(z * 0.12f);
		return -profundidad * t + ondulacion_fondo * t;
	}

	// Tierra
	float colinas = 2.0f + 1.5f * sinf(x * 0.035f) * sinf(z * 0.03f + 1.0f) + 0.5f * sinf(x * 0.11f + z * 0.07f);
	return colinas * glm::smoothstep(1.0f, 1.6f, d);
}

glm::vec3 Terreno::NormalEn(float x, float z) const
{
	// Pendiente por diferencias cuanto cambia la altura al moverse un poco en X y Z
	float e = 0.5f;
	float hx = (Altura(x + e, z) - Altura(x - e, z)) / (2.0f * e);
	float hz = (Altura(x, z + e) - Altura(x, z - e)) / (2.0f * e);
	return glm::normalize(glm::vec3(-hx, 1.0f, -hz));
}

void Terreno::AgregarCuadro(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float x0, float z0, float lado)
{
	// 4 esquinas del cuadrito
	float xs[4] = { x0, x0 + lado, x0 + lado, x0 };
	float zs[4] = { z0, z0, z0 + lado, z0 + lado };
	unsigned int b = (unsigned int)(v.size() / 8);   // numero del primer vertice nuevo
	for (int k = 0; k < 4; k++)
	{
		glm::vec3 n = NormalEn(xs[k], zs[k]);
		v.insert(v.end(), { xs[k], Altura(xs[k], zs[k]), zs[k],
							xs[k] / metros_por_textura, zs[k] / metros_por_textura,
							-n.x, -n.y, -n.z });
	}
	// 2 triangulos por cuadrito
	idx.insert(idx.end(), { b, b + 1, b + 2,  b + 2, b + 3, b });
}

void Terreno::Crear()
{
	std::vector<GLfloat> v_tierra, v_fondo;          // vertices del pasto y de la arena
	std::vector<unsigned int> i_tierra, i_fondo;     // indices del pasto y de la arena

	float lado = tamano / celdas;
	float inicio = -tamano / 2.0f;

	for (int i = 0; i < celdas; i++)
	{
		for (int j = 0; j < celdas; j++)
		{
			float x0 = inicio + i * lado;
			float z0 = inicio + j * lado;

			// Si el centro del cuadrito cae en el lago va a la malla de arena, si no a la de pasto
			if (EsLago(x0 + lado / 2, z0 + lado / 2))
				AgregarCuadro(v_fondo, i_fondo, x0, z0, lado);
			else
				AgregarCuadro(v_tierra, i_tierra, x0, z0, lado);
		}
	}

	tierra = new MeshModel();
	tierra->CreateMeshModel(v_tierra.data(), i_tierra.data(), (unsigned int)v_tierra.size(), (unsigned int)i_tierra.size());
	fondo = new MeshModel();
	fondo->CreateMeshModel(v_fondo.data(), i_fondo.data(), (unsigned int)v_fondo.size(), (unsigned int)i_fondo.size());

	textura_pasto = Texture("assets/planeta_azul/texturas/pasto.jpg");
	textura_pasto.LoadTextureA();
	textura_arena = Texture("assets/planeta_azul/texturas/arena.jpg");
	textura_arena.LoadTextureA();
}

void Terreno::Dibujar(const ContextoRender& ctx)
{
	glm::mat4 model(1.0f);                        
	glm::vec3 blanco(1.0f);
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));
	material_suelo.UseMaterial(ctx.u_specular_intensity, ctx.u_shininess);

	textura_pasto.UseTexture();
	tierra->RenderMeshModel();

	textura_arena.UseTexture();
	fondo->RenderMeshModel();

}
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
	shader.Crear(animado_vert, animado_frag);
}

void Agua::Dibujar(const ContextoRender& ctx, float tiempo, const DatosLuz& luz,
                   const glm::vec3& luz_sumergida, bool encendida)
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
    shader.SetVec3("luz_sumergida",luz_sumergida);
    shader.SetFloat("intensidad_sumergida",encendida ? .60f : 0.0f);

	// Transparencia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);      // el agua no tapa lo que se dibuje despues

	textura.UseTexture();
	malla->RenderMeshModel();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
//>>>>>>>>>>>>>>>>>>>>>> Fuego >>>>>>>>>>>>>>>>>>>>>>


Fuego::~Fuego()
{
	delete malla;
}

void Fuego::Crear()
{
	// Dos cuadros cruzados de 1 x 1 (uno en el plano XY y otro en el ZY),
	// con varios cuadritos por lado para que las ondas los puedan doblar.
	const int div = 6;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int cuadro = 0; cuadro < 2; cuadro++)
	{
		unsigned int b = (unsigned int)(v.size() / 8);
		for (int i = 0; i <= div; i++)          // a lo ancho
		{
			for (int j = 0; j <= div; j++)      // a lo alto
			{
				float u = (float)i / div, w = (float)j / div;
				float x = (cuadro == 0) ? u - 0.5f : 0.0f;
				float z = (cuadro == 0) ? 0.0f : u - 0.5f;
				v.insert(v.end(), { x, w, z,  u, w,  0.0f, 0.0f, 1.0f });
			}
		}
		for (int i = 0; i < div; i++)
		{
			for (int j = 0; j < div; j++)
			{
				unsigned int a = b + i * (div + 1) + j, c = a + div + 1;
				idx.insert(idx.end(), { a, c, a + 1,  a + 1, c, c + 1 });
			}
		}
	}
	malla = new MeshModel();
	malla->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());

	textura = Texture("assets/planeta_azul/texturas/fuego.jpg");
	textura.LoadTextureA();
	shader.Crear(animado_vert, animado_frag);
}

void Fuego::Dibujar(const ContextoRender& ctx, glm::vec3 posicion, float ancho, float alto, float tiempo)
{
	GLint shader_anterior = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_anterior);

	glm::mat4 model(1.0f);
	model = glm::translate(model, posicion);
	model = glm::scale(model, glm::vec3(ancho, alto, ancho));

	shader.Usar();
	shader.SetMat4("model", model);
	shader.SetMat4("view", ctx.vista);
	shader.SetMat4("projection", ctx.proyeccion);

	// Movimiento la llama tiembla rapido y la textura sube
	shader.SetFloat("tiempo", tiempo + posicion.x);   // cada fuego desfasado
	shader.SetFloat("amplitud", 0.08f);
	shader.SetFloat("frecuencia", 6.0f);
	shader.SetFloat("velocidad", 10.0f);
	shader.SetVec2("velocidad_u_v", glm::vec2(0.0f, -0.8f));   // la textura sube
	shader.SetFloat("repeticion_u_v", 1.0f);

	// Aspecto sin luz
	shader.SetVec3("color_base", glm::vec3(1.0f));
	shader.SetFloat("transparencia", 1.0f);
	shader.SetFloat("desvanecer", 1.0f);
	shader.SetInt("usar_luz", 0);

	// Mezcla
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	textura.UseTexture();
	malla->RenderMeshModel();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glUseProgram(shader_anterior);
}
//>>>>>>>>>>>>>>>>>>>>>> Vegetacion >>>>>>>>>>>>>>>>>>>>>>


// Posiciones
struct Lugar
{
	float x, z, escala, giro;
};

//rboles sueltos en el pasto
static const Lugar arboles[] = {
	{ -40, -110, 2.0f,   0 }, {  45, -115, 1.8f,  60 }, { -150,  60, 2.2f, 120 },
	{ -120, 110, 1.9f, 200 }, { 130,   80, 2.1f,  30 }, {  120, 130, 1.8f, 270 },
	{ -60,  150, 2.0f,  90 }, {  60,  150, 2.2f, 150 }, {    0, 150, 1.9f, 300 },
	{ -30,  -20, 1.7f,  45 }, {  40,  -10, 1.8f, 210 }, { -135, 140, 2.0f, 330 },
};

//Arbustos
static const Lugar arbustos[] = {
	{ -25, -35, 1.6f,   0 }, {  30, -40, 1.4f,  80 }, { -55, -100, 1.8f, 160 },
	{  60, -90, 1.5f, 240 }, { -100, 40, 1.7f,  20 }, {  100,  40, 1.6f, 100 },
	{ -90, 120, 1.8f, 180 }, {  95, 105, 1.5f, 260 }, {  -20, 140, 1.6f, 340 },
	{  25, 140, 1.4f,  50 }, { -145, 15, 1.7f, 130 }, {  145,  10, 1.6f, 210 },
};

//Rocas en el pasto
static const Lugar rocas[] = {
	{ -70, -130, 3.0f,   0 }, {  80, -135, 2.5f,  70 }, { -140,  90, 3.5f, 140 },
	{ 140,   60, 2.8f, 210 }, { -10, -120, 2.0f, 280 }, {   50, -60, 1.8f, 350 },
	{ -75,   20, 2.2f,  40 }, { 150, -110, 3.2f, 110 }, { -150, -130, 3.0f, 180 },
	{  10,  160, 2.6f, 250 },
};

//Rocas en la orilla del lago
static const Lugar rocas_orilla[] = {
	{  90, 93, 1.8f,  0 }, { -74, 112, 2.2f, 60 }, { -90, 47, 1.6f, 120 },
	{ -33,  8, 2.0f, 180 }, {  74,  28, 1.7f, 240 }, {   0, 136, 2.4f, 300 },
};

//Dentro del lago
static const Lugar rocas_lago[] = {
	{ -30, 60, 4.0f, 0 }, { 25, 80, 3.0f, 70 }, { 0, 100, 4.5f, 140 },
	{  45, 55, 3.5f, 210 }, { -50, 85, 3.0f, 280 }, { 10, 45, 2.5f, 350 },
};
static const Lugar algas[] = {
	{ -15, 70, 1.4f,  0 }, { 20, 62, 1.2f,  45 }, { 35, 90, 1.5f,  90 }, { -40, 75, 1.3f, 135 },
	{  -5, 95, 1.6f, 180 }, { 55, 75, 1.2f, 225 }, { -60, 60, 1.4f, 270 }, { 15, 105, 1.5f, 315 },
};

bool Vegetacion::EnZonaLibre(float x, float z) const
{
	// Aldea
	if (glm::distance(glm::vec2(x, z), glm::vec2(-110.0f, -40.0f)) < 50.0f) return true;
	if (glm::distance(glm::vec2(x, z), glm::vec2(110.0f, -30.0f)) < 45.0f) return true;
	if (glm::distance(glm::vec2(x, z), glm::vec2(0.0f, -65.0f)) < 30.0f) return true;
	return false;
}

void Vegetacion::Crear(const Terreno& t)
{
	terreno = &t;

	arbol1.LoadModel("assets/planeta_azul/modelos/arbol_hojas_1.obj");
	arbol2.LoadModel("assets/planeta_azul/modelos/arbol_hojas_2.obj");
	pino1.LoadModel("assets/planeta_azul/modelos/pino_1.obj");
	pino2.LoadModel("assets/planeta_azul/modelos/pino_2.obj");
	arbusto.LoadModel("assets/planeta_azul/modelos/arbusto.obj");
	arbusto_flores.LoadModel("assets/planeta_azul/modelos/arbusto_flores.obj");
	pasto.LoadModel("assets/planeta_azul/modelos/pasto_alto.obj");
	flores.LoadModel("assets/planeta_azul/modelos/flores.obj");
	roca1.LoadModel("assets/planeta_azul/modelos/roca_mediana_1.obj");
	roca2.LoadModel("assets/planeta_azul/modelos/roca_mediana_2.obj");
	piedra.LoadModel("assets/planeta_azul/modelos/piedra_redonda.obj");
	alga.LoadModel("assets/planeta_azul/modelos/alga.obj");

	shader.CreateFromString(shader.ReadFile("shaders/shader_light.vert").c_str(), vegetacion_frag);
}

void Vegetacion::DibujarEn(Model& modelo, float x, float z, float escala, float giro, glm::vec3 color)
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, terreno->Altura(x, z), z));   // sobre el suelo
	model = glm::rotate(model, glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(u_color, 1, glm::value_ptr(color));
	modelo.RenderModel();
}

void Vegetacion::Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces)
{
	// Guardar el shader de main para regresar a el al final
	GLint shader_main = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_main);

	// Mismos uniforms que main.cpp le manda a shader_light
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, num_luces);      // fogata y antorchas
	shader.SetSpotLights(nullptr, 0);
	material_plantas.UseMaterial(shader.GetSpecularIntensityLocation(), shader.GetShininessLocation());
	u_model = shader.GetModelLocation();
	u_color = shader.getColorLocation();

	//Bosque alrededor del terreno dos filas de arboles en cada lad
	Model* tipos_arbol[4] = { &arbol1, &arbol2, &pino1, &pino2 };
	int i = 0;
	for (float t = -180.0f; t <= 180.0f; t += 10.0f)
	{
		float escala = 1.8f + (i % 3) * 0.4f;   // tamanos variados
		float giro = i * 47.0f;
		// fila de afuera y fila de adentro
		DibujarEn(*tipos_arbol[i % 4],       t, -178.0f, escala, giro);
		DibujarEn(*tipos_arbol[(i + 1) % 4], t,  178.0f, escala, giro);
		DibujarEn(*tipos_arbol[(i + 2) % 4], -178.0f, t, escala, giro);
		DibujarEn(*tipos_arbol[(i + 3) % 4],  178.0f, t, escala, giro);
		float u = t + 5.0f;
		if (!EnZonaLibre(u, -163.0f)) DibujarEn(*tipos_arbol[(i + 2) % 4], u, -163.0f, escala, giro + 90);
		if (!EnZonaLibre(u,  163.0f)) DibujarEn(*tipos_arbol[(i + 3) % 4], u,  163.0f, escala, giro + 90);
		if (!EnZonaLibre(-163.0f, u)) DibujarEn(*tipos_arbol[i % 4], -163.0f, u, escala, giro + 90);
		if (!EnZonaLibre( 163.0f, u)) DibujarEn(*tipos_arbol[(i + 1) % 4],  163.0f, u, escala, giro + 90);
		i++;
	}

	// Arboles, arbustos y rocas
	int k = 0;
	for (const Lugar& l : arboles)       // se alternan: arbol, pino, arbol, pino...
		DibujarEn(k++ % 2 == 0 ? arbol1 : pino1, l.x, l.z, l.escala, l.giro);
	k = 0;
	for (const Lugar& l : arbustos)      // se alternan: arbusto, arbusto con flores...
		DibujarEn(k++ % 2 == 0 ? arbusto : arbusto_flores, l.x, l.z, l.escala, l.giro);
	for (const Lugar& l : rocas)    DibujarEn(roca1, l.x, l.z, l.escala, l.giro, glm::vec3(1.3f));
	for (const Lugar& l : rocas_orilla) DibujarEn(roca2, l.x, l.z, l.escala, l.giro, glm::vec3(1.3f));

	//Pasto y flores
	int n = 0;
	for (float x = -165.0f; x <= 165.0f; x += 15.0f)
	{
		for (float z = -165.0f; z <= 165.0f; z += 15.0f)
		{
			n++;
			float px = x + (n % 3) * 4.0f;      // se recorre un poco para que no se vea en fila
			float pz = z + (n % 4) * 3.0f;
			if (terreno->DistanciaLago(px, pz) < 1.2f || EnZonaLibre(px, pz))
				continue;
			if (n % 3 == 0)
				DibujarEn(flores, px, pz, 1.3f, n * 30.0f);
			else
				DibujarEn(pasto, px, pz, 1.6f, n * 30.0f);
			DibujarEn(piedra, px + 6.0f, pz + 2.0f, 3.0f, n * 50.0f);   // una piedrita junto
		}
	}

	//Fondo del lago: rocas y algas
	for (const Lugar& l : rocas_lago) DibujarEn(roca1, l.x, l.z, l.escala, l.giro, glm::vec3(0.7f, 0.85f, 0.95f));
	for (const Lugar& l : algas)     DibujarEn(alga, l.x, l.z, l.escala, l.giro, glm::vec3(0.45f, 0.85f, 0.7f));

	glUseProgram(shader_main);
}
//>>>>>>>>>>>>>>>>>>>>>> PezAbisal >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones respecto al centro del cuerpo (en metros, antes de escalar)
static const glm::vec3 abisal_piv_aleta_izq(-0.38f, -0.36f, -0.16f);
static const glm::vec3 abisal_piv_aleta_der( 0.48f, -0.30f, -0.11f);
static const glm::vec3 piv_farol    ( 0.06f,  1.19f,  0.64f);

// Recorrido: elipse en lo hondo del lago, siempre un poco arriba del fondo
static const float abisal_centro_x = -10.0f, abisal_centro_z = 65.0f;
static const float abisal_radio_x = 38.0f, abisal_radio_z = 24.0f;
static const float abisal_velocidad = 0.07f;       // radianes por segundo
static const float sobre_fondo = 5.0f;      // metros arriba del fondo
static const float abisal_escala = 2.4f;

void PezAbisal::Crear(const Terreno& t)
{
	terreno = &t;
	cuerpo.LoadModel("assets/planeta_azul/modelos/pez_abisal_cuerpo.obj");
	aleta_izq.LoadModel("assets/planeta_azul/modelos/pez_abisal_aleta_izq.obj");
	aleta_der.LoadModel("assets/planeta_azul/modelos/pez_abisal_aleta_der.obj");
	farol.LoadModel("assets/planeta_azul/modelos/pez_abisal_farol.obj");

	// Halo del farol: un cuadrado que siempre mira a la camara
	halo = Texture("assets/planeta_azul/texturas/halo_farol.png");
	halo.LoadTextureA();
	GLfloat vertices[] = { -.5f,-.5f,0, 0,0, 0,0,-1,   .5f,-.5f,0, 1,0, 0,0,-1,
	                        .5f, .5f,0, 1,1, 0,0,-1,  -.5f, .5f,0, 0,1, 0,0,-1 };
	unsigned int indices[] = { 0,1,2, 0,2,3 };
	plano_halo.CreateMeshModel(vertices, indices, 32, 6);
}

void PezAbisal::Actualizar(float dt, Window& ventana)
{
	tiempo += dt;
	angulo += abisal_velocidad * dt;

	// Tecla P prender / apagar la linterna
	if (ventana.TeclaPresionada(GLFW_KEY_P))
		linterna = !linterna;

	// Posicion en la elipse, siguiendo el fondo del lago (sin salirse del agua)
	posicion.x = abisal_centro_x + abisal_radio_x * cosf(angulo);
	posicion.z = abisal_centro_z + abisal_radio_z * sinf(angulo);
	posicion.y = glm::min(terreno->Altura(posicion.x, posicion.z) + sobre_fondo, -6.0f)
		+ 0.4f * sinf(tiempo * 1.3f);

	// Rumbo = derivada de la elipse
	giro = atan2f(-abisal_radio_x * sinf(angulo), abisal_radio_z * cosf(angulo));

	// El reflector sale del farol hacia adelante y un poco hacia abajo
	glm::mat4 m = MatrizCuerpo();
	glm::vec3 origen = glm::vec3(m * glm::vec4(piv_farol, 1.0f));
	glm::vec3 direccion = glm::normalize(glm::vec3(m * glm::vec4(0.0f, -0.6f, 1.0f, 0.0f)));
	foco = SpotLight(0.65f, 0.95f, 1.0f,          // azul claro
		0.0f, 4.0f,                              // ambiental, difusa
		origen.x, origen.y, origen.z,
		direccion.x, direccion.y, direccion.z,
		1.0f, 0.022f, 0.0019f,                   // atenuacion (alcance de unos 60 m)
		28.0f);                                  // abertura del cono en grados
}

glm::mat4 PezAbisal::MatrizCuerpo() const
{
	float balanceo = glm::radians(6.0f) * sinf(tiempo * 2.0f);   // nada moviendose de lado a lado
	glm::mat4 model(1.0f);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro + balanceo, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(abisal_escala));
	return model;
}

void PezAbisal::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	float aleteo = glm::radians(25.0f) * sinf(tiempo * 4.0f);

	// --- Cuerpo (padre) ---
	glm::mat4 model = MatrizCuerpo();
	glm::mat4 modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	// --- Aletas: suben y bajan en sentidos contrarios ---
	model = modelaux;
	model = glm::translate(model, abisal_piv_aleta_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, abisal_piv_aleta_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_der.RenderModel();

	// --- Farol: si la linterna esta prendida se dibuja sin sombra (modo 2 = color plano) ---
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint modo = glGetUniformLocation(programa, "modo_efecto");
	model = modelaux;
	model = glm::translate(model, piv_farol);
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	if (linterna) glUniform1i(modo, 2);
	farol.RenderModel();
	glUniform1i(modo, 0);
}

void PezAbisal::DibujarHalo(const ContextoRender& ctx)
{
	if (!linterna) return;
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint modo = glGetUniformLocation(programa, "modo_efecto");
	GLint opacidad = glGetUniformLocation(programa, "opacidad_particula");

	// Cuadrado en el farol girado hacia la camara (inversa de la rotacion de la vista)
	glm::vec3 p = glm::vec3(MatrizCuerpo() * glm::vec4(piv_farol, 1.0f));
	glm::mat4 model = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
	model[3] = glm::vec4(p, 1.0f);
	model = glm::scale(model, glm::vec3(2.5f + 0.2f * sinf(tiempo * 5.0f)));   // late un poco

	glUniform1i(modo, 4);
	glUniform1f(opacidad, 0.9f);
	glUniform3f(ctx.u_color, 0.6f, 0.95f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // se suma: brilla
	glDepthMask(GL_FALSE);
	halo.UseTexture();
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	plano_halo.RenderMeshModel();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glUniform1i(modo, 0);
	glUniform1f(opacidad, 1.0f);
	glUniform3f(ctx.u_color, 1.0f, 1.0f, 1.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> ArbolesLuminosos >>>>>>>>>>>>>>>>>>>>>>


struct PuntoArbol
{
	float x, z, escala, giro;
};

// En la parte honda del lago; el pez abisal pasa entre ellos
static const PuntoArbol pos_arboles[ArbolesLuminosos::num_arboles] = {
	{   6.0f,  71.0f, 10.0f,   0.0f }, { -26.0f,  74.0f,  9.0f,  60.0f },
	{  -9.0f,  53.0f, 11.0f, 130.0f }, {  37.0f,  70.0f,  9.5f, 200.0f },
	{  12.0f,  94.0f,  8.5f, 250.0f }, { -36.0f,  97.0f, 10.0f, 310.0f },
	{ -48.0f,  52.0f,  9.0f,  20.0f }, {  24.0f,  44.0f,  8.0f, 170.0f },
};

static const float alto_copa = 0.75f;      // donde va la luz (fraccion de la altura del arbol)

void ArbolesLuminosos::Crear(const Terreno& t)
{
	terreno = &t;
	// Mismo modelo que el planeta morado (no se duplica en el proyecto)
	arbol.LoadModel("assets/planeta_morado/modelos/arbol_luminoso.obj", false, false);
}

void ArbolesLuminosos::Actualizar(float dt, bool es_de_noche, Window& ventana)
{
	tiempo += dt;

	// Igual que las antorchas: el ciclo los prende al anochecer y la tecla I a mano
	if (es_de_noche != era_de_noche)
	{
		encendidos = es_de_noche;
		era_de_noche = es_de_noche;
	}
	if (ventana.TeclaPresionada(GLFW_KEY_I))
		encendidos = !encendidos;

	// El brillo sube o baja poco a poco (no de golpe)
	float objetivo = encendidos ? 1.0f : 0.0f;
	brillo += (objetivo - brillo) * glm::min(1.0f, 2.0f * dt);

	// Una luz morada en la copa de cada arbol
	num_luces = 0;
	if (brillo < 0.02f) return;
	for (int i = 0; i < num_arboles; i++)
	{
		const PuntoArbol& a = pos_arboles[i];
		glm::vec3 p(a.x, terreno->Altura(a.x, a.z) + a.escala * alto_copa, a.z);
		float pulso = 0.25f * sinf(tiempo * 1.5f + i);          // laten despacio
		pos_luces[num_luces] = p;
		luces[num_luces++] = PointLight(0.7f, 0.3f, 1.0f,     // morado
			0.0f, (2.0f + pulso) * brillo,
			p.x, p.y, p.z,
			1.0f, 0.07f, 0.017f);                             // alcance de unos 40 m
	}
}

void ArbolesLuminosos::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Los frutos azules brillan con "brillo_frutos" del shader_light
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint frutos = glGetUniformLocation(programa, "brillo_frutos");
	glUniform1f(frutos, 0.3f + 3.4f * brillo);

	for (int i = 0; i < num_arboles; i++)
	{
		const PuntoArbol& a = pos_arboles[i];
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(a.x, terreno->Altura(a.x, a.z) - 0.2f, a.z));
		model = glm::rotate(model, glm::radians(a.giro), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(a.escala));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		arbol.RenderModel();
	}
	glUniform1f(frutos, 0.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Aldea >>>>>>>>>>>>>>>>>>>>>>


// Posiciones al centro de la aldea
struct PuntoAldea
{
	float x, z, giro;
};

// Chozas mirando hacia la fogata 4 cerca y 3 mas lejos.
// La entrada de la aldea queda libre del lado +X.
static const int num_chozas = 7;
static const PuntoAldea pos_chozas[num_chozas] = {
	{  18.4f,  18.4f, 225.0f },
	{ -18.4f,  18.4f, 135.0f },
	{ -18.4f, -18.4f,  45.0f },
	{  18.4f, -18.4f, 315.0f },
	{   0.0f,  40.0f, 180.0f },
	{ -40.0f,   0.0f,  90.0f },
	{   0.0f, -40.0f,   0.0f },
};

// Antorchas
//   3 junto a la fogata, 2 en la entrada y 8 en un circulo de radio 33
static const int num_antorchas = 13;
static const PuntoAldea pos_antorchas[num_antorchas] = {
	{  13.0f,   0.0f, 0.0f }, {  -6.5f,  11.3f, 0.0f }, {  -6.5f, -11.3f, 0.0f },
	{  40.0f,  -8.0f, 0.0f }, {  40.0f,   8.0f, 0.0f },
	{  30.5f,  12.6f, 0.0f }, {  12.6f,  30.5f, 0.0f }, { -12.6f,  30.5f, 0.0f }, { -30.5f,  12.6f, 0.0f },
	{ -30.5f, -12.6f, 0.0f }, { -12.6f, -30.5f, 0.0f }, {  12.6f, -30.5f, 0.0f }, {  30.5f, -12.6f, 0.0f },
};

static const float alto_antorcha = 3.4f;   // donde empieza la llama

int Aldea::NumChozas() const
{
	return num_chozas;
}

glm::vec2 Aldea::PosicionChoza(int i) const
{
	return centro + glm::vec2(pos_chozas[i].x, pos_chozas[i].z);
}

glm::vec3 Aldea::SobreElSuelo(float x, float z) const
{
	return glm::vec3(x, terreno->Altura(x, z), z);
}

void Aldea::Crear(const Terreno& t)
{
	terreno = &t;

	choza_muu.LoadModel("assets/planeta_azul/modelos/choza_muu.obj");
	choza_mantah.LoadModel("assets/planeta_azul/modelos/choza_mantah.obj");
	choza_krakee.LoadModel("assets/planeta_azul/modelos/choza_krakee.obj");
	choza_antuk.LoadModel("assets/planeta_azul/modelos/choza_antuk.obj");
	antorcha.LoadModel("assets/planeta_azul/modelos/antorcha.obj");
	fogata.LoadModel("assets/planeta_azul/modelos/fogata.obj");

	fuego.Crear();
}

void Aldea::Actualizar(float tiempo, bool es_de_noche, Window& ventana)
{
	// Tecla U para prender o apagar la fogata
	if (ventana.TeclaPresionada(GLFW_KEY_U))
		fogata_encendida = !fogata_encendida;

	// Se vuelve a llenar el arreglo de luces cada frame con las que esten prendidas
	num_luces = 0;

	if (fogata_encendida)
	{
		glm::vec3 p = SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 1.0f, 0.0f);
		float parpadeo = 0.3f * sinf(tiempo * 9.0f);           // la luz tiembla como fuego
		pos_luces[num_luces] = p;
        luces[num_luces++] = PointLight(1.0f, 0.55f, 0.2f,        // color naranja
			0.0f, 3.0f + parpadeo,                               // ambiental, difusa
			p.x, p.y, p.z,                                       // posicion
			1.0f, 0.045f, 0.007f);                              // atenuacion
	}

	if (es_de_noche != era_de_noche)
    {
        era_de_noche = es_de_noche;
        antorchas_encendidas = es_de_noche;
    }
    if (ventana.TeclaPresionada(GLFW_KEY_O)) antorchas_encendidas = !antorchas_encendidas;
	if (antorchas_encendidas)
	{
		for (int i = 0; i < num_antorchas && num_luces < max_point_lights; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z)
				+ glm::vec3(0.0f, alto_antorcha, 0.0f);
			float parpadeo = 0.2f * sinf(tiempo * 12.0f + i);
			pos_luces[num_luces] = p;
            luces[num_luces++] = PointLight(1.0f, 0.6f, 0.25f,
				0.0f, 1.5f + parpadeo,
				p.x, p.y, p.z,
				1.0f, 0.09f, 0.032f);                            // alcance
		}
	}
}

void Aldea::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Chozas
	Model* chozas[4] = { &choza_muu, &choza_mantah, &choza_krakee, &choza_antuk };
	for (int i = 0; i < num_chozas; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + pos_chozas[i].x, centro.y + pos_chozas[i].z)
			- glm::vec3(0.0f, 0.3f, 0.0f));                     // un poco enterrada por si hay desnivel
		model = glm::rotate(model, glm::radians(pos_chozas[i].giro), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		chozas[i % 4]->RenderModel();      // se repiten los 4 modelos de choza
	}

	//Antorchas
	for (int i = 0; i < num_antorchas; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		antorcha.RenderModel();
	}

	//Fogata
	glm::mat4 model(1.0f);
	model = glm::translate(model, SobreElSuelo(centro.x, centro.y));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	fogata.RenderModel();
}

void Aldea::DibujarLlamas(const ContextoRender& ctx, float tiempo)
{
	// Llama en la punta de cada antorcha
	if (antorchas_encendidas)
	{
		for (int i = 0; i < num_antorchas; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z);
			fuego.Dibujar(ctx, p + glm::vec3(0.0f, alto_antorcha - 0.1f, 0.0f), 0.6f, 1.0f, tiempo);
		}
	}

	// Llama grande de la fogata solo si esta prendida
	if (fogata_encendida)
		fuego.Dibujar(ctx, SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 0.2f, 0.0f), 2.0f, 3.0f, tiempo);
}
//>>>>>>>>>>>>>>>>>>>>>> Peces >>>>>>>>>>>>>>>>>>>>>>


// Recorrido de cada Air fish (forma de 8)
struct RecorridoOcho
{
	float cx, cz;          // centro del 8
	float ancho_x, largo_z;  // tamano del 8
	float profundidad;     // altura
	float velocidad;       // que tan rapido recorre el 8
	float fase;            // para que no empiecen todos en el mismo punto
	float escala;
};

static const RecorridoOcho peces_aire[] = {
	//  cx    cz   ancho_x largo_z  prof   vel    fase  escala
	{   0.0f, 70.0f, 40.0f, 22.0f,  -6.0f, 0.12f, 0.0f, 1.5f },
	{ -10.0f, 75.0f, 30.0f, 18.0f, -12.0f, 0.15f, 2.0f, 1.2f },
	{  15.0f, 65.0f, 35.0f, 20.0f, -18.0f, 0.10f, 4.0f, 1.8f },
};

// Cardumen de Peepers todos dan vueltas al mismo circulo, uno detras de otro
static const float centro_cardumen_x = 10.0f, centro_cardumen_z = 80.0f;
static const float radio_cardumen = 18.0f;
static const float velocidad_cardumen = 0.3f;
static const int num_peepers = 6;

void Peces::Crear()
{
	air_fish.LoadModel("assets/planeta_azul/modelos/pez_aire.obj");
	peeper.LoadModel("assets/planeta_azul/modelos/peeper.obj");
}

void Peces::Actualizar(float dt)
{
	tiempo += dt;
}

void Peces::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Air fish forma de 8
	for (const RecorridoOcho& r : peces_aire)
	{
		float t = tiempo * r.velocidad + r.fase;

		// Posicion sobre el 8
		float x = r.cx + r.ancho_x * sinf(t);
		float z = r.cz + r.largo_z * sinf(2.0f * t) / 2.0f;
		float y = r.profundidad + 0.8f * sinf(3.0f * t);          // sube y baja

		// Direccion de avance = derivada de la posicion
		float dx = r.ancho_x * cosf(t);
		float dz = r.largo_z * cosf(2.0f * t);
		float giro = atan2f(dx, dz);                              // mirar hacia donde avanza
		float coleteo = glm::radians(10.0f) * sinf(tiempo * 6.0f + r.fase);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::rotate(model, giro + coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(r.escala));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		air_fish.RenderModel();
	}

	//Peepers cardumen en circulo
	for (int i = 0; i < num_peepers; i++)
	{
		float t = tiempo * velocidad_cardumen - i * 0.35f;   // cada uno va un poco atras del anterior
		float radio = radio_cardumen + (i % 3) * 2.0f;        // no todos en la misma linea
		float x = centro_cardumen_x + radio * cosf(t);
		float z = centro_cardumen_z + radio * sinf(t);
		float y = -10.0f - (i % 2) * 2.5f + 0.5f * sinf(tiempo * 2.0f + i);

		// Direccion de avance en el circulo: (-sen t, cos t)
		float giro = atan2f(-sinf(t), cosf(t));
		float coleteo = glm::radians(12.0f) * sinf(tiempo * 9.0f + i);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::rotate(model, giro + coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		peeper.RenderModel();
	}
}
//>>>>>>>>>>>>>>>>>>>>>> Necromorfos >>>>>>>>>>>>>>>>>>>>>>


static const float altura_necromorfo = 3.4f;

// La caminata
// Si el circulo los mueve a otra velocidad, los pies patinan
static const float velocidad_caminata = 2.05f;

// Que necromorfos (por numero de choza) bailan cuando se prende la fogata
static const bool bailan[] = { true, false, true, false, false, false, false };

void Necromorfos::Crear(const Terreno& t, const Aldea& aldea)
{
	terreno = &t;
	fogata = aldea.centro;

	// Malla + esqueleto + caminata (animacion 0) y despues el baile (animacion 1)
	necromorfo.Cargar("assets/planeta_azul/modelos/necromorfo_caminar.fbx", "assets/planeta_azul/texturas/necromorfo.jpg",true);
	anim_caminar = 0;
	anim_bailar = necromorfo.AgregarAnimacion("assets/planeta_azul/modelos/necromorfo_bailar.fbx");
	// La caminata no venia "In Place" asi que hay que moverle
	necromorfo.QuitarAvance(anim_caminar, "mixamorig:Hips");

	shader.CreateFromString(esqueleto_vert, shader.ReadFile("shaders/shader_light.frag").c_str());

	// Uno por choza
	num_caminantes = glm::min(aldea.NumChozas(), max_caminantes);
	for (int i = 0; i < num_caminantes; i++)
	{
		glm::vec2 choza = aldea.PosicionChoza(i);
		glm::vec2 hacia_fogata = glm::normalize(fogata - choza);

		Caminante& c = caminantes[i];
		c.centro = choza + hacia_fogata * 9.0f;
		c.radio = 3.5f;
		c.angulo = i * 1.3f;                                       // cada uno empieza en otro punto
		float sentido = (i % 2 == 0) ? 1.0f : -1.0f;               // unos van al reves
		c.velocidad = sentido * velocidad_caminata / c.radio;       // v = w * r  ->  w = v / r
		c.baila_con_fogata = (i < 7) ? bailan[i] : false;
	}
}

void Necromorfos::Actualizar(float dt, bool fogata_encendida)
{
	tiempo += dt;
	bailando = fogata_encendida;

	for (int i = 0; i < num_caminantes; i++)
	{
		// Los de la aldea se quedan quietos mientras bailan
		if (bailando && caminantes[i].baila_con_fogata)
			continue;
		caminantes[i].angulo += caminantes[i].velocidad * dt;
	}
}

// Circuito peatonal cerrado con una vuelta al lago y otra a la aldea.
static glm::vec2 PuntoPatrulla(float distancia)
{
    const glm::vec2 hitos[] = {
        {-96, 71}, {-73, 128}, {0, 130}, {65, 118}, {101, 94}, {96, 68},
        {84, 17}, {44, 4}, {0, 4}, {-35, -4}, {-58, 13}, {-85, 1},
        {-130, 6}, {-154, -24}, {-150, -65}, {-111, -87}, {-72, -64},
        {-65, -32}, {-78, 1}, {-108, 27}, {-112, 53}};
    constexpr int cantidad = sizeof(hitos) / sizeof(hitos[0]);
    float longitud = 0.0f;
    for (int i = 0; i < cantidad; ++i)
        longitud += glm::distance(hitos[i], hitos[(i + 1) % cantidad]);
    distancia = std::fmod(distancia, longitud);
    if (distancia < 0) distancia += longitud;
    for (int i = 0; i < cantidad; ++i)
    {
        const glm::vec2 a = hitos[i], b = hitos[(i + 1) % cantidad];
        const float tramo = glm::distance(a, b);
        if (distancia <= tramo)
        {
            const float t = distancia / tramo;
            const glm::vec2 m = .5f * (b - hitos[(i + cantidad - 1) % cantidad]);
            const glm::vec2 n = .5f * (hitos[(i + 2) % cantidad] - a);
            return glm::mix(a, b, t * t * (3.0f - 2.0f * t)) +
                   t * (1.0f - t) * ((1.0f - t) * m - t * n);
        }
        distancia -= tramo;
    }
    return hitos[0];
}

glm::vec3 Necromorfos::PosicionPatrulla() const
{
    const glm::vec2 punto = PuntoPatrulla(tiempo * velocidad_caminata);
    return glm::vec3(punto.x, terreno->Altura(punto.x, punto.y), punto.y);
}

glm::vec3 Necromorfos::RumboPatrulla() const
{
    const glm::vec2 delta = PuntoPatrulla(tiempo * velocidad_caminata + .1f) -
                            PuntoPatrulla(tiempo * velocidad_caminata - .1f);
    return glm::normalize(glm::vec3(delta.x, 0, delta.y));
}

void Necromorfos::DibujarUno(const Caminante& c, int animacion, float tiempo_animacion, GLuint u_model)
{
	// Posicion en el circulo
	float x = c.centro.x + c.radio * cosf(c.angulo);
	float z = c.centro.y + c.radio * sinf(c.angulo);

	float giro;
	if (animacion == anim_bailar)
	{
		giro = atan2f(fogata.x - x, fogata.y - z);              // voltea hacia la fogata
	}
	else
	{
		// Direccion de avance = derivada del circulo (con el signo de la velocidad)
		float s = (c.velocidad >= 0.0f) ? 1.0f : -1.0f;
		giro = atan2f(-sinf(c.angulo) * s, cosf(c.angulo) * s);
	}

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, terreno->Altura(x, z), z));
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(altura_necromorfo))*necromorfo.Normalizacion();
    if (&c == caminantes)
    {
        const glm::vec3 rumbo = RumboPatrulla();
        model = glm::translate(glm::mat4(1), PosicionPatrulla());
        model = glm::rotate(model, atan2f(rumbo.x, rumbo.z), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(altura_necromorfo))*necromorfo.Normalizacion();
        animacion = anim_caminar;
    }
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    necromorfo.CalcularPose(animacion, tiempo_animacion);
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	necromorfo.Dibujar((GLuint)programa);
}

void Necromorfos::Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces)
{
	GLint shader_main = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_main);

	// Mismos uniforms que main.cpp manda al shader de luz
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, num_luces);
	shader.SetSpotLights(nullptr, 0);
	glUniform3f(shader.getColorLocation(), 1.0f, 1.0f, 1.0f);
	glUniform1f(shader.GetSpecularIntensityLocation(), 0.3f);
	glUniform1f(shader.GetShininessLocation(), 16.0f);

	for (int i = 0; i < num_caminantes; i++)
	{
		const Caminante& c = caminantes[i];
		bool baila = bailando && c.baila_con_fogata;
		// Cada uno con un desfase en su animacion para que no se muevan identicos
		DibujarUno(c, baila ? anim_bailar : anim_caminar, tiempo + i * 0.4f, shader.GetModelLocation());
	}

	glUseProgram(shader_main);
}
//>>>>>>>>>>>>>>>>>>>>>> Stalker >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones
static const glm::vec3 piv_boca_sup     ( 0.00f,  0.05f,  1.08f);
static const glm::vec3 piv_boca_inf     ( 0.00f, -0.02f,  0.98f);
static const glm::vec3 piv_aleta_del_izq ( 0.21f, -0.19f,  0.68f);
static const glm::vec3 piv_aleta_del_der (-0.21f, -0.19f,  0.68f);
static const glm::vec3 piv_aleta_tras_izq( 0.16f, -0.21f, -0.49f);
static const glm::vec3 piv_aleta_tras_der(-0.16f, -0.21f, -0.49f);

// Recorrido elipse dentro del lago
static const float centro_x = 0.0f, centro_z = 70.0f;
static const float radio_x = 45.0f, radio_z = 30.0f;
static const float profundidad = -5.0f;     // el agua esta en y = -1
static const float velocidad = 0.12f;       // radianes por segundo
static const float escala = 2.4f;           

void Stalker::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/stalker_cuerpo.obj");
	boca_sup.LoadModel("assets/planeta_azul/modelos/stalker_boca_sup.obj");
	boca_inf.LoadModel("assets/planeta_azul/modelos/stalker_boca_inf.obj");
	aleta_del_izq.LoadModel("assets/planeta_azul/modelos/stalker_aleta_del_izq.obj");
	aleta_del_der.LoadModel("assets/planeta_azul/modelos/stalker_aleta_del_der.obj");
	aleta_tras_izq.LoadModel("assets/planeta_azul/modelos/stalker_aleta_tras_izq.obj");
	aleta_tras_der.LoadModel("assets/planeta_azul/modelos/stalker_aleta_tras_der.obj");
}

void Stalker::Actualizar(float dt)
{
	tiempo += dt;
	angulo += velocidad * dt;

	// Posicion en la elipse, subiendo y bajando un poco
	posicion.x = centro_x + radio_x * cosf(angulo);
	posicion.z = centro_z + radio_z * sinf(angulo);
	posicion.y = profundidad + 0.5f * sinf(tiempo * 1.5f);

	// Mira hacia donde avanza derivada de la elipse = (-A sen t, B cos t)
	giro = atan2f(-radio_x * sinf(angulo), radio_z * cosf(angulo));
}

void Stalker::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float remo = glm::radians(25.0f) * sinf(tiempo * 3.0f);                 // aletas delanteras
	float remo_tras = glm::radians(25.0f) * sinf(tiempo * 3.0f + 3.142f);   // traseras: al reves
	float abre = 0.5f + 0.5f * sinf(tiempo * 1.2f);                         // 0 = cerrada, 1 = abierta
	float vaiven = glm::radians(6.0f) * sinf(tiempo * 3.0f);               // el cuerpo se menea al nadar

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro + vaiven, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Boca superior
	model = modelaux;
	model = glm::translate(model, piv_boca_sup);
	model = glm::rotate(model, glm::radians(-6.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca_sup.RenderModel();

	//Boca inferior
	model = modelaux;
	model = glm::translate(model, piv_boca_inf);
	model = glm::rotate(model, glm::radians(14.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca_inf.RenderModel();

	//Aletas delanteras
	model = modelaux;
	model = glm::translate(model, piv_aleta_del_izq);
	model = glm::rotate(model, remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_del_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_del_der);
	model = glm::rotate(model, -remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_del_der.RenderModel();

	//Aletas traseras
	model = modelaux;
	model = glm::translate(model, piv_aleta_tras_izq);
	model = glm::rotate(model, remo_tras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_tras_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_tras_der);
	model = glm::rotate(model, -remo_tras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_tras_der.RenderModel();
}
//>>>>>>>>>>>>>>>>>>>>>> Pidgeys >>>>>>>>>>>>>>>>>>>>>>


// Donde se pega cada ala al cuerpo
static const glm::vec3 hombro_izq( 0.28f, 0.05f, 0.04f);
static const glm::vec3 hombro_der(-0.28f, 0.05f, 0.04f);

// Vuelo de cada ave de la parvada
struct Vuelo
{
	float cx, cz;       // centro del circulo
	float radio;
	float altura;
	float velocidad;    // radianes por segundo (negativa = al reves)
	float fase;         // para que no empiecen en el mismo punto
	float escala;
};

static const int num_aves = 8;
static const Vuelo vuelos[num_aves] = {
	//   cx      cz    radio  altura  vel    fase  escala
	{ -110.0f, -40.0f, 35.0f, 28.0f,  0.20f, 0.0f, 1.0f },   // sobre la aldea
	{ -110.0f, -40.0f, 35.0f, 30.0f,  0.20f, 0.6f, 0.9f },
	{ -110.0f, -40.0f, 35.0f, 27.0f,  0.20f, 1.2f, 1.1f },
	{    0.0f,  70.0f, 60.0f, 35.0f, -0.15f, 0.0f, 1.0f },   // sobre el lago (alto)
	{    0.0f,  70.0f, 60.0f, 37.0f, -0.15f, 0.5f, 1.0f },
	{  110.0f, -30.0f, 30.0f, 25.0f,  0.25f, 2.0f, 0.9f },   // zona de criaturas
	{  110.0f, -30.0f, 30.0f, 26.0f,  0.25f, 2.7f, 1.0f },
	{    0.0f, -40.0f, 45.0f, 32.0f, -0.18f, 3.0f, 1.1f },   // entrada
};

static const float tiempo_reaparecer = 8.0f;   // segundos para que salga otra presa

void Pidgeys::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/pidgey_cuerpo.obj");
	ala_izq.LoadModel("assets/planeta_azul/modelos/pidgey_ala_izq.obj");
	ala_der.LoadModel("assets/planeta_azul/modelos/pidgey_ala_der.obj");
}

void Pidgeys::Actualizar(float dt)
{
	tiempo += dt;
	angulo_presa += velocidad_presa * dt;

	// Si se la comieron, despues de unos segundos aparece otra
	if (!presa_viva)
	{
		tiempo_comida += dt;
		if (tiempo_comida > tiempo_reaparecer)
			presa_viva = true;
	}
}

glm::vec3 Pidgeys::PosicionPresa() const
{
	return glm::vec3(centro_presa.x + radio_presa * cosf(angulo_presa),
	                 altura_presa,
	                 centro_presa.y + radio_presa * sinf(angulo_presa));
}

void Pidgeys::ComerPresa()
{
	presa_viva = false;
	tiempo_comida = 0.0f;
}

void Pidgeys::DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala_ave)
{
	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, pos);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));     // hacia donde vuela
	model = glm::rotate(model, alabeo, glm::vec3(0.0f, 0.0f, 1.0f));   // se inclina en la vuelta
	model = glm::scale(model, glm::vec3(escala_ave));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Ala izquierda
	model = modelaux;
	model = glm::translate(model, hombro_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	ala_izq.RenderModel();

	//Ala derecha
	model = modelaux;
	model = glm::translate(model, hombro_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	ala_der.RenderModel();
}

void Pidgeys::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Parvada
	for (int i = 0; i < num_aves; i++)
	{
		const Vuelo& v = vuelos[i];
		float t = tiempo * v.velocidad + v.fase;

		float x = v.cx + v.radio * cosf(t);
		float z = v.cz + v.radio * sinf(t);
		float aleteo = glm::radians(40.0f) * sinf(tiempo * 9.0f + i);
		float y = v.altura + 0.3f * sinf(tiempo * 9.0f + i);              // sube un poco con cada aleteo

		// Direccion de avance = derivada del circulo 
		float s = (v.velocidad >= 0.0f) ? 1.0f : -1.0f;
		float giro = atan2f(-sinf(t) * s, cosf(t) * s);
		float alabeo = glm::radians(15.0f) * s;                             // se inclina hacia adentro

		DibujarAve(ctx, glm::vec3(x, y, z), giro, alabeo, aleteo, v.escala);
	}

	//La presa del Reaper
	if (presa_viva)
	{
		glm::vec3 p = PosicionPresa();
		p.y += 0.3f * sinf(tiempo * 10.0f);
		float giro = atan2f(-sinf(angulo_presa), cosf(angulo_presa));
		float aleteo = glm::radians(40.0f) * sinf(tiempo * 10.0f);
		DibujarAve(ctx, p, giro, glm::radians(15.0f), aleteo, 1.3f);
	}
}
//>>>>>>>>>>>>>>>>>>>>>> Reaper >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones respecto a la cabeza
static const glm::vec3 piv_boca      ( 0.00f, -0.18f,  -0.68f);
static const glm::vec3 piv_tent_sup_izq( 0.89f,  0.15f,  -0.39f);
static const glm::vec3 piv_tent_sup_der(-0.89f,  0.13f,  -0.50f);
static const glm::vec3 piv_tent_inf_izq( 0.90f, -0.46f,  -0.35f);
static const glm::vec3 piv_tent_inf_der(-0.91f, -0.41f,  -0.46f);
static const glm::vec3 piv_aleta_izq  ( 0.43f, -0.87f,  -2.10f);
static const glm::vec3 piv_aleta_der  (-0.42f, -0.84f,  -2.20f);
static const glm::vec3 piv_cola      (-0.05f,  0.48f, -17.01f);

static const float y_base = -10.0f;          // profundidad cuando nada
static const float duracion_salto = 3.5f;    // segundos
static const float escala_reaper = 1.0f;           // el modelo ya mide

void Reaper::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/reaper_cuerpo.obj");
	boca.LoadModel("assets/planeta_azul/modelos/reaper_boca.obj");
	tent_sup_izq.LoadModel("assets/planeta_azul/modelos/reaper_tent_sup_izq.obj");
	tent_sup_der.LoadModel("assets/planeta_azul/modelos/reaper_tent_sup_der.obj");
	tent_inf_izq.LoadModel("assets/planeta_azul/modelos/reaper_tent_inf_izq.obj");
	tent_inf_der.LoadModel("assets/planeta_azul/modelos/reaper_tent_inf_der.obj");
	aleta_izq.LoadModel("assets/planeta_azul/modelos/reaper_aleta_izq.obj");
	aleta_der.LoadModel("assets/planeta_azul/modelos/reaper_aleta_der.obj");
	cola.LoadModel("assets/planeta_azul/modelos/reaper_cola.obj");
}

void Reaper::Actualizar(float dt, Window& ventana, Pidgeys& pidgeys)
{
	tiempo += dt;

	// Tecla U: salta (solo si no esta saltando y hay presa volando)
	if (ventana.TeclaPresionada(GLFW_KEY_C) && !saltando && pidgeys.PresaViva())
	{
		saltando = true;
		s = 0.0f;
		ya_mordio = false;
	}

	// Siempre nada justo debajo de la presa (mismo circulo, mismo angulo)
	float a = pidgeys.AnguloPresa();
	posicion.x = pidgeys.centro_presa.x + pidgeys.radio_presa * cosf(a);
	posicion.z = pidgeys.centro_presa.y + pidgeys.radio_presa * sinf(a);
	giro = atan2f(-sinf(a), cosf(a));                 // tangente del circulo

	float vel_horizontal = pidgeys.velocidad_presa * pidgeys.radio_presa;   // v = w * r
	float vel_vertical = 0.0f;

	if (saltando)
	{
		s += dt / duracion_salto;
		float H = pidgeys.altura_presa - y_base;        // cuanto tiene que subir

		// Parabola: sube, llega a la presa en s = 0.5 y baja
		posicion.y = y_base + H * 4.0f * s * (1.0f - s);
		vel_vertical = H * 4.0f * (1.0f - 2.0f * s) / duracion_salto;   // derivada de la parabola

		// Abre la boca mientras sube y la cierra de golpe al llegar arriba
		abre_boca = (s < 0.5f) ? sinf(s * 3.142f) : 0.0f;

		if (s >= 0.5f && !ya_mordio)
		{
			pidgeys.ComerPresa();
			ya_mordio = true;
		}
		if (s >= 1.0f)
		{
			saltando = false;
			posicion.y = y_base;
		}
	}
	else
	{
		posicion.y = y_base + 0.6f * sinf(tiempo * 0.8f);
		abre_boca = 0.15f + 0.15f * sinf(tiempo * 1.5f);   // la abre poquito mientras nada
	}

	// Inclinacion del cuerpo: angulo de la velocidad (vertical contra horizontal).
	// En el salto se usa directo al caer al agua regresa poco a poco a horizontal.
	float inclinacion_objetivo = atan2f(vel_vertical, vel_horizontal);
	if (saltando)
		inclinacion = inclinacion_objetivo;
	else
		inclinacion += (inclinacion_objetivo - inclinacion) * glm::min(1.0f, 3.0f * dt);
}

void Reaper::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float coleteo = glm::radians(12.0f) * sinf(tiempo * 2.0f);
	float aleteo = glm::radians(15.0f) * sinf(tiempo * 2.5f);
	float mandibula = glm::radians(30.0f) * abre_boca;
	// Los tentaculos se mueven solos y se abren mucho al atacar
	float tentaculos = glm::radians(8.0f) * sinf(tiempo * 3.0f) + glm::radians(35.0f) * abre_boca;

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -inclinacion, glm::vec3(1.0f, 0.0f, 0.0f));   // nariz arriba/abajo
	model = glm::scale(model, glm::vec3(escala_reaper));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Boca
	model = modelaux;
	model = glm::translate(model, piv_boca);
	model = glm::rotate(model, mandibula, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca.RenderModel();

	//Tentaculos de la cara
	model = modelaux;
	model = glm::translate(model, piv_tent_sup_izq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_sup_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_sup_der);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_sup_der.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_inf_izq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_inf_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_inf_der);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_inf_der.RenderModel();

	//Aletas
	model = modelaux;
	model = glm::translate(model, piv_aleta_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_der.RenderModel();

	//Cola
	model = modelaux;
	model = glm::translate(model, piv_cola);
	model = glm::rotate(model, coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cola.RenderModel();
}
//>>>>>>>>>>>>>>>>>>>>>> EscenaPlanetaAzul >>>>>>>>>>>>>>>>>>>>>>

void EscenaPlanetaAzul::Inicializar()
{
	//Skybox de dia y de noche
	// Se cargan sin voltear
	stbi_set_flip_vertically_on_load(false);
	CargarSkybox("assets/planeta_azul/texturas/skybox/noche/");
	skybox_noche = skybox;
	CargarSkybox("assets/planeta_azul/texturas/skybox/dia/");
	skybox_dia = skybox;

	//Camara: empieza en la orilla mirando hacia +Z
	camara = Camera(glm::vec3(0.0f, 10.0f, -60.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -8.0f, 20.0f, 0.3f);

	//Suelo y lago
	terreno.Crear();
    pez_abisal.Crear(terreno);
    arboles_sumergidos.Crear(terreno);
	glm::vec2 tamano_lago = terreno.radio_lago * 2.2f;   // un poco mas grande que el lago
	agua.Crear(terreno.centro_lago, tamano_lago, nivel_agua, "assets/planeta_azul/texturas/agua.jpg");

	vegetacion.Crear(terreno);
	aldea.Crear(terreno);
    CargarLamparas();
	peces.Crear();
	necromorfos.Crear(terreno, aldea);
	stalker.Crear();
	pidgeys.Crear();
	reaper.Crear();

	luz_sol = ciclo.LuzDireccional();

}

void EscenaPlanetaAzul::Actualizar(float dt, Window& ventana)
{
    if (ventana.TeclaPresionada(GLFW_KEY_Y)) luz_lago_encendida = !luz_lago_encendida;
	tiempo += dt;

	// Ciclo de dia y noche actualiza la luz y escoge el skybox
	ciclo.Actualizar(dt);
	luz_sol = ciclo.LuzDireccional();
	skybox = ciclo.EsDeNoche() ? skybox_noche : skybox_dia;

	// Aldea: fogata manual y antorchas que se prenden de noche
	aldea.Actualizar(tiempo, ciclo.EsDeNoche(), ventana);
    pez_abisal.Actualizar(dt, ventana);
    arboles_sumergidos.Actualizar(dt,ciclo.EsDeNoche(),ventana);

	// Peces nadando
	peces.Actualizar(dt);

	// Necromorfos caminan, y bailan mientras la fogata este prendida
	necromorfos.Actualizar(dt, aldea.fogata_encendida);

	// Avatar nadando, aves volando, y el Reaper (tecla C: salta por un Pidgey)
	stalker.Actualizar(dt);
	pidgeys.Actualizar(dt);
	reaper.Actualizar(dt, ventana, pidgeys);

}

void EscenaPlanetaAzul::Dibujar(const ContextoRender& ctx)
{
	// 1) Todo lo opaco primero
	terreno.Dibujar(ctx);
	aldea.Dibujar(ctx);
    arboles_sumergidos.Dibujar(ctx);
    pez_abisal.Dibujar(ctx);
    DibujarLamparas(ctx);
	peces.Dibujar(ctx);          // bajo el agua: se dibujan antes que el agua
	stalker.Dibujar(ctx);
	reaper.Dibujar(ctx);
	pidgeys.Dibujar(ctx);


    pez_abisal.DibujarHalo(ctx);

	// 2) Plantas y rocas
	vegetacion.Dibujar(ctx, &luz_sol, luces_visibles, num_luces_visibles);
	necromorfos.Dibujar(ctx, &luz_sol, luces_visibles, num_luces_visibles);   // shader del esqueleto

	// 3) El agua es transparente
	agua.Dibujar(ctx,tiempo,ciclo.Luz(),
                 {terreno.centro_lago.x,nivel_agua-2,terreno.centro_lago.y},luz_lago_encendida);

	// 4) El fuego al final de todo
	aldea.DibujarLlamas(ctx, tiempo);
}

bool EscenaPlanetaAzul::ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo,
                                 float& distancia, float& altura) const
{
    if (modo == 3)
    {
        objetivo = stalker.posicion + glm::vec3(0,.5f,0);
        rumbo = glm::vec3(sinf(stalker.giro), 0, cosf(stalker.giro));
        distancia = 12.0f;
        altura = -2.0f;
        return true;
    }
    if (modo != 4) return false;
    objetivo = necromorfos.PosicionPatrulla() + glm::vec3(0,2,0);
    rumbo = necromorfos.RumboPatrulla();
    distancia = 12.0f;
    altura = 6.5f;
    return true;
}

//>>>>>>>>>>>>>>>>>>>>>> Luminarias de suelo y tejados >>>>>>>>>>>>>>>>>>>>>>
namespace
{
const glm::vec2 lampara_entrada(-65,-40);
const glm::vec2 lampara_sendero(-51,-31);
const glm::vec2 lampara_orilla(-27,4);
const glm::vec2 lampara_puente(19,6);
const glm::vec2 lampara_pinos(87,66);
const glm::vec2 lampara_playa(27,132);
const glm::vec2 luces_sendero[] = {lampara_entrada,lampara_sendero,lampara_orilla,
                                  lampara_puente,lampara_pinos,lampara_playa};
}

void EscenaPlanetaAzul::CargarLamparas()
{
    lampara_suelo.LoadModel("assets/planeta_azul/modelos/lampara_suelo.obj");
    halo_suelo = Texture("assets/planeta_naranja/texturas/halo_faro.png");
    halo_suelo.LoadTextureA();
    std::vector<GLfloat> vertices;
    std::vector<unsigned> indices;
    for (const auto& p : luces_sendero)
    {
        const unsigned base = unsigned(vertices.size()/8);
        for (int z=0;z<=8;++z)
            for (int x=0;x<=8;++x)
            {
                const float u=x/8.0f,v=z/8.0f;
                const float px=p.x+(u-.5f)*16.0f,pz=p.y+(v-.5f)*16.0f;
                vertices.insert(vertices.end(),{px,terreno.Altura(px,pz)+.035f,pz,u,v,0,-1,0});
                if (x<8 && z<8)
                {
                    const unsigned j=base+z*9+x;
                    indices.insert(indices.end(),{j,j+1,j+9,j+9,j+1,j+10});
                }
            }
    }
    malla_halo_suelo.CreateMeshModel(vertices.data(),indices.data(),unsigned(vertices.size()),unsigned(indices.size()));
    GLfloat vertices_tejado[] = {-.5f,-.5f,0,0,0,0,0,-1, .5f,-.5f,0,1,0,0,0,-1,
                                 .5f,.5f,0,1,1,0,0,-1, -.5f,.5f,0,0,1,0,0,-1};
    unsigned indices_tejado[] = {0,1,2,0,2,3};
    malla_halo_tejado.CreateMeshModel(vertices_tejado,indices_tejado,32,6);
}

void EscenaPlanetaAzul::DibujarLamparas(const ContextoRender& ctx)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    const GLint modo = glGetUniformLocation(programa,"modo_efecto");
    const GLint opacidad = glGetUniformLocation(programa,"opacidad_particula");
    const GLint noche = glGetUniformLocation(programa,"factor_noche");
    auto enviar = [&](const glm::mat4& m) {
        glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(m));
    };
    glUniform1f(noche,ciclo.EsDeNoche() ? 1.0f : 0.0f);
    for (const auto& p : luces_sendero)
    {
        enviar(glm::translate(glm::mat4(1),glm::vec3(p.x,terreno.Altura(p.x,p.y),p.y)));
        lampara_suelo.RenderModel(true,ctx.u_specular_intensity,ctx.u_shininess);
    }
    if (!ciclo.EsDeNoche()) return;
    glUniform1i(modo,4);
    glUniform1f(opacidad,.62f);
    glUniform3f(ctx.u_color,1.2f,1.03f,.72f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2,-4);
    halo_suelo.UseTexture();
    enviar(glm::mat4(1));
    malla_halo_suelo.RenderMeshModel();
    glDisable(GL_POLYGON_OFFSET_FILL);
    const float tejados[] = {8.4f,8.05f,8.4f,8.33f,8.4f,8.05f,8.4f};
    for (int i=0;i<aldea.NumChozas();++i)
    {
        const glm::vec2 p = aldea.PosicionChoza(i);
        glm::mat4 m = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
        m[3] = glm::vec4(p.x,terreno.Altura(p.x,p.y)+tejados[i],p.y,1);
        enviar(glm::scale(m,glm::vec3(2.8f)));
        malla_halo_tejado.RenderMeshModel();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1i(modo,0);
    glUniform1f(opacidad,1);
    glUniform1f(noche,0);
    glUniform3f(ctx.u_color,1,1,1);
}
}

float EscenaPlanetaAzul::SueloPatrulla(float x, float z) const
{
    return terreno.Altura(x, z);
}

PointLight EscenaPlanetaAzul::LuzLocal() const
{
    return PointLight(.16f,.65f,1,0,luz_lago_encendida ? 4.0f : 0.0f,terreno.centro_lago.x,nivel_agua-2,
                      terreno.centro_lago.y,1,.025f,.003f);
}

void EscenaPlanetaAzul::PrepararLuces(const glm::vec3& ojo)
{
    struct Candidata { const PointLight* luz; float distancia2; };
    Candidata candidatas[max_point_lights + ArbolesLuminosos::num_arboles];
    unsigned int total = 0;
    auto agregar = [&](const PointLight* luces, const glm::vec3* posiciones, unsigned int cantidad)
    {
        for (unsigned int i = 0; i < cantidad; ++i)
        {
            const glm::vec3 delta = posiciones[i] - ojo;
            candidatas[total++] = {&luces[i], glm::dot(delta,delta)};
        }
    };
    agregar(aldea.luces,aldea.pos_luces,aldea.num_luces);
    agregar(arboles_sumergidos.luces,arboles_sumergidos.pos_luces,arboles_sumergidos.num_luces);
    num_luces_visibles = std::min(total, unsigned(max_point_lights - 1));
    std::partial_sort(candidatas,candidatas+num_luces_visibles,candidatas+total,
        [](const Candidata& a, const Candidata& b) { return a.distancia2 < b.distancia2; });
    for (unsigned int i = 0; i < num_luces_visibles; ++i)
        luces_visibles[i] = *candidatas[i].luz;
}
