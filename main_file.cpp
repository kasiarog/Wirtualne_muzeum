#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "myCube.h"
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

float aspectRatio = 1;
float m = 1;
//int window_width = 1800, window_height = 1300;
int window_width = 1366, window_height = 768;

// liczenie czasu
float dt = 0.0f;
float prevFrame = 0.0f, currentFrame = 0.0f;

// deklarowanie wszystkich wektorów potrzebnych do liczenia położenia
glm::vec3 cameraPosition = glm::vec3(0, 1, 19.7f);
glm::vec3 lookatPosition = glm::vec3(0, 1, -5);
glm::vec3 upVector = glm::vec3(0, 1, 0);
glm::vec3 rightVector, leftVector, forwardVector;

// obsługa obracania się i myszki
bool firstMouse = true;
float yaw = -90.0f;	// gdyby yaw było 0 to wtedy patrzylibyśmy się w prawo
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// animacje
bool OPEN_DOOR = false;
float speed = PI / 2;
float angle = 0;
float doorspeed = 0;
float drzwi_angle;

// deklarowanie tekstur
GLuint tex_floor, tex_wall, tex_ceiling;
GLuint tex_plant, tex_plant2, tex_pot;
GLuint tex_monument, tex_wenus, tex_moai, tex_dyskobol, tex_stool, tex_wolf;
GLuint tex_frame, tex_frame2, tex_frame3;
GLuint tex_cat1, tex_cat2, tex_cat3, tex_cat4, tex_cat5, tex_kasia, tex_kasia2, tex_wojtek, tex_kira, tex_kira2, tex_kira3, tex_bunia, tex_bunia2;
GLuint tex_door, tex_Robot, tex_Robot2;
GLuint tex_kotrog, tex_wojtek2, tex_bricks, tex_kotrog2, tex_hermes1, tex_hermes2, tex_meme1, tex_meme2, tex_kira4;
GLuint tex_binia, tex_binia2, tex_kapelusz;

// klasa do wczytywania modeli
class Model {
public:
	std::vector<glm::vec4> verts;
	std::vector<glm::vec4> norms;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;

	void texModel(glm::mat4 P, glm::mat4 V, glm::mat4 M, GLuint texture) {

		spTextured->use(); //Aktywuj program cieniujący

		glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P)); //Załaduj do programu cieniującego macierz rzutowania
		glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V)); //Załaduj do programu cieniującego macierz widoku
		glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M)); //Załaduj do programu cieniującego macierz modelu
		//glUniform3f(spTextured->u("cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z); // coś popsułem 


		glEnableVertexAttribArray(spTextured->a("vertex"));
		glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, verts.data()); //Współrzędne wierzchołków

		glEnableVertexAttribArray(spTextured->a("texCoord"));
		glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, texCoords.data()); //Współrzędne teksturowania

		glEnableVertexAttribArray(spTextured->a("normal"));
		glVertexAttribPointer(spTextured->a("normal"), 2, GL_FLOAT, false, 0, norms.data()); //Współrzędne teksturowania

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(spTextured->u("tex"), 0);

		//glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

		glDisableVertexAttribArray(spTextured->a("vertex"));
		glDisableVertexAttribArray(spTextured->a("color"));
		glDisableVertexAttribArray(spTextured->a("normal"));
	}


	void loadModel(std::string plik) {
		using namespace std;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(plik,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		//cout << importer.GetErrorString() << endl;

		aiMesh* mesh = scene->mMeshes[0];

		for (int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D vertex = mesh->mVertices[i]; //aiVector3D podobny do glm::vec3
			verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

			aiVector3D normal = mesh->mNormals[i]; //Wektory znormalizowane
			norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

			//0 to numer zestawu współrzędnych teksturowania
			aiVector3D texCoord = mesh->mTextureCoords[0][i];
			texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
		}

		//dla każdego wielokąta składowego
		for (int i = 0; i < mesh->mNumFaces; i++) {
			aiFace& face = mesh->mFaces[i]; //face to jeden z wielokątów siatki
			//dla każdego indeksu->wierzchołka tworzącego wielokąt
			//dla aiProcess_Triangulate to zawsze będzie 3
			for (int j = 0; j < face.mNumIndices; j++) {
				//cout << face.mIndices[j] << " ";
				indices.push_back(face.mIndices[j]);
			}
		}

		// wczytywanie odpowiednio tekstur
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
			aiString str;//nazwa pliku

			material->GetTexture(aiTextureType_DIFFUSE, i, &str);

			// cout << str.C_Str() << endl;
		}
	}

};

Model plant, plant2, pot;
Model monument, wenus, moai, dyskobol, stool, wolf;
Model frame, frame2, frame3, drzwi;
Model robot;


//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

GLFWwindowsizefun windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return 0;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod) {

	float cameraSpeed = (4.5f * dt);	// normalnie było 2.5

	// przemieszczanie się
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPosition -= glm::normalize(glm::cross(lookatPosition, upVector)) * cameraSpeed;
		cameraPosition.y = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPosition += glm::normalize(glm::cross(lookatPosition, upVector)) * cameraSpeed;
		cameraPosition.y = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPosition += cameraSpeed * 1.0f * lookatPosition;
		cameraPosition.y = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPosition -= cameraSpeed * 1.0f * lookatPosition;
		cameraPosition.y = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		doorspeed = 2 * PI;
	}


	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
			glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
			exit(EXIT_SUCCESS);
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_E) {
			OPEN_DOOR = not(OPEN_DOOR);
		}
		if (key == GLFW_KEY_R) {
			doorspeed = 0;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// to jest konieczne do tego, żeby przy obrocie o 90 stopni nie zmianiły się kierunki
	if (pitch > 89.0f)	pitch = 89.0f;
	if (pitch < -89.0f)	pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookatPosition = glm::normalize(front);
}


GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetWindowSizeCallback(window, windowResizeCallback(window, window_width, window_height));
	glfwSetKeyCallback(window, key_callback);

	//wczytywanie tekstur
	tex_floor = readTexture("textures/tile2a.png");
	tex_wall = readTexture("textures/wall2.png");
	tex_ceiling = readTexture("textures/ceiling.png");

	tex_plant = readTexture("models/plant/texture.png");
	tex_plant2 = readTexture("models/pot/leaf_texture.png");
	tex_pot = readTexture("models/pot/pot_texture.png");

	tex_monument = readTexture("models/budda/textura.png");
	tex_wenus = readTexture("models/wenus/textura.png");
	tex_moai = readTexture("models/moai/textura.png");
	tex_dyskobol = readTexture("models/dyskobol/statue.png");
	tex_stool = readTexture("models/stool/black.png");

	tex_frame = readTexture("models/frame1/T_Frame_Ornate_01_Albedo.png");
	tex_frame2 = readTexture("models/frame2/Victorian Paintings_BaseColor_Utility - sRGB - Texture.png");
	tex_frame3 = readTexture("models/frame3/Textures/OldPotraitPaintingWitch_01_Base_Color.png");
	tex_door = readTexture("models/drzwi2/textures/Big_door_COL.png");

	tex_cat1 = readTexture("textures/foty/royal_cat.png");
	tex_cat2 = readTexture("textures/foty/royal_cat2.png");
	tex_cat3 = readTexture("textures/foty/royal_cat3.png");
	tex_cat4 = readTexture("textures/foty/royal_cat4.png");
	tex_cat5 = readTexture("textures/foty/royal_cat5.png");
	tex_kasia = readTexture("textures/foty/kasia.png");
	tex_kasia2 = readTexture("textures/foty/kasia2.png");
	tex_wojtek = readTexture("textures/foty/wojtek.png");
	tex_kira = readTexture("textures/foty/kira.png");
	tex_kira2 = readTexture("textures/foty/kira2.png");
	tex_kira3 = readTexture("textures/foty/kira3.png");
	tex_bunia = readTexture("textures/foty/bunia.png");
	tex_bunia2 = readTexture("textures/foty/bunia2.png");
	tex_Robot = readTexture("models/Robot2/textures/Robot.png");
	tex_Robot2 = readTexture("models/Robot2/textures/Enemy.png");
	tex_kotrog = readTexture("textures/foty/kotrog.png");
	tex_wojtek2 = readTexture("textures/foty/wojtek2.png");
	tex_bricks = readTexture("textures/bricks2.png");
	tex_kotrog2 = readTexture("textures/foty/Kittencorn.png");
	tex_meme1 = readTexture("textures/foty/meme1.png");
	tex_meme2 = readTexture("textures/foty/meme2.png");
	tex_hermes1 = readTexture("textures/foty/hermes1.png");
	tex_hermes2 = readTexture("textures/foty/hermes2.png");
	tex_kira4 = readTexture("textures/foty/kira4.png");
	tex_binia = readTexture("textures/foty/Binia.png");
	tex_binia2 = readTexture("textures/foty/Binia2.png");
	tex_kapelusz = readTexture("textures/foty/kot_kapelusz.png");


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_floor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glBindTexture(GL_TEXTURE_2D, tex_ceiling);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	//wczytywanie modeli
	plant.loadModel(std::string("models/plant/indoor plant_02_+2.fbx"));
	plant2.loadModel(std::string("models/pot/Low-Poly Plant_.fbx"));
	pot.loadModel(std::string("models/pot/Low-Poly Plant_.ply"));
	monument.loadModel(std::string("models/budda/12348_statue_v1_l3.obj"));
	wenus.loadModel(std::string("models/wenus/12328_Statue_v1_L2.obj"));
	moai.loadModel(std::string("models/moai/12329_Statue_v1_l3.obj"));
	dyskobol.loadModel(std::string("models/dyskobol/12338_Statue_v1_L3.obj"));
	stool.loadModel(std::string("models/stool/stool.fbx"));
	wolf.loadModel(std::string("models/wolf/Wolf_One_obj.obj"));
	frame.loadModel(std::string("models/frame1/SM_Frame_Ornate_01.obj"));  //jest i .obj i .fbx
	frame2.loadModel(std::string("models/frame2/Victorian Painting.obj"));
	frame3.loadModel(std::string("models/frame3/OldPortraitWitch_01.obj"));
	drzwi.loadModel(std::string("models/drzwi2/uploads_files_138410_Old_Basement_Door_.obj"));
	robot.loadModel(std::string("models/Robot2/uploads_files_4201781_Robot.obj"));
}

//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	glDeleteTextures(1, &tex_floor);
	glDeleteTextures(1, &tex_wall);
	glDeleteTextures(1, &tex_ceiling);
	glDeleteTextures(1, &tex_plant);
	glDeleteTextures(1, &tex_plant2);
	glDeleteTextures(1, &tex_pot);
	glDeleteTextures(1, &tex_monument);
	glDeleteTextures(1, &tex_wenus);
	glDeleteTextures(1, &tex_moai);
	glDeleteTextures(1, &tex_dyskobol);
	glDeleteTextures(1, &tex_stool);
	glDeleteTextures(1, &tex_wolf);
	glDeleteTextures(1, &tex_frame);
	glDeleteTextures(1, &tex_frame2);
	glDeleteTextures(1, &tex_frame3);
	glDeleteTextures(1, &tex_cat1);
	glDeleteTextures(1, &tex_cat2);
	glDeleteTextures(1, &tex_cat3);
	glDeleteTextures(1, &tex_cat4);
	glDeleteTextures(1, &tex_cat5);
	glDeleteTextures(1, &tex_kasia);
	glDeleteTextures(1, &tex_kasia2);
	glDeleteTextures(1, &tex_wojtek);
	glDeleteTextures(1, &tex_kira);
	glDeleteTextures(1, &tex_kira2);
	glDeleteTextures(1, &tex_kira3);
	glDeleteTextures(1, &tex_bunia);
	glDeleteTextures(1, &tex_bunia2);
	glDeleteTextures(1, &tex_Robot);
	glDeleteTextures(1, &tex_Robot2);
	glDeleteTextures(1, &tex_kotrog);
	glDeleteTextures(1, &tex_wojtek2);
	glDeleteTextures(1, &tex_bricks);
	glDeleteTextures(1, &tex_kotrog2);
	glDeleteTextures(1, &tex_meme1);
	glDeleteTextures(1, &tex_meme2);
	glDeleteTextures(1, &tex_hermes1);
	glDeleteTextures(1, &tex_hermes2);
	glDeleteTextures(1, &tex_kira4);
	glDeleteTextures(1, &tex_binia);
	glDeleteTextures(1, &tex_binia2);
	glDeleteTextures(1, &tex_kapelusz);
}


void texKostka(glm::mat4 P, glm::mat4 V, glm::mat4 M, int m, GLuint texture) {
	//procedura rysowania kostki - m determinuje liczbę kafelków

	float myCubeTexCoords[] = {
		m, 0.0f, 0.0f, m, 0.0f, 0.0f,
		m, 0.0f, m,    m, 0.0f, m,

		m, 0.0f, 0.0f, m, 0.0f, 0.0f,
		m, 0.0f, m,    m, 0.0f, m,

		m, 0.0f, 0.0f, m, 0.0f, 0.0f,
		m, 0.0f, m,    m, 0.0f, m,

		m, 0.0f,   0.0f, m, 0.0f, 0.0f,
		m, 0.0f,   m,    m, 0.0f, m,

		m, 0.0f, 0.0f, m, 0.0f, 0.0f,
		m, 0.0f, m,    m, 0.0f, m,

		m, 0.0f, 0.0f, m, 0.0f, 0.0f,
		m, 0.0f, m,    m, 0.0f, m,
	};

	spTextured->use(); //Aktywuj program cieniujący

	glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P)); //Załaduj do programu cieniującego macierz rzutowania
	glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V)); //Załaduj do programu cieniującego macierz widoku
	glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M)); //Załaduj do programu cieniującego macierz modelu


	glEnableVertexAttribArray(spTextured->a("vertex"));
	glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(spTextured->a("texCoord"));
	glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, myCubeTexCoords); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(spTextured->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(spTextured->a("vertex"));
	glDisableVertexAttribArray(spTextured->a("color"));
}


void drawFloor(glm::mat4 P, glm::mat4 V, float x, float z, float m, glm::vec3 translate_vec, bool secret = false) {
	glm::mat4 M_floor = glm::mat4(1.0f);
	//M_floor = glm::rotate(M_floor, float(glm::radians(90.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	M_floor = glm::translate(M_floor, translate_vec);
	M_floor = glm::scale(M_floor, glm::vec3(x, 0.001f, z));
	if (!secret) texKostka(P, V, M_floor, m, tex_floor);
	else texKostka(P, V, M_floor, m, tex_bricks);
}

void drawCeiling(glm::mat4 P, glm::mat4 V, float x, float z, float m, glm::vec3 translate_vec, bool secret = false) {
	glm::mat4 M_ceiling = glm::mat4(1.0f);
	M_ceiling = glm::translate(M_ceiling, glm::vec3(0.0f, 2.0f, 0.0f));
	M_ceiling = glm::translate(M_ceiling, translate_vec);
	M_ceiling = glm::scale(M_ceiling, glm::vec3(x, 0.001f, z));

	if (!secret) texKostka(P, V, M_ceiling, m, tex_ceiling);
	else texKostka(P, V, M_ceiling, m, tex_bricks);
}

void drawWall(glm::mat4 P, glm::mat4 V, float z, float rotate, float m, glm::vec3 translate_vec, bool secret = false) {
	glm::mat4 M_wall = glm::mat4(1.0f);
	M_wall = glm::rotate(M_wall, float(glm::radians(90.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
	M_wall = glm::rotate(M_wall, float(glm::radians(rotate)), glm::vec3(1.0f, 0.0f, 0.0f));

	M_wall = glm::translate(M_wall, translate_vec);
	M_wall = glm::scale(M_wall, glm::vec3(1.0f, 0.001f, z));
	if (!secret) texKostka(P, V, M_wall, m, tex_wall);
	else texKostka(P, V, M_wall, m, tex_bricks);
}


void drawPlant(glm::mat4 P, glm::mat4 V, float x, float y, float z) {

	//rysowanie plant
	glm::mat4 M_plant = glm::mat4(1.0f);
	M_plant = glm::translate(M_plant, glm::vec3(x, y - 0.13f, z));
	M_plant = glm::rotate(M_plant, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_plant = glm::scale(M_plant, glm::vec3(0.15f, 0.15f, 0.15f));
	plant.texModel(P, V, M_plant, tex_plant);

	//rysowanie doniczki
	glm::mat4 M_pot = glm::mat4(1.0f);
	M_pot = glm::translate(M_pot, glm::vec3(x, y, z));
	M_pot = glm::rotate(M_pot, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_pot = glm::scale(M_pot, glm::vec3(0.6f, 0.6f, 0.8f));
	pot.texModel(P, V, M_pot, tex_pot);
}


void drawPlant2(glm::mat4 P, glm::mat4 V, float x, float y, float z) {

	//rysowanie plant2
	glm::mat4 M_plant2 = glm::mat4(1.0f);
	M_plant2 = glm::translate(M_plant2, glm::vec3(x, y, z));
	M_plant2 = glm::rotate(M_plant2, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//M_plant2 = glm::scale(M_plant2, glm::vec3(0.2f, 0.2f, 0.2f));
	//M_plant2 = glm::scale(M_plant2, glm::vec3(0.15f, 0.15f, 0.15f));
	plant2.texModel(P, V, M_plant2, tex_plant2);

	//rysowanie doniczki
	glm::mat4 M_pot = glm::mat4(1.0f);
	M_pot = glm::translate(M_pot, glm::vec3(x, y, z));
	M_pot = glm::rotate(M_pot, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_pot = glm::scale(M_pot, glm::vec3(0.6f, 0.6f, 0.8f));
	pot.texModel(P, V, M_pot, tex_pot);
}


//void drawWolf(glm::mat4 P, glm::mat4 V, float x, float y, float z) {
//	glm::mat4 M_wolf = glm::mat4(1.0f);
//	M_wolf = glm::translate(M_wolf, glm::vec3(x, y, z));
//	//M_wolf = glm::rotate(M_wolf, glm::radians(-90.0f), glm::vec3(1, 0, 0));
//	//M_pot = glm::scale(M_wolf, glm::vec3(0.02f, 0.02f, 0.02f));
//	wolf.texModel(P, V, M_wolf, tex_wolf);
//}

void drawFrame(glm::mat4 P, glm::mat4 V, float x, float y, float z, float obrot, glm::vec3 skala) {

	glm::mat4 M_frame = glm::mat4(1.0f);
	M_frame = glm::translate(M_frame, glm::vec3(x, y, z + 0.09f));
	M_frame = glm::rotate(M_frame, glm::radians(obrot), glm::vec3(0, 1, 0));
	M_frame = glm::scale(M_frame, glm::vec3(0.01f, 0.01f, 0.01f));
	M_frame = glm::scale(M_frame, skala);
	frame.texModel(P, V, M_frame, tex_frame);
}

//jeśli chcemy narysować na prawej ścianie to obrot = -90, obrot2 = 0
//jeśli chcemy narysować na lewej ścianie to obrot = 90, obrot2 = 0
//jeśli chcemy narysować tyłem obrot = 90, obrot2 = -90
//jeśli chcemy narysować przodem to obrot = 90, obrot2 = 90

//Kasiu wiem że kazałaś mi nie pytać o tą funkcje, ale mam dużo pytań, bo koordynaty x,y,z się nie zgadzają -miau
void drawFrame2(glm::mat4 P, glm::mat4 V, float x, float y, float z, float obrot, float obrot2, glm::vec3 skala, GLuint picture) {

	glm::mat4 M_frame = glm::mat4(1.0f);
	glm::mat4 M_picture = glm::mat4(1.0f);
	if (obrot > 0) {
		M_frame = glm::translate(M_frame, glm::vec3(-x + 0.04f, y, -z));
		M_picture = glm::translate(M_picture, glm::vec3(-x + 0.01f, y - 0.9f, -z + 1.67f));
	}
	else {
		M_frame = glm::translate(M_frame, glm::vec3(x - 0.04f, y, z));
		M_picture = glm::translate(M_picture, glm::vec3(x - 0.01f, y - 0.9f, z - 1.67f));
	}

	M_frame = glm::rotate(M_frame, glm::radians(obrot), glm::vec3(0, 1, 0));
	M_frame = glm::rotate(M_frame, float(glm::radians(-obrot2)), glm::vec3(0, 1, 0));
	if (obrot2 < 0)	M_frame = glm::translate(M_frame, glm::vec3(1.75f, 0, -1.65));
	else if (obrot2 > 0)	M_frame = glm::translate(M_frame, glm::vec3(1.7f, 0, 1.7f));

	M_frame = glm::scale(M_frame, skala);
	frame2.texModel(P, V, M_frame, tex_frame2);

	M_picture = glm::rotate(M_picture, float(glm::radians(90.0f)), glm::vec3(0, 0, 1));
	M_picture = glm::rotate(M_picture, float(glm::radians(-obrot)), glm::vec3(0, 1, 0));
	M_picture = glm::rotate(M_picture, float(glm::radians(obrot2)), glm::vec3(0, 0, 1));
	M_picture = glm::scale(M_picture, glm::vec3(0.5f, 0.001f, 0.35f));
	M_picture = glm::scale(M_picture, skala);   //tego chyba brakowało, ale nigdzie nie skalowaliśmy -miau

	texKostka(P, V, M_picture, 1, picture);
}

void drawPic(glm::mat4 P, glm::mat4 V, float x, float y, float z, float obrot, float obrot2, glm::vec3 skala, GLuint picture) {
	glm::mat4 M_picture = glm::mat4(1.0f);

	if (obrot > 0) {
		M_picture = glm::translate(M_picture, glm::vec3(-x + 0.01f, y - 0.9f, -z + 1.67f));
	}
	else {
		M_picture = glm::translate(M_picture, glm::vec3(x - 0.01f, y - 0.9f, z - 1.67f));
	}
	//Kasiu mam coraz więcej pytań o te funkcje, ale działam z tym co mam
	M_picture = glm::rotate(M_picture, float(glm::radians(90.0f)), glm::vec3(0, 0, 1));
	M_picture = glm::rotate(M_picture, float(glm::radians(-obrot)), glm::vec3(0, 1, 0));
	M_picture = glm::rotate(M_picture, float(glm::radians(obrot2)), glm::vec3(0, 0, 1));
	M_picture = glm::scale(M_picture, glm::vec3(0.35f, 0.001f, 0.35f));
	M_picture = glm::scale(M_picture, skala);   //tego chyba brakowało, ale nigdzie nie skalowaliśmy -miau
	texKostka(P, V, M_picture, 1, picture);
}

void drawFrame3(glm::mat4 P, glm::mat4 V, float x, float y, float z, float obrot, glm::vec3 skala) {

	glm::mat4 M_frame = glm::mat4(1.0f);
	M_frame = glm::translate(M_frame, glm::vec3(x, y, z));
	M_frame = glm::rotate(M_frame, glm::radians(obrot), glm::vec3(0, 1, 0));
	M_frame = glm::scale(M_frame, skala);
	frame3.texModel(P, V, M_frame, tex_frame3);
}


void drawDoor(glm::mat4 P, glm::mat4 V, float x, float y, float z, bool lewe, bool open) {
	glm::mat4 M_door = glm::mat4(1.0f);

	if (not(open))
	{
		M_door = glm::translate(M_door, glm::vec3(x, y, z));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
		if (lewe) { M_door = glm::scale(M_door, glm::vec3(-1.0f, 1.0f, 1.0f)); }
	}
	if (open && not(lewe))
	{
		M_door = glm::translate(M_door, glm::vec3(x - 1.48, y, z - 1.5));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
		M_door = glm::scale(M_door, glm::vec3(1.0f, -1.0f, 1.0f));
	}
	if (open && lewe)
	{
		M_door = glm::translate(M_door, glm::vec3(x + 1.48, y, z - 1.5));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
	}
	//M_door = glm::scale(M_door, glm::vec3(0.7f, 0.7f, 0.7f));

	drzwi.texModel(P, V, M_door, tex_door);
}


void drawDoor2(glm::mat4 P, glm::mat4 V, float x, float y, float z, bool lewe, bool open, float angle) {
	// TA FUNKCJA JAK NARAZIE NIE MA ZUPEŁNIE SENSU  POZA TYM ŻE PRÓBUJE ANIMOWAĆ DRZWI XD
	// BO TEN GŁUPI MODEL< NIE JEST CHWYCONY NA ZAWIASIE :))

	glm::mat4 M_door = glm::mat4(1.0f);
	while (angle > 90) angle -= 90;
	while (angle < 90) angle += 90;    //zabezpieczenie żeby kąt był 0-90

	if (not(lewe))
	{
		M_door = glm::translate(M_door, glm::vec3(x - 1.48, y, z - 1.5));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		M_door = glm::rotate(M_door, glm::radians(-angle), glm::vec3(0, 0, 1));
		M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
		M_door = glm::scale(M_door, glm::vec3(1.0f, -1.0f, 1.0f));
	}
	if (lewe)
	{
		M_door = glm::translate(M_door, glm::vec3(x + 1.48, y, z - 1.5));
		M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		M_door = glm::rotate(M_door, glm::radians(-angle), glm::vec3(0, 0, 1));
		M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
	}

	drzwi.texModel(P, V, M_door, tex_door);
}

void drawDoor_static(glm::mat4 P, glm::mat4 V, float x, float y, float z, bool lewe) {
	glm::mat4 M_door = glm::mat4(1.0f);
	M_door = glm::translate(M_door, glm::vec3(x, y, z));
	M_door = glm::rotate(M_door, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_door = glm::scale(M_door, glm::vec3(0.3f, 0.18f, 0.25f));
	M_door = glm::scale(M_door, glm::vec3(0.7f, 0.7f, 0.7f));
	if (lewe) { M_door = glm::scale(M_door, glm::vec3(-1.0f, 1.0f, 1.0f)); }
	drzwi.texModel(P, V, M_door, tex_door);
}


void drawBudda(glm::mat4 P, glm::mat4 V, float x, float y, float z, float rotate) {

	glm::mat4 M_monument = glm::mat4(1.0f);
	M_monument = glm::translate(M_monument, glm::vec3(x, y, z));
	M_monument = glm::rotate(M_monument, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_monument = glm::rotate(M_monument, glm::radians(rotate), glm::vec3(0, 0, 1));
	M_monument = glm::scale(M_monument, glm::vec3(0.04f, 0.04f, 0.04f));
	monument.texModel(P, V, M_monument, tex_monument);
}

void drawWenus(glm::mat4 P, glm::mat4 V, float x, float y, float z, float rotate) {

	glm::mat4 M_wenus = glm::mat4(1.0f);
	M_wenus = glm::translate(M_wenus, glm::vec3(x, y, z));
	M_wenus = glm::rotate(M_wenus, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_wenus = glm::rotate(M_wenus, glm::radians(rotate), glm::vec3(0, 0, 1));
	M_wenus = glm::scale(M_wenus, glm::vec3(0.008f, 0.008f, 0.008f));
	wenus.texModel(P, V, M_wenus, tex_wenus);
}


void drawMoai(glm::mat4 P, glm::mat4 V, float x, float y, float z, float rotate) {

	glm::mat4 M_moai = glm::mat4(1.0f);
	M_moai = glm::translate(M_moai, glm::vec3(x, y - 0.1f, z + 0.2f));
	M_moai = glm::rotate(M_moai, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_moai = glm::rotate(M_moai, glm::radians(rotate), glm::vec3(0, 0, 1));
	M_moai = glm::scale(M_moai, glm::vec3(0.003f, 0.003f, 0.003f));
	moai.texModel(P, V, M_moai, tex_moai);
}


void drawDyskobol(glm::mat4 P, glm::mat4 V, float x, float y, float z, float rotate) {

	glm::mat4 M_dyskobol = glm::mat4(1.0f);
	M_dyskobol = glm::translate(M_dyskobol, glm::vec3(x, y, z));
	M_dyskobol = glm::rotate(M_dyskobol, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_dyskobol = glm::rotate(M_dyskobol, glm::radians(rotate), glm::vec3(0, 0, 1));
	M_dyskobol = glm::scale(M_dyskobol, glm::vec3(0.008f, 0.008f, 0.008f));
	dyskobol.texModel(P, V, M_dyskobol, tex_dyskobol);
}


void drawStool(glm::mat4 P, glm::mat4 V, float x, float y, float z, float rotate) {

	glm::mat4 M_stool = glm::mat4(1.0f);
	M_stool = glm::translate(M_stool, glm::vec3(x, y + 0.3, z));
	M_stool = glm::rotate(M_stool, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	M_stool = glm::rotate(M_stool, glm::radians(rotate), glm::vec3(0, 0, 1));
	M_stool = glm::scale(M_stool, glm::vec3(0.02f, 0.02f, 0.02f));
	stool.texModel(P, V, M_stool, tex_stool);
}

void drawRobot(glm::mat4 P, glm::mat4 V, float x, float y, float z, float angle, float dist, float rotation, glm::vec3 kierunek) {

	glm::mat4 M_robot = glm::mat4(1.0f);
	M_robot = glm::translate(M_robot, glm::vec3(x, y, z));
	M_robot = glm::translate(M_robot, kierunek * dist * sin(angle));
	M_robot = glm::rotate(M_robot, glm::radians(rotation), glm::vec3(0, 1, 0));  //obrót głowy, tylko
	M_robot = glm::scale(M_robot, glm::vec3(0.2f, 0.2f, 0.2f));
	robot.texModel(P, V, M_robot, tex_Robot);
}

void drawRobot2(glm::mat4 P, glm::mat4 V, float x, float y, float z, float angle, float dist, float rotation, glm::vec3 kierunek) {

	glm::mat4 M_robot = glm::mat4(1.0f);
	M_robot = glm::translate(M_robot, glm::vec3(x, y, z));
	M_robot = glm::translate(M_robot, kierunek * dist * sin(angle));
	M_robot = glm::rotate(M_robot, glm::radians(rotation), glm::vec3(0, 1, 0));  //obrót głowy, tylko
	M_robot = glm::scale(M_robot, glm::vec3(0.2f, 0.2f, 0.2f));
	robot.texModel(P, V, M_robot, tex_Robot2);
}

void drawRobot3(glm::mat4 P, glm::mat4 V, float x, float y, float z, float angle, float dist, float rotation, glm::vec3 kierunek, bool texx = true) {

	glm::mat4 M_robot = glm::mat4(1.0f);
	M_robot = glm::translate(M_robot, glm::vec3(x, y, z));
	if (angle < 0) angle *= -1.0f;
	while (angle > 90)  angle -= 90;
	angle -= 45;
	//w tym momencie angle jest w zakresie <-45; 45> i ładnie wejdzie w funkcje tangens
	M_robot = glm::translate(M_robot, kierunek * dist * tan(glm::radians(angle)));
	M_robot = glm::rotate(M_robot, glm::radians(rotation), glm::vec3(0, 1, 0));  //obrót głowy, tylko
	M_robot = glm::scale(M_robot, glm::vec3(0.2f, 0.2f, 0.2f));
	if (texx) robot.texModel(P, V, M_robot, tex_Robot2);
	else robot.texModel(P, V, M_robot, tex_Robot);
}




void drawRobot_static(glm::mat4 P, glm::mat4 V, float x, float y, float z, bool texx, float rot2, float rotation, float scale = 0.2) {

	glm::mat4 M_robot = glm::mat4(1.0f);
	M_robot = glm::translate(M_robot, glm::vec3(x, y, z));
	M_robot = glm::rotate(M_robot, glm::radians(rotation), glm::vec3(0, 1, 0));  //obrót głowy, tylko
	M_robot = glm::rotate(M_robot, glm::radians(rot2), glm::vec3(0, 0, 1));
	M_robot = glm::scale(M_robot, glm::vec3(scale, scale, scale));
	if (texx) robot.texModel(P, V, M_robot, tex_Robot);
	else robot.texModel(P, V, M_robot, tex_Robot2);
}

void drawRobot_circle(glm::mat4 P, glm::mat4 V, float x, float y, float z, float angle, bool texx, float dx, float dy, float dz, float rx, float ry, float rz, float ox, float oy, float oz, float rtx, float rty, float rtz, float scale = 0.2)
{
	// P, V - wiadomo
	// x, y, z wspolrzedne poczatkowe
	//texx - ktora tekstura robota
	//angle - zmienna przekazujaca uplyw czasu
	//dx, dy, dz - dystans na kazdej osi, "mnożnik"
	//rx, ry, rz - obrót na każdej z osi
	//ox, oy, oz - offset na kazdej z osi, do funkcji sin(angle + offset)
	//scale - oczywiście skala 
	//chyba dodam jeszcze jakieś zmienne od obracania się w czasie - rtx, rty, rtz 
	// note: możnaby to spakować w jakieś wektory, ale chyba nie ma to znaczenia, i tak jest "messy"
	glm::mat4 M_robot = glm::mat4(1.0f);
	glm::vec3 tran = glm::vec3(x + dx * sin(angle + ox), y + dy * sin(angle + oy), z + dz * sin(angle + oz));
	M_robot = glm::translate(M_robot, tran);
	M_robot = glm::rotate(M_robot, glm::radians(rx), glm::vec3(1, 0, 0));
	M_robot = glm::rotate(M_robot, glm::radians(ry), glm::vec3(0, 1, 0));
	M_robot = glm::rotate(M_robot, glm::radians(rz), glm::vec3(0, 0, 1));
	M_robot - glm::scale(M_robot, glm::vec3(scale, scale, scale));
	if (texx) robot.texModel(P, V, M_robot, tex_Robot);
	else robot.texModel(P, V, M_robot, tex_Robot2);

}



//****************************************************************************************************************************************

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle, float drzwi_angle) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości

	glm::mat4 V = glm::lookAt(cameraPosition, lookatPosition + cameraPosition, upVector); //Wylicz macierz widoku
	glm::mat4 P = glm::perspective(60.0f * PI / 180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania

	//Główne pomieszczenie
	drawFloor(P, V, 5.0f, 5.0f, 8, glm::vec3(0, 0, 0));
	drawCeiling(P, V, 5.0f, 5.0f, 6, glm::vec3(0, 0, 0));

	for (int i = 1; i <= 3; i++) {
		if (i == 2)
			drawWall(P, V, 5.0f, 90.0f, 5, glm::vec3(1.0f, -5.0f, 0.0f));

		if (i == 3)
			drawWall(P, V, 5.0f, 0, 5, glm::vec3(1.0f, 5.0f, 0.0f));
		else
			drawWall(P, V, 5.0f, 0, 5, glm::vec3(1.0f, -5.0f, 0.0f));
	}

	// ściany z wnęką na korytarz
	for (int i = 0; i < 2; i++) {
		if (i % 2) drawWall(P, V, 1.75f, 90.0f, 5, glm::vec3(1.0f, 5.0f, 3.25f));
		else drawWall(P, V, 1.75f, 90.0f, 5, glm::vec3(1.0f, 5.0f, -3.25f));
	}

	drawWall(P, V, 3.5f, 0, 5, glm::vec3(1.0f, 0.0f, -1.5f));

	for (int i = 0; i < 2; i++) {
		if (i % 2) {
			drawWall(P, V, 1.5f, 90.0f, 5, glm::vec3(1.0f, 2, 3.5f));
		}
		else {
			drawWall(P, V, 1.5f, 90.0f, 5, glm::vec3(1.0f, 2, -3.5f));
		}
	}

	drawWall(P, V, 0.75f, 90.0f, 5, glm::vec3(1.0f, 2, 0.0f));



	// Korytarz
	for (int i = 0; i < 3; i++) {
		drawFloor(P, V, 1.5f, 1.5f, 2, glm::vec3(0.0f, 0.0f, 6.5f + (3.0f * i)));
	}

	for (int i = 0; i < 2; i++) {
		if (i % 2) drawWall(P, V, 4.5f, 0.0f, 5, glm::vec3(1.0f, 1.5f, 9.5f));
		else drawWall(P, V, 4.5f, 0.0f, 5, glm::vec3(1.0f, -1.5f, 9.5f));
	}

	drawCeiling(P, V, 1.5f, 4.5f, 2, glm::vec3(0.0f, 0.0f, 9.5f));


	// Drugie pomieszczenie
	drawFloor(P, V, 3.0f, 3.0f, 5, glm::vec3(0.0f, 0.0f, 12.5f + 4.5f));
	drawCeiling(P, V, 3.0f, 3.0f, 4, glm::vec3(0.0f, 0.0f, 12.5f + 4.5f));

	for (int i = 1; i <= 3; i++) {
		if (i == 2)
			drawWall(P, V, 3.0f, 90.0f, 5, glm::vec3(1.0f, 20.0f, 0));

		if (i == 3)
			drawWall(P, V, 3.0f, 0, 5, glm::vec3(1.0f, 3.0f, 17));
		else
			drawWall(P, V, 3.0f, 0, 5, glm::vec3(1.0f, -3.0f, 17));
	}

	// ściany z wnęką na korytarz
	for (int i = 0; i < 2; i++) {
		if (i % 2) drawWall(P, V, 0.75f, 90.0f, 5, glm::vec3(1.0f, 14, 2.25f));
		else drawWall(P, V, 0.75, 90.0f, 5, glm::vec3(1.0f, 14, -2.25f));
	}


	float frame2Height = 1.9f;

	// pomieszczenie z lewej
	drawFrame(P, V, -2.5f, 1, -5, 0, glm::vec3(1.8f, 1.8f, 1.8f));
	drawPlant2(P, V, -1.2f - 2.5f, 0, -4.5f);
	drawPlant2(P, V, 1.2f - 2.5f, 0, -4.5f);

	drawFrame2(P, V, 5, frame2Height, 4.1f, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_cat1);
	drawFrame2(P, V, 5, frame2Height, 1.8f, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_cat4);
	drawMoai(P, V, -4.8f, 0, -1.5f, 90);

	drawFrame2(P, V, 0, frame2Height, -1.0f, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_cat2);
	drawFrame2(P, V, 0, frame2Height, 1.7f, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_cat3);
	drawMoai(P, V, -0.2f, 0, -1.5f, -90);

	drawFrame2(P, V, 3.5f, frame2Height, -0.305f, 90, -90, glm::vec3(1.0, 1.0, 1.0), tex_cat5);
	drawPlant2(P, V, 0.9f - 3.5f, 0, 1.65f);
	drawPlant2(P, V, -0.9f - 3.5f, 0, 1.65f);

	drawWenus(P, V, -2.5f, 0.0f, -1.5f, -1.0f);
	drawPlant(P, V, 0.6f - 2.5f, 0, -1.25f);
	drawPlant(P, V, -0.6f - 2.5f, 0, -1.25f);


	// pomieszczenie z prawej
	drawFrame(P, V, 2.5f, 1, -5, 0, glm::vec3(1.8f, 1.8f, 1.8f));
	drawPlant(P, V, -1.2f + 2.5f, 0, -4.5f);
	drawPlant(P, V, 1.2f + 2.5f, 0, -4.5f);

	drawFrame2(P, V, 5, frame2Height, -1.0f, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_bunia);
	drawFrame2(P, V, 5, frame2Height, 1.7f, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_bunia2);
	drawMoai(P, V, 4.8f, 0, -1.5f, -90);

	drawFrame2(P, V, 0, frame2Height, 4.1f, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_kira);
	drawFrame2(P, V, 0, frame2Height, 1.8f, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_kira2);
	drawMoai(P, V, 0.2f, 0, -1.5f, 90);

	drawFrame2(P, V, -3.5f, frame2Height, -0.305f, 90, -90, glm::vec3(1.0, 1.0, 1.0), tex_kira3);
	drawPlant(P, V, 0.9f + 3.5, 0, 1.5f);
	drawPlant(P, V, -0.9f + 3.5, 0, 1.5f);

	drawDyskobol(P, V, 2.5f, 0.0f, -1.5f, 90);
	drawPlant2(P, V, 0.6f + 2.5f, 0, -1.25f);
	drawPlant2(P, V, -0.6f + 2.5f, 0, -1.25f);

	drawFrame2(P, V, -3.25f, frame2Height, -0.35f, 90, 90, glm::vec3(1.0f, 1.0f, 1.0f), tex_kapelusz);
	drawFrame2(P, V, 3.25f, frame2Height, -0.35f, 90, 90, glm::vec3(1.0f, 1.0f, 1.0f), tex_kira4);

	// korytarz
	drawFrame3(P, V, 0.0f, 1.4f, 2.0f, 0, glm::vec3(1.0, 1.0, 1.0));
	drawMoai(P, V, 0.0f, 0.0f, 2.0f, 0);

	// korytarz z lewej
	drawPlant(P, V, -2.55f, 0.0f, 2.35f);
	drawFrame2(P, V, 5, frame2Height, -1.75f, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_wojtek);
	drawBudda(P, V, -4.5f, 0.0f, 4.5f, 90.0f);
	drawBudda(P, V, -4.5f, 0.0f, 2.5f, 90.0f);
	drawStool(P, V, -2.5f, 0.0f, 4.7f, 90);

	drawPlant(P, V, -1.0f, 0.0f, 5.5f);
	drawPlant2(P, V, -1.0f, 0.0f, 7.5f);
	drawPlant(P, V, -1.0f, 0.0f, 9.5f);
	drawPlant2(P, V, -1.0f, 0.0f, 11.5f);
	// drawPlant(P, V, -1.0f, 0.0f, 13.5f);  //w drzwiach

	// korytarz z prawej
	drawPlant(P, V, 2.55f, 0.0f, 2.35f);
	drawFrame2(P, V, 5, frame2Height, 5.25f, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_kasia);
	drawBudda(P, V, 4.5f, 0.0f, 4.5f, -90.0f);
	drawBudda(P, V, 4.5f, 0.0f, 2.5f, -90.0f);
	drawStool(P, V, 2.5f, 0.0f, 4.7f, 90);

	drawPlant(P, V, 1.0f, 0.0f, 5.45f);
	drawPlant2(P, V, 1.0f, 0.0f, 7.5f);
	drawPlant(P, V, 1.0f, 0.0f, 9.45f);
	drawPlant2(P, V, 1.0f, 0.0f, 11.5f);
	// drawPlant(P, V, 1.0f, 0.0f, 13.45f);  // w drzwiach


	// drugie pomieszczenie
	float baseZ = 20.0f;

	drawFrame2(P, V, -2.25f, frame2Height, -baseZ + 7.665f, 90, 90, glm::vec3(1.0f, 1.0f, 1.0f), tex_bunia2);
	drawFrame2(P, V, 2.25f, frame2Height, -baseZ + 7.665f, 90, 90, glm::vec3(1.0f, 1.0f, 1.0f), tex_kira2);

	drawBudda(P, V, -2.5f, 0.0f, baseZ - 5.0f, 45);
	drawBudda(P, V, 2.5f, 0.0f, baseZ - 5.0f, -45);

	drawFrame2(P, V, 3, frame2Height, -baseZ + 5.5f, 90, 0, glm::vec3(1.0f, 1.0f, 1.0f), tex_binia); // z lewej
	drawPlant2(P, V, 2.5f, 0.0f, baseZ - 2.75f);
	drawFrame2(P, V, 3, frame2Height, -baseZ + 3.5f, 90, 0, glm::vec3(1.0f, 1.0f, 1.0f), tex_binia2);

	drawFrame2(P, V, 3, frame2Height, baseZ - 2.0f, -90, 0, glm::vec3(1.0f, 1.0f, 1.0f), tex_hermes1); // z prawej
	drawPlant2(P, V, -2.5f, 0.0f, baseZ - 2.75f);
	drawFrame2(P, V, 3, frame2Height, baseZ + 0, -90, 0, glm::vec3(1.0f, 1.0f, 1.0f), tex_hermes2);

	drawPlant(P, V, -2.0f, 0.0f, baseZ - 0.5f);
	drawPlant(P, V, 2.0f, 0.0f, baseZ - 0.5f);

	//animowane modele i inne
	drawDoor(P, V, 0, 1, 13.9, false, OPEN_DOOR);    // te drzwi są click to open  na 'E'
	drawDoor(P, V, 0, 1, 13.9, true, OPEN_DOOR);

	//drawDoor2(P, V, 0, 1, 13.9, false, OPEN_DOOR, drzwi_angle);     //te drzwi udają animacje, ale działa dość... chu**** słabo ;) 
	//drawDoor2(P, V, 0, 1, 13.9, true, OPEN_DOOR, drzwi_angle);

	drawDoor_static(P, V, 0, 1, 20.016f, false);        //to te za kamerą startową, prowadzące do tajnego pokoju :P 
	drawDoor_static(P, V, 0, 1, 20.016f, true);


	// to są testowe robociki, żeby sprawdzić czy wszystko śmiga
	//drawRobot(P, V, 0, 1, 16, angle, 1.2, 60, glm::vec3(1,0,0));    
	// P,V, x,y,z angle - czas do funkcji tryg , dist - mnożnik dystansu jaki ma chodzić,rotation -  obrót głowy gdzie patrzy, wektor w jakim kierunku się porusza.
	//drawRobot2(P, V, 0, 1, 18, angle, -1.2, 0, glm::vec3(0, 0, 0));  //wektor 0,0,0 znaczy ze sie nie porusza
	//drawRobot2(P, V, 2.4, 1, 18, angle, 0.6, 155, glm::vec3(0, 0, 1));

	//totalnie sekretne pomieszczenie
	drawFloor(P, V, 3.0f, 3.0f, 5, glm::vec3(0.0f, 0.0f, 23.0f), true);
	drawCeiling(P, V, 3.0f, 3.0f, 4, glm::vec3(0.0f, 0.0f, 23.0f), true);
	for (int i = 1; i <= 3; i++) {
		if (i == 2)
			drawWall(P, V, 3.0f, 90.0f, 5, glm::vec3(1.0f, 26.0f, 0.0f), true);

		if (i == 3)
			drawWall(P, V, 3.0f, 0, 5, glm::vec3(1.0f, 3.0f, 23.0f), true);
		else
			drawWall(P, V, 3.0f, 0, 5, glm::vec3(1.0f, -3.0f, 23.0f), true);
	}
	drawWall(P, V, 3.0f, 90.0f, 5, glm::vec3(1.0f, 20.01f, 0.0f), true);
	//i wszystkie artefakty w środku, np. sterta robotów i pewien ważny obraz
	drawRobot_static(P, V, 2.0f, 0.2f, 21.0f, true, -6.7f, -90.0f);
	drawRobot_static(P, V, 1.8f, 0.2f, 21.3f, true, -6.7f, -90.0f);
	drawRobot_static(P, V, 2.1f, 0.5f, 21.0f, true, 15.0f, -77.0f);
	drawRobot_static(P, V, 2.0f, 0.3f, 21.3f, true, 60.0f, 0.0f);
	drawRobot_static(P, V, 2.2f, 0.4f, 21.6f, true, -6.7f, -113.0f);
	drawRobot_static(P, V, 1.9f, 0.3f, 21.5f, true, -45.0f, -45.0f);
	drawRobot_static(P, V, 2.0f, 0.9f, 22.8f, false, -6.7f, 180.0f, 0.5); // boss
	drawFrame2(P, V, 2.99, frame2Height, -19.1, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_kasia2);
	drawFrame2(P, V, 2.99, frame2Height, -21.1, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_wojtek2);
	drawFrame2(P, V, 2.99, frame2Height, -23.1, 90, 0, glm::vec3(1.0, 1.0, 1.0), tex_kotrog);
	//to sie nie skaluje dobrze, bo ramka ucieka na bok XD - zrobilem nowe drawpic, bez ramy, dziala :P
	drawPic(P, V, 0.0, 1.8, -24, 90, -90, glm::vec3(3.0, 3.0, 3.0), tex_kotrog2); //wstawmy tu jakiegoś głupiego mema, albo chociaż ładne zdjęcie jakieś
	drawRobot3(P, V, -6.4f, 0.9f, 14.0f, angle * 7, 6.66f, 0.0f, glm::vec3(1, 0, 1), false); //piekielny tajny robot z niewinną teksturą aniołka
	drawFrame2(P, V, 2.99, frame2Height, 23.1, -90, 0, glm::vec3(1.0, 1.0, 1.0), tex_meme2);
	drawPic(P, V, -2.99f, 1.85f, -23.1f, 90, 180, glm::vec3(3.0f * 0.5f, 4.0f * 0.5f, 2.0f), tex_meme1);
	drawPic(P, V, -2.0f, 1.85f, -18.4f, 90, 90, glm::vec3(1.5f, 2.0f, 2.0f), tex_kira4);


	//koniec totalnie sekretnego pomieszczenia

	// Inni zwiedzający, animowani:
	drawRobot2(P, V, 2.2, 1.1, 17, angle, 2.2, 60, glm::vec3(0, 0, 1));
	drawRobot2(P, V, -1.96, 0.6, 17, angle, -2.2, 240, glm::vec3(0, 0, 1));
	drawRobot2(P, V, 2.2, 0.7, 15.7, angle, -1.2, 60, glm::vec3(0, 0, 1));


	// w korytarzu
	drawRobot(P, V, 0, 0.8, 12, -angle, 0.4, 0, glm::vec3(1, 0, 0));


	// pokoje dalej
	//drawRobot_static(P, V, 0.0f, 1.0f, 0.0f, false, 0.0f, 0.0f, 0.5);  //placeholder pokazujacy mi 0.0.0 żebym stawiał reszte dobrze

	//drawRobot_circle(P, V, 0.0f, 1.0f, 3.0f, angle, false, 1.0f, 0.3f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f); //robi sie zbyt skomplikowane

	drawRobot2(P, V, 0.9f, 0.8f, -1.5f, angle, 2.2f, -90.0f, glm::vec3(0, 0, 1));
	drawRobot2(P, V, -4.1f, 0.8f, -1.5f, angle / 3.0, 2.2f, 180.0f, glm::vec3(0, 0, 1));
	drawRobot(P, V, 4.2f, 0.8f, -1.5f, 2 * angle, 2.2f, 0.0f, glm::vec3(0, 0, 1));
	drawRobot2(P, V, 1.9f, 0.7f, -3.5f, angle / 2.0, 1.6f, 180.0f, glm::vec3(1, 0, 0));

	drawRobot3(P, V, -0.4f, 0.9f, 9.0f, angle / 1.3, 4.5f, 0.0f, glm::vec3(0, 0, 1), true);

	drawRobot3(P, V, 0.0f, 0.7f, 3.22f, angle * 4, 2.7f, 90.0f, glm::vec3(1, 0, 0), true); //mnożnik angle to jak speed
	drawRobot3(P, V, 0.0f, 0.7f, 2.88f, angle * -4, -2.7f, -90.0f, glm::vec3(1, 0, 0), true);

	drawRobot_static(P, V, 3.4f, 0.87f, 0.0f, false, 0.0f, 0.0f, 0.5);
	drawRobot_static(P, V, 4.0f, 0.2f, 0.32f, true, -16.7f, -46.0f);
	drawRobot_static(P, V, 3.7f, 0.3f, 0.21f, true, -6.7f, 12.0f);
	drawRobot_static(P, V, 3.4f, 0.3f, 0.29f, true, -6.7f, -6.0f);
	drawRobot_static(P, V, 3.1f, 0.3f, 0.25f, false, -6.7f, -12.0f);
	drawRobot_static(P, V, 2.8f, 0.3f, 0.3f, true, -6.7f, 20.0f);


	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL. 
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla

	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		currentFrame = glfwGetTime();
		dt = currentFrame - prevFrame;
		prevFrame = currentFrame;

		glm::vec3 rightVector = glm::cross(lookatPosition, upVector);
		glm::vec3 leftVector = glm::cross(upVector, lookatPosition);
		glm::vec3 forwardVector = glm::vec3(lookatPosition.x, 0, lookatPosition.z);

		angle += speed * dt;
		drzwi_angle += doorspeed * dt;

		drawScene(window, angle, drzwi_angle); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
