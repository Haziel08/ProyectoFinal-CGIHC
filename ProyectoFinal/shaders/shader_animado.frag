#version 330
// Shader animado (agua, fuego...): textura que se mueve con el tiempo,
// transparencia y luz direccional.
//   Agua:  usarLuz = 1, transparencia 0.6, desvanecer 0
//   Fuego: usarLuz = 0, transparencia 1.0, desvanecer 1, textura subiendo
//          (en C++ se mezcla con glBlendFunc(GL_SRC_ALPHA, GL_ONE): lo negro no se ve)

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D textura;
uniform float tiempo;
uniform vec2  velocidadUV;     // cuanto se mueve la textura por segundo
uniform float repeticionUV;    // cuantas veces se repite la textura
uniform vec3  colorBase;       // color que multiplica a la textura
uniform float transparencia;   // 0 = invisible, 1 = opaco
uniform float desvanecer;      // 1 = se apaga hacia arriba y a los lados (forma de llama)

uniform int   usarLuz;         // 1 = recibe la luz direccional, 0 = no
uniform vec3  luzDireccion;    // hacia donde viaja la luz (igual que DirectionalLight)
uniform vec3  luzColor;
uniform float luzAmbiental;
uniform float luzDifusa;
uniform vec3  posCamara;
uniform float brillo;          // shininess del reflejo

void main()
{
	// La textura se recorre con el tiempo: parece que el agua fluye
	vec2 uv = TexCoord * repeticionUV + velocidadUV * tiempo;
	vec3 rgb = texture(textura, uv).rgb * colorBase;

	// Forma de llama: brillante abajo al centro, se apaga hacia arriba y hacia los lados
	float haciaArriba = 1.0 - TexCoord.y;
	float haciaLados = 1.0 - abs(TexCoord.x * 2.0 - 1.0);
	float forma = haciaArriba * haciaLados * 2.0;
	rgb = mix(rgb, rgb * forma, desvanecer);

	if (usarLuz == 1)
	{
		vec3 N = normalize(Normal);
		vec3 L = normalize(-luzDireccion);           // hacia la luz
		vec3 V = normalize(posCamara - FragPos);     // hacia la camara

		// Igual que en shader_light: ambiental + difusa + especular
		float difusa = max(dot(N, L), 0.0);
		float especular = pow(max(dot(V, reflect(-L, N)), 0.0), brillo);
		rgb = rgb * luzColor * (luzAmbiental + luzDifusa * difusa)
			+ luzColor * especular * luzDifusa;
	}

	color = vec4(rgb, transparencia);
}
