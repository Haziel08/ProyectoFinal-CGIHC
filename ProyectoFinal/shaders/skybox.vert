#version 330
//>>>>>>>>>>>>>>>>>>>>>> Entradas y uniforms >>>>>>>>>>>>>>>>>>>>>>>>>

layout (location = 0) in vec3 pos;
out vec3 tex_coords;

uniform mat4 projection;
uniform mat4 view;
//>>>>>>>>>>>>>>>>>>>>>> Funcion principal del shader >>>>>>>>>>>>>>>>
void main()
{
tex_coords= -pos;
vec4 posicion = projection * view * vec4(pos, 1.0);
gl_Position = posicion.xyww;
}

