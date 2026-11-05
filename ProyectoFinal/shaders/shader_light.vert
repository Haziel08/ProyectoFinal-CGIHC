#version 330
//>>>>>>>>>>>>>>>>>>>>>> Entradas y uniforms >>>>>>>>>>>>>>>>>>>>>>>>>

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec4 v_col;
out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;
out vec4 v_color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;


//>>>>>>>>>>>>>>>>>>>>>> Funcion principal del shader >>>>>>>>>>>>>>>>
void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	v_col = vec4(0.0, 1.0, 0.0, 1.0f);
	v_color=vec4(color,1.0f);
	tex_coord = tex;
	//para tomar las Transformaciones geométricas y la transpuesta es para que las escalas se inviertan en caso de escalas no uniformes en las normales
	normal = mat3(transpose(inverse(model))) * norm;
	
	frag_pos = (model * vec4(pos, 1.0)).xyz; //para obtener un vec3 sólo posición con respecto al origen en orden xyz: Swizzling 
}
