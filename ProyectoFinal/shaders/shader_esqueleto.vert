#version 330
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
	//para tomar las Transformaciones geomÃƒÆ’Ã‚Â©tricas y la transpuesta es para que las escalas se inviertan en caso de escalas no uniformes en las normales
	normal = mat3(transpose(inverse(model))) * normal_animada;
	
	frag_pos = (model * pos_animada).xyz; //para obtener un vec3 sÃƒÆ’Ã‚Â³lo posiciÃƒÆ’Ã‚Â³n con respecto al origen en orden xyz: Swizzling 
}
