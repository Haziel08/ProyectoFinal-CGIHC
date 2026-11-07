#version 330
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
