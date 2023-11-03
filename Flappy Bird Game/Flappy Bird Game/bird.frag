#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D birdTexture;
uniform float gametime;

void main(void){

    vec4 tex_color = texture(birdTexture, tex_Coord);
    if (tex_color.a < 0.1)
        discard;
    out_Color = tex_color + 0.5 * vec4(sin(gametime*3 + 0.15), sin(gametime*3), sin(gametime*3 + 1.5), 0);
    
}
 