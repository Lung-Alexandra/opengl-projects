#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D backgroundTexture;

void main(void){

    vec4 tex_color = texture(backgroundTexture, tex_Coord);
    out_Color = mix(tex_color,vec4(0.0,0.0,0.0,1.0),0.2);
    
}
 