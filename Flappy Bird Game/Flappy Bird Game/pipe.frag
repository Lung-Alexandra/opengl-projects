#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D pipeTexture;

//	Variabile pentru culori;
vec4 background = vec4(0.00, 0.74, 1.00, 1.0);

void main(void){

    vec4 tex_color = texture(pipeTexture, tex_Coord);
    if (tex_color.a < 0.1)
        discard;
    out_Color = tex_color;
    
}
 