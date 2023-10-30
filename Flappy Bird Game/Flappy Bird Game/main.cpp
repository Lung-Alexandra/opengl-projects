﻿//	Biblioteci

#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"			//	Biblioteca pentru texturare;
#include <iostream>
#include <vector>

//  Identificatorii obiectelor de tip OpenGL;
GLuint
VaoId,
VboId,
EboId,
BirdProgramId,
PipeProgramId,
BackgroundProgramId,
myMatrixLocation,
viewLocation,
projLocation,
matrRotlLocation,
codColLocation;
GLuint
textures[3];
//	Dimensiunile ferestrei de afisare;
GLfloat
winWidth = 1200, winHeight = 900;
//	Variabile catre matricile de transformare;
glm::mat4
myMatrix, resizeMatrix;


//	Variabila ce determina schimbarea culorii pixelilor in shader;
int codCol;

//	Elemente pentru matricea de proiectie;
float  xMin = -500.f, xMax = 500, yMin = -500, yMax = 500;

float PI = 3.141592;

float
deltax = xMax - xMin, deltay = yMax - yMin, // lungimile laturilor dreptunghiului decupat
xcenter = (xMin + xMax) * 0.5, ycenter = (yMin + yMax) * 0.5; // centrul dreptunghiului decupat

float pipe_xmin = -0.25f, pipe_xmax = 0.25f,
pipe_ymin = 0.0f, pipe_ymax = 1.f;

float pipeVelocity = 200, gametime = 0;

// Global variables
float birdY = 0.f; // Initial bird position
float birdVelocity = 0;

float bird_xmin = -0.5f, bird_xmax = 0.5f,
bird_ymin = -0.5f, bird_ymax = 0.5f;
float rotationAngle = 0;


const float gravity = 0.0005f;
const float jump_strength = 0.3f;
float score = 0; 
int maxScore = 0;

struct Pipe {
	float x;
	float y;
	bool visible;
};

std::vector<Pipe> pipes = { {0,0,true},{300.f,-10,true},{600.f,20,true},{900.f,10,true}, {1200.f,200,true} };

void game_over(void) {
	if (birdY < yMin) {
		if (score > maxScore) {
			maxScore = score; // Actualizați scorul maxim dacă scorul curent este mai mare
		}
		// Bird hit the ground
		std::cout << "Game Over. Your score: " << score << std::endl;
		score = 0; // Resetați scorul curent
		exit(0);
	}
}

 void MoveDown(void) {
	birdVelocity -= gravity;
	birdY += birdVelocity;
	if (birdVelocity < -1)
		birdVelocity = -1;
	/*
	     v -> (-1,0.5) => v+1 -> (0,1.5) => (v+1)/1.5 -> (0,pi) 
	*/

	rotationAngle = ((birdVelocity + 1) / 1.5f) * PI - PI/2;
	game_over();
	glutPostRedisplay();
}


void MoveUp(void) {
	if (birdVelocity > 0.5)
		birdVelocity = 0.5;
	else
		birdVelocity += jump_strength;
}


void ProcessNormalKey(unsigned char key, int x, int y) {
	if (key == ' ') {
		MoveUp();
	}
}


//	Functia de incarcare a texturilor in program;
void LoadTexture(const char* photoPath, unsigned int textureId)
{
	// first parameter represents how many textures we want to generate
	glGenTextures(1, &textures[textureId]);
	glBindTexture(GL_TEXTURE_2D, textures[textureId]);
	//	Desfasurarea imaginii pe orizonatala/verticala in functie de parametrii de texturare;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height;
	unsigned char* image = SOIL_load_image(photoPath, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//  Crearea si compilarea obiectelor de tip shader;
//	Trebuie sa fie in acelasi director cu proiectul actual;
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei;
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
void CreateShaders(void)
{
	BirdProgramId = LoadShaders("bird.vert", "bird.frag");
	PipeProgramId = LoadShaders("pipe.vert", "pipe.frag");
	BackgroundProgramId = LoadShaders("background.vert", "background.frag");
}

//  Se initializeaza un Vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);
void CreateVBO(void)
{

	//	Atributele varfurilor -  COORDONATE, CULORI, COORDONATE DE TEXTURARE;
	GLfloat Vertices[] = {
		// bird's coordonate (actually a square)
		//	Coordonate;					              Culori;			Coordonate de texturare;
		 bird_xmin, bird_ymin , 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,		0.0f, 0.0f,	
		 bird_xmax, bird_ymin , 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 
		 bird_xmax, bird_ymax , 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,		1.0f, 1.0f,	
		 bird_xmin, bird_ymax , 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,		0.0f, 1.0f,  

		//pipe coordonate
		
		// Coordonate;								Culori;				Coordonate de texturare;
		pipe_xmin, pipe_ymin, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,  // Partea de jos a conductei
		pipe_xmax, pipe_ymin, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,  // Partea de jos a conductei
		pipe_xmax, pipe_ymax, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,  // Partea de sus a conductei
		pipe_xmin, pipe_ymax, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,  // Partea de sus a conductei

		//background
		xMin, yMin, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f, 
		xMax, yMin, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		xMax, yMax, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
		xMin, yMax, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,

	};

	//	Indicii care determina ordinea de parcurgere a varfurilor;
	GLuint Indices[] = {
		//indices for bird draw
			0,1,2,3,
		//indices for pipedraw
			4,5,6,7,
		//indices for background
			8,9,10,11,
	};

	//  Transmiterea datelor prin buffere;

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer pentru VARFURI - COORDONATE, CULORI si TEXTURARE;
	glGenBuffers(1, &VboId);													//  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId);										//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboId);														//  Generarea bufferului si indexarea acestuia catre variabila EboId;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);									//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	//	Se activeaza lucrul cu atribute;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	//  Se asociaza atributul (2 =  texturare) pentru shader;
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
}

//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(BirdProgramId);
	glDeleteProgram(PipeProgramId);
	glDeleteProgram(BackgroundProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori, Textura), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		//  Culoarea de fond a ecranului;
	CreateShaders();							//  Initilizarea shaderelor;
	
	//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateVBO();
	

	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	myMatrixLocation = glGetUniformLocation(BirdProgramId, "myMatrix");
	myMatrixLocation = glGetUniformLocation(PipeProgramId, "myMatrix");

	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
	

	//	Incarcarea texturii si legarea acesteia cu shaderul;
	glUseProgram(BirdProgramId);
	LoadTexture("bird.png",0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	

	glUseProgram(PipeProgramId);
	LoadTexture("pipe.png",1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	

	glUseProgram(BackgroundProgramId);
	LoadTexture("background.png", 2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);

}
void DrawBackground(void) {
	glUseProgram(BackgroundProgramId);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	
	myMatrix = resizeMatrix;

	//	Transmiterea variabilei uniforme pentru TEXTURARE spre shaderul de fragmente;
	glUniform1i(glGetUniformLocation(BackgroundProgramId, "backgroundTexture"), 0);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// Draw bird
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void*)(8 * sizeof(GLuint)));
}

void DrawBird(void) {
	glUseProgram(BirdProgramId);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	//	Matrici pentru transformari;
	glm::mat4 matrRot = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 matrScale = glm::scale(glm::mat4(1.f), glm::vec3(80, 80, 1.0));
	glm::mat4 matrTranslate = glm::translate(glm::mat4(1.f), glm::vec3(-300, birdY, 1.0));

	myMatrix = resizeMatrix * matrTranslate * matrScale * matrRot;

	//	Transmiterea variabilei uniforme pentru TEXTURARE spre shaderul de fragmente;
	glUniform1i(glGetUniformLocation(BirdProgramId, "birdTexture"), 0);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// Draw bird
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}

void DrawPipeDown(float x, float y) {
	glUseProgram(PipeProgramId);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	// matrici pentru pipe
	glm::mat4 matrScalePipe = glm::scale(glm::mat4(1.f), glm::vec3(200, 400, 1.0));
	glm::mat4 matrTranslatePipe = glm::translate(glm::mat4(1.f),
				glm::vec3(-gametime * pipeVelocity + x, yMin+y, 1.0));

	myMatrix = resizeMatrix * matrTranslatePipe * matrScalePipe ;

	//	Transmiterea variabilei uniforme pentru TEXTURARE spre shaderul de fragmente;
	glUniform1i(glGetUniformLocation(PipeProgramId, "pipeTexture"), 0);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// Draw bottom pipe
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));

}

void DrawPipeUp(float x, float y) {
	glUseProgram(PipeProgramId);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glUniform1i(glGetUniformLocation(PipeProgramId, "pipeTexture"), 0);

	// matrici pentru pipe
	glm::mat4 matrRotPipe = glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0.0, 0.0, 1.0));
	// scale to a negative value on x to flip horizontal the texture
	glm::mat4 matrScalePipe = glm::scale(glm::mat4(1.f), glm::vec3(-200, 400, 1.0));
	glm::mat4 matrTranslatePipe = glm::translate(glm::mat4(1.f),
				glm::vec3(-gametime*pipeVelocity+x, yMax+y, 1.0));

	myMatrix = resizeMatrix * matrTranslatePipe * matrScalePipe * matrRotPipe;

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	// Draw bottom pipe
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));
}

//  Functia de desenarea a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);			//  Se curata ecranul OpenGL pentru a fi desenat noul continut;
	gametime += 0.001f;
	
	DrawBackground();
	DrawBird();
	for (Pipe& pipe : pipes) {
		DrawPipeDown(pipe.x, pipe.y);
		DrawPipeUp(pipe.x, pipe.y);
	}
	
	glutSwapBuffers();	//	Inlocuieste imaginea deseneata in fereastra cu cea randata; 
	glFlush();			//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
}


//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);					//	Se folosesc 2 buffere (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Flappy Bird");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();							//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);		//  Desenarea scenei in fereastra;
	glutIdleFunc(RenderFunction);			//	Asigura rularea continua a randarii;

	glutIdleFunc(MoveDown);

	//	Functii ce proceseaza inputul de la tastatura utilizatorului;
	glutKeyboardFunc(ProcessNormalKey);

	glutCloseFunc(Cleanup);					//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}
