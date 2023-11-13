#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D backgroundTexture;
uniform float gametime;

void main(void){

    vec4 tex_color = texture(backgroundTexture, tex_Coord);
    // Calculate the rotated position of the sun based on game time
    float rotationSpeed = 0.5;
    float rotationAngle = rotationSpeed * gametime;

    float yOffset = sin(rotationAngle)*0.7;
    float xOffset = cos(rotationAngle)*0.75;

    // Draw the sun
    vec2 sunCenter = vec2(0.5 + xOffset, 1 - yOffset);
    float sunRadius = 0.1;
    vec4 sunColor = vec4(1.0, 1.0, 0.0, 1.0);

    //Draw moon
    vec2 moonCenter= vec2(0.5 - xOffset, 1 + yOffset);
    float moonRadius = 0.08;
    vec4 moonColor = vec4(0.8, 0.8, 0.8, 1.0);

    vec2 shiftedMoonCenter = vec2(moonCenter.x - 0.05, moonCenter.y);

    float distanceToSun = length(tex_Coord - sunCenter);
    float distanceToMoon = length(tex_Coord - moonCenter);
    float distanceToShiftedMoon = length(tex_Coord - shiftedMoonCenter);

    float brightness = 1;
    if (sunCenter.y > yOffset){
        brightness = smoothstep(0, sunCenter.y - yOffset, 0.4);
        tex_color *= brightness;
    }
    if (distanceToSun < sunRadius) {
        out_Color = vec4(mix(sunColor, tex_color, smoothstep(0.05, sunRadius, distanceToSun)));
    }
    else if (distanceToMoon < moonRadius && distanceToShiftedMoon > moonRadius){
        out_Color = vec4(mix(tex_color,moonColor, smoothstep(0.0, moonRadius,distanceToMoon)));
    }
    else {
        out_Color = tex_color;
    }
}