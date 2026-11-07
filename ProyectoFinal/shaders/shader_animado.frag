#version 330
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
