//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_03_Shader.frag |
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D myTexture;

//	Variabile pentru culori;
vec4 background = vec4(0.00, 0.74, 1.00, 1.0);
vec4 black = vec4(0.0,0.0,0.0,1.0);
void main(void)
  {
  //  out_Color=ex_Color;
  //  out_Color=mix(red,green,0.9);
  //  out_Color = mix(texture(myTexture, tex_Coord), ex_Color, 0.2);	//	Amestecarea texturii si a culorii;
   
   vec4 tex_color = texture(myTexture, tex_Coord);
  
   if(tex_color.a < 0.1)
	 discard;
   out_Color =  tex_color;	
   
  }
 