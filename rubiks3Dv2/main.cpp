//Rendering

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Matrix Math

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/matrix_decompose.hpp>

// Text Rendering

#include <ft2build.h>
#include FT_FREETYPE_H

// Functionality

#include <iostream>
#include <ctime>
#include <vector>
#include <array>
#include <fstream>
#include <string>

// Homebrew

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include "Constants.h"

#define YELLOW	1.00f, 	0.84f, 	0.00f
#define BLUE	0.00f, 	0.27f, 	0.78f
#define RED		0.72f, 	0.07f, 	0.10f
#define GREEN	0.00f, 	0.61f, 	0.12f
#define ORANGE	1.00f, 	0.35f, 	0.00f
#define WHITE	1.00f, 	1.00f, 	1.00f
#define BLACK	0.00f, 	0.00f, 	0.00f

const GLuint SCR_WIDTH = 1490;
//const GLuint SCR_WIDTH = 3456;
const GLuint SCR_HEIGHT = 810; 
//const GLuint SCR_HEIGHT = 1944; 

GLuint corner, edgeU, edgeR, edgeL, edgeD, center, blank; // Sticker Textures

class Cubelet;
struct Character;
struct Move;

class Cubelet
{
public:
	std::vector<glm::vec3> colors;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 transformation = glm::mat4(1.0f);
	std::vector<Cubelet*> children = {};
	std::string cubeID = "";
	std::array<int, 6> cubeType = {};

	Cubelet(std::vector<glm::vec3> colors, glm::vec3 position, std::string cubeID, std::array<int, 6> cubeType)
	{
		this->cubeID = cubeID;
		this->colors = colors;
		this->position = position;
		this->cubeType = cubeType;
		this->transformation = glm::translate(this->transformation, this->position);
	}
	void DrawFace(int face, float diffuseMultiplier, float ambientMultiplier, Shader* lightingShader)
	{
		if (cubeType[face] == CORNER)
			glBindTexture(GL_TEXTURE_2D, corner);
		else if (cubeType[face] == CENTER)
			glBindTexture(GL_TEXTURE_2D, center);
		else if (cubeType[face] == EDGEU)
			glBindTexture(GL_TEXTURE_2D, edgeU);
		else if (cubeType[face] == EDGER)
			glBindTexture(GL_TEXTURE_2D, edgeR);
		else if (cubeType[face] == EDGEL)
			glBindTexture(GL_TEXTURE_2D, edgeL);
		else if (cubeType[face] == EDGED)
			glBindTexture(GL_TEXTURE_2D, edgeD);
		else if (cubeType[face] == NONE)
			glBindTexture(GL_TEXTURE_2D, blank);

		lightingShader->SetVector3("material.ambient", this->colors[face][0] * ambientMultiplier, this->colors[face][1] * ambientMultiplier, this->colors[face][2] * ambientMultiplier);
		lightingShader->SetVector3("material.diffuse", this->colors[face][0] * diffuseMultiplier, this->colors[face][1] * diffuseMultiplier, this->colors[face][2] * diffuseMultiplier);
		glDrawArrays(GL_TRIANGLES, 6 * face, 6);
	}
	void Draw(Shader * lightingShader)
	{
		float diffuseMultiplier = 2.00f;
		float ambientMultiplier = 0.2f * diffuseMultiplier;

		lightingShader->Use();

		glm::mat4 model = glm::mat4(1.0f);
		model *= this->transformation;

		lightingShader->SetMatrix3("normal", glm::inverse(model));
		lightingShader->SetMatrix4("model", model);

		for (int i = 0; i < 6; i++)
			DrawFace(i, diffuseMultiplier, ambientMultiplier, lightingShader);
		for (size_t i = 0; i < children.size(); i++)
			children[i]->Draw(lightingShader);
	}
	void GetPosition(std::vector<glm::vec3> * positions)
	{
		positions->push_back(this->position);
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->GetPosition(positions);
		}
	}
	void AddChild(std::vector<glm::vec3> colors, glm::vec3 position, std::string ID, std::array<int, 6> cubeType)
	{
		children.push_back(new Cubelet(colors, position, ID, cubeType));
	}
	void CopyChild(Cubelet & cubeToCopy)
	{
		children.push_back(&cubeToCopy);
	}
	void RemoveChild(Cubelet & cubeToRemove)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			if (children[i] == &cubeToRemove)
			{
				children.erase(children.begin() + i);
				break;
			}
		}
	}
	void Rotate(float angle, glm::vec3 axis, glm::vec3 axis2, glm::mat4 transformation)
	{
		glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis2);
		glm::mat3 rrotation = glm::rotate(glm::mat4(1.0f), glm::radians(-angle), axis2);

		glm::vec3 scale; glm::quat rotation1; glm::vec3 skew; glm::vec4 perspective;

		this->transformation = glm::rotate(transformation, glm::radians(angle), axis);

		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->Rotate(angle, axis2, axis2, glm::mat4(rotation) * children[i]->transformation * glm::mat4(rrotation));
			glm::decompose(children[i]->transformation, scale, rotation1, children[i]->position, skew, perspective);
		}
	}
};

struct Character {
	GLuint  TextureID; // ID handle of the glyph texture
	glm::vec2 Size;  // Size of glyph
	glm::vec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint  Advance; // Offset to advance to next glyph
};

struct Move
{
	int face = 0;
	glm::vec3 axis = glm::vec3(0.0f);
	glm::vec3 axis2 = glm::vec3(0.0f);
	float angle = 0.0f;
	float length = 0.0f;
	bool child = true;
	float animationCurrentAngle = 0.0f;
	Move(int face, glm::vec3 axis, glm::vec3 axis2, float angle, float length, bool child = true)
	{
		this->face = face;
		this->axis = axis;
		this->axis2 = axis2;
		this->angle = angle;
		this->length = length;
		this->child = child;
		this->animationCurrentAngle = 0.0f;
	}
};

bool FloatComparison(float x, float y, float accuracy) { return std::abs(x - y) <= accuracy; }
bool InitOpenGL();
void LoadShaders();
void InitCamera();
void LoadCube();
void LoadVertices();
void LoadTextures();
void GenerateText(std::string fontPath, int pxSize);
void Load();
int main();
void Destroy();
void Update();
void SetPointLight(Shader* lightingShader, int ID);
void SetSpotlight(Shader* lightingShader);
void ConfigureLighting(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawLines(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawSkybox(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawModel(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawCube(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawTextToScreen();
void Draw();
void ProcessInput(GLFWwindow* window);
void Animate();
void UndoMove();
void PerformMove(int moveType, int moveDirection, float moveSpeed, bool multiple = false, bool thisUndo = false);
void SetFace(int direction, float moveSpeed, bool multiple = false, bool thisUndo = false);
void Scramble();
void Up(int direction, int faceIndex, float speed);
void Right(int direction, int faceIndex, float speed);
void Front(int direction, int faceIndex, float speed);
void Left(int direction, int faceIndex, float speed);
void Back(int direction, int faceIndex, float speed);
void Down(int direction, int faceIndex, float speed);
glm::vec3 GetAxis(int axisID, int faceIndex);
void UpdateChildren(int faceIndex, std::vector<glm::vec3> positions, std::vector<int> cubePositions);
void RenderText(Shader * textShader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
std::vector<float> LoadVertexData(std::string filepath);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

bool freeCam = false;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int xAxis = XAXIS;
int yAxis = YAXIS;
int zAxis = ZAXIS;
float timer = 0.0f;
bool timerActive = false;

int moveCount = 0;
bool scrambling = false;
int laf = 2; //LAF = look at Face
int laft = 0; // LAFT = look at face top
bool spotlight;

bool animating = false;
float animationTimer = 0.0f;
float animationCurrentAngle = 0.0f;
float animationNewAngle = 0.0f;
std::vector<std::array<int, 5>> moveQueue = {};
std::vector<std::array<int, 5>> undoStack = {};
std::vector<Move*> currentMoves = {};
std::map<GLchar, Character> Characters;

bool uPress = false;
bool rPress = false;
bool fPress = false;
bool lPress = false;
bool bPress = false;
bool dPress = false;
bool sPress = false;
bool hPress = false;
bool vPress = false;
bool mPress = false;
bool zPress = false;
bool tPress = false;
bool leftPress = false;
bool rightPress = false;
bool upPress = false;
bool downPress = false;
bool qPress = false;
bool spacePress = false;

Cubelet* cube = new Cubelet({ {BLACK}, {BLACK}, {BLACK}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(0.0f, 0.0f, 0.0f), "111", {NONE, NONE, NONE, NONE, NONE, NONE});

Shader* lightingShader = nullptr;
Shader* lineShader = nullptr;
Shader* skyboxShader = nullptr;
Shader* textShader = nullptr;
Shader* modelShader = nullptr;

Model* nanosuit = nullptr;

Camera camera(glm::vec3(0, 0, 0));

GLFWwindow* window = nullptr;
FT_Library ft;
FT_Face face;

GLuint cubeVBO, cubeVAO, axisVAO, axisVBO, skyboxVAO, skyboxVBO, skyboxTexture, textVAO, textVBO;;

float lightDistance = 6.0f;
glm::vec3 pointlightDirections[] = {
	glm::vec3(0.0f, lightDistance, 0.0f), 
	glm::vec3(lightDistance, 0.0f, 0.0f), 
	glm::vec3(0.0f, 0.0f, lightDistance), 
	glm::vec3(-lightDistance, 0.0f, 0.0f), 
	glm::vec3(0.0f, 0.0f, -lightDistance), 
	glm::vec3(0.0f, -lightDistance, 0.0f), 
	glm::vec3(0.5f, 0.5f, 0.5f), 
};

bool InitOpenGL()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rubik's Cube", nullptr, nullptr);
	if (window == NULL)
	{
		std::cerr << "ERROR: Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "ERROR: Failed to initialize the GLAD Library" << std::endl;
		return false;
	}

	if (FT_Init_FreeType(&ft))
		std::cerr << "ERROR: Could not initialize the FreeType Library" << std::endl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void LoadShaders()
{
	lightingShader = new Shader("res/shaders/multipleLights.vert", "res/shaders/multipleLights.frag");
	lineShader = new Shader("res/shaders/lines.vert", "res/shaders/lines.frag");
	skyboxShader = new Shader("res/shaders/skybox.vert", "res/shaders/skybox.frag");
	textShader = new Shader("res/shaders/text.vert", "res/shaders/text.frag");
	modelShader = new Shader("res/shaders/modelV3.vert", "res/shaders/modelV3.frag");
}

void InitCamera()
{
	camera.Position = glm::vec3(7.5, 11.25, 15);
	camera.Pitch = -35;
	camera.Yaw = -130;
	camera.Update();
}

void LoadCube()
{
	cube->AddChild({ {YELLOW}, {BLACK}, {BLACK}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(0.0f, 2.1f, 0.0f), "011", { CENTER, NONE, NONE, NONE, NONE, NONE }); // YELLOW
	cube->children.back()->AddChild({ {YELLOW}, {BLACK}, {RED}, {BLUE}, {BLACK}, {BLACK} }, glm::vec3(-2.1f, 2.1f, 2.1f), "000", { CORNER, NONE, CORNER, CORNER, NONE, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {BLACK}, {BLACK}, {BLUE}, {BLACK}, {BLACK} }, glm::vec3(-2.1f, 2.1f, 0.0f), "001", { EDGEL, NONE, NONE, EDGER, NONE, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {BLACK}, {BLACK}, {BLUE}, {ORANGE}, {BLACK} }, glm::vec3(-2.1f, 2.1f, -2.1f), "002", { CORNER, NONE, NONE, CORNER, CORNER, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {BLACK}, {RED}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(0.0f, 2.1f, 2.1f), "010", { EDGEU, NONE, EDGED, NONE, NONE, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {BLACK}, {BLACK}, {BLACK}, {ORANGE}, {BLACK} }, glm::vec3(0.0f, 2.1f, -2.1f), "012", { EDGED, NONE, NONE, NONE, EDGED, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {GREEN}, {RED}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(2.1f, 2.1f, 2.1f), "020", { CORNER, CORNER, CORNER, NONE, NONE, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {GREEN}, {BLACK}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(2.1f, 2.1f, 0.0f), "021", { EDGER, EDGER, NONE, NONE, NONE });
	cube->children.back()->AddChild({ {YELLOW}, {GREEN}, {BLACK}, {BLACK}, {ORANGE}, {BLACK} }, glm::vec3(2.1f, 2.1f, -2.1f), "022", { CORNER, CORNER, NONE, NONE, CORNER, NONE });

	cube->AddChild({ {BLACK}, {GREEN}, {BLACK}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(2.1f, 0.0f, 0.0f), "121", { NONE, CENTER, NONE, NONE, NONE, NONE }); // GREEN
	cube->children.back()->AddChild({ {BLACK}, {GREEN}, {RED}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(2.1f, 0.0f, 2.1f), "120", { NONE, EDGEU, EDGER, NONE, NONE, NONE });
	cube->children.back()->AddChild({ {BLACK}, {GREEN}, {BLACK}, {BLACK}, {ORANGE}, {BLACK} }, glm::vec3(2.1f, 0.0f, -2.1f), "122", { NONE, EDGED, NONE, NONE, EDGER, NONE });
	cube->children.back()->AddChild({ {BLACK}, {GREEN}, {RED}, {BLACK}, {BLACK}, {WHITE} }, glm::vec3(2.1f, -2.1f, 2.1f), "220", { NONE, CORNER, CORNER, NONE, NONE, CORNER });
	cube->children.back()->AddChild({ {BLACK}, {GREEN}, {BLACK}, {BLACK}, {BLACK}, {WHITE} }, glm::vec3(2.1f, -2.1f, 0.0f), "221", { NONE, EDGEL, NONE, NONE, NONE, EDGER });
	cube->children.back()->AddChild({ {BLACK}, {GREEN}, {BLACK}, {BLACK}, {ORANGE}, {WHITE} }, glm::vec3(2.1f, -2.1f, -2.1f), "222", { NONE, CORNER, NONE, NONE, CORNER, CORNER });

	cube->AddChild({ {BLACK}, {BLACK}, {RED}, {BLACK}, {BLACK}, {BLACK} }, glm::vec3(0.0f, 0.0f, 2.1f), "110", { NONE, NONE, CENTER, NONE, NONE, NONE }); // RED
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {RED}, {BLUE}, {BLACK}, {BLACK} }, glm::vec3(-2.1f, 0.0f, 2.1f), "100", { NONE, NONE, EDGEL, EDGEU, NONE, NONE });
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {RED}, {BLUE}, {BLACK}, {WHITE} }, glm::vec3(-2.1f, -2.1f, 2.1f), "200", { NONE, NONE, CORNER, CORNER, NONE, CORNER });
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {RED}, {BLACK}, {BLACK}, {WHITE} }, glm::vec3(0.0f, -2.1f, 2.1f), "210", { NONE, NONE, EDGEU, NONE, NONE, EDGEU });

	cube->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLUE}, {BLACK}, {BLACK} }, glm::vec3(-2.1f, 0.0f, 0.0f), "101", { NONE, NONE, NONE, CENTER, NONE, NONE }); // BLUE
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLUE}, {ORANGE}, {BLACK} }, glm::vec3(-2.1f, 0.0f, -2.1f), "102", { NONE, NONE, NONE, EDGED, EDGEL, NONE });
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLUE}, {BLACK}, {WHITE} }, glm::vec3(-2.1f, -2.1f, 0.0f), "201", { NONE, NONE, NONE, EDGEL, NONE, EDGEL });
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLUE}, {ORANGE}, {WHITE} }, glm::vec3(-2.1f, -2.1f, -2.1f), "202", { NONE, NONE, NONE, CORNER, CORNER, CORNER });

	cube->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLACK}, {ORANGE}, {BLACK} }, glm::vec3(0.0f, 0.0f, -2.1f), "112", { NONE, NONE, NONE, NONE, CENTER, NONE }); // ORANGE
	cube->children.back()->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLACK}, {ORANGE}, {WHITE} }, glm::vec3(0.0f, -2.1f, -2.1f), "212", { NONE, NONE, NONE, NONE, EDGEU, EDGED });

	cube->AddChild({ {BLACK}, {BLACK}, {BLACK}, {BLACK}, {BLACK}, {WHITE} }, glm::vec3(0.0f, -2.1f, 0.0f), "211", { NONE, NONE, NONE, NONE, NONE, CENTER }); // WHITE
}

void LoadVertices()
{
	std::vector<float>vertices = LoadVertexData("res/vertexFiles/cube.txt");

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	std::vector<float>skyboxVertices = LoadVertexData("res/vertexFiles/cubemap.txt");

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(GLfloat), &skyboxVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

	std::vector<float>axisVertices = LoadVertexData("res/vertexFiles/axes.txt");

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(GLfloat), &axisVertices[0], GL_STATIC_DRAW);

	glBindVertexArray(axisVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void LoadTextures()
{
	std::vector<std::string> faces
	{
		"res/cubemaps/indoors/MarriottMadisonWest/posx.jpg", 
		"res/cubemaps/indoors/MarriottMadisonWest/negx.jpg", 
		"res/cubemaps/indoors/MarriottMadisonWest/posy.jpg", 
		"res/cubemaps/indoors/MarriottMadisonWest/negy.jpg", 
		"res/cubemaps/indoors/MarriottMadisonWest/posz.jpg", 
		"res/cubemaps/indoors/MarriottMadisonWest/negz.jpg"
	};

	skyboxTexture = LoadTextureFromImage(CUBEMAP, faces);

	corner = LoadTextureFromImage(TEXTURE, { "res/textures/stickerCorner.png" });
	edgeU = LoadTextureFromImage(TEXTURE, { "res/textures/stickerEdgeU.png" });
	edgeR = LoadTextureFromImage(TEXTURE, { "res/textures/stickerEdgeR.png" });
	edgeL = LoadTextureFromImage(TEXTURE, { "res/textures/stickerEdgeL.png" });
	edgeD = LoadTextureFromImage(TEXTURE, { "res/textures/stickerEdgeD.png" });
	center = LoadTextureFromImage(TEXTURE, { "res/textures/stickerCenter.png" });
	blank = LoadTextureFromImage(TEXTURE, { "res/textures/stickerBlank.png" });

	lightingShader->Use();
	lightingShader->SetInt("material.diffuseMap", 0);

	skyboxShader->Use();
	skyboxShader->SetInt("skybox", 0);
}

void GenerateText(std::string fontPath, int pxSize)
{
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
		std::cerr << "ERROR: Failed to load a font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, pxSize); // Set px size

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cerr << "ERROR: Failed to load a glyph" << std::endl;
			continue;
		}
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 	0, 	GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {texture, glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x};
		Characters.insert({ c, character });
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

}

void Load()
{
	InitCamera();
	LoadShaders();
	nanosuit = new Model("res/models/nanosuit/nanosuit.obj");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader->Use();
	textShader->SetMatrix4("projection", projection);

	LoadCube();
	LoadVertices();
	LoadTextures();

	GenerateText("res/fonts/Retron2000.ttf", 48);
}

int main()
{
	if (!InitOpenGL()) return -1;

	srand(time(NULL));

	Load();

	while (!glfwWindowShouldClose(window))
	{
		Update();
		Draw();
	}
	Destroy();
	return 0;
}

void Destroy()
{
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteVertexArrays(1, &textVAO);
	glDeleteVertexArrays(1, &axisVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &textVBO);
	glDeleteBuffers(1, &axisVBO);
	glfwDestroyWindow(window);
	delete lightingShader;
	glfwTerminate();
	delete modelShader;
	delete skyboxShader;
	delete textShader;
	delete lineShader;
	delete nanosuit;
	for (size_t i = 0; i < currentMoves.size(); i++)
	{
		delete currentMoves[i];
	}
	exit(0);
}

void Update()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (timerActive)
		timer += deltaTime;

	ProcessInput(window);

	Animate();
}

void SetPointLight(Shader* lightingShader, int ID)
{
	lightingShader->SetVector3("pointLights["+std::to_string(ID)+"].position", pointlightDirections[ID]);
	lightingShader->SetVector3("pointLights["+std::to_string(ID)+"].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader->SetVector3("pointLights["+std::to_string(ID)+"].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader->SetVector3("pointLights["+std::to_string(ID)+"].specular", 1.0f, 1.0f, 1.0f);
	lightingShader->SetFloat("pointLights["+std::to_string(ID)+"].constant", 1.0f);
	lightingShader->SetFloat("pointLights["+std::to_string(ID)+"].linear", 0.09);
	lightingShader->SetFloat("pointLights["+std::to_string(ID)+"].quadratic", 0.032);
}

void SetSpotlight(Shader* lightingShader)
{
	lightingShader->SetVector3("spotLight.position", camera.Position);
	lightingShader->SetVector3("spotLight.direction", camera.Front);
	lightingShader->SetVector3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	lightingShader->SetVector3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	lightingShader->SetVector3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	lightingShader->SetFloat("spotLight.constant", 1.0f);
	lightingShader->SetFloat("spotLight.linear", 0.09);
	lightingShader->SetFloat("spotLight.quadratic", 0.032);
	lightingShader->SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	lightingShader->SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

void ConfigureLighting(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	// Uncomment for directional light (i.e. sun). Currently disabled due to colors becoming washed out.

	//lightingShader->SetVector3("dirLight.direction", lightDir);
	//lightingShader->SetVector3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	//lightingShader->SetVector3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	//lightingShader->SetVector3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	for (int i = 0; i < (sizeof(pointlightDirections) / sizeof(*pointlightDirections)); i++)
	{
		SetPointLight(lightingShader, i);
	}

	if (spotlight)
	{
		lightingShader->SetBool("spotlightOn", true);
		SetSpotlight(lightingShader);
	}
	else
	{
		lightingShader->SetBool("spotlightOn", false);
	}

	lightingShader->SetVector3("viewPos", camera.Position);

	lightingShader->SetVector3("material.specular", 1.0f, 1.0f, 1.0f);
	lightingShader->SetFloat("material.shininess", 64.0f);

	lightingShader->SetMatrix4("projection", projection);
	lightingShader->SetMatrix4("view", view);
	lightingShader->SetMatrix4("model", model);
}

void DrawLines(glm::mat4 projection, glm::mat4 view, glm::mat4 model )
{
	lineShader->Use();
	lineShader->SetMatrix4("projection", projection);
	lineShader->SetMatrix4("view", view);

	lineShader->SetVector3("lineColor", glm::vec3(BLUE)[0], glm::vec3(BLUE)[1], glm::vec3(BLUE)[2]);

	glBindVertexArray(axisVAO);

	lineShader->SetMatrix4("model", model);
	glDrawArrays(GL_LINES, 0, 2);

	lineShader->SetVector3("lineColor", glm::vec3(WHITE)[0], glm::vec3(WHITE)[1], glm::vec3(WHITE)[2]);

	lineShader->SetMatrix4("model", model);
	glDrawArrays(GL_LINES, 2, 2);

	lineShader->SetVector3("lineColor", glm::vec3(ORANGE)[0], glm::vec3(ORANGE)[1], glm::vec3(ORANGE)[2]);
	lineShader->SetMatrix4("model", model);
	glDrawArrays(GL_LINES, 4, 4);
}

void DrawSkybox(glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	glDepthFunc(GL_LEQUAL);
	skyboxShader->Use();
	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	skyboxShader->SetMatrix4("view", view);
	skyboxShader->SetMatrix4("projection", projection);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void DrawModel(Shader * lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	lightingShader->Use();

	ConfigureLighting(lightingShader, projection, view, model);
	lightingShader->SetMatrix4("projection", projection);
	lightingShader->SetMatrix4("view", view);
	float angle = glfwGetTime() * 100;
	model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

	lightingShader->SetMatrix4("model", model);
	lightingShader->SetMatrix4("normal", glm::inverse(model));
	nanosuit->Draw(*lightingShader);

}

void DrawCube(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	lightingShader->Use();

	ConfigureLighting(lightingShader, projection, view, model);

	glBindVertexArray(cubeVAO);
	glActiveTexture(GL_TEXTURE0);
	cube->Draw(lightingShader);

}

void DrawTextToScreen()
{
	// Manually aligned using various amounts of dots
	glDisable(GL_DEPTH_TEST);
	if (freeCam)
	{
		RenderText(textShader, "CAMERA MODE", 									25.0f, 				SCR_HEIGHT - 40.0f, 	0.7f, glm::vec3(YELLOW));
		RenderText(textShader, "W / UP  / NUMPAD 5 .... FORWARDS", 				25.0f, 				SCR_HEIGHT - 75.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "A / LEFT / NUMPAD 1 .... STRAFE LEFT", 			25.0f, 				SCR_HEIGHT - 105.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "S / DOWN / NUMPAD 2 .... BACKWARDS", 			25.0f, 				SCR_HEIGHT - 135.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "D / RIGHT / NUMPAD 3 .... STRAFE RIGHT", 		25.0f, 				SCR_HEIGHT - 165.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LSHIFT .... 'RUN'", 							25.0f, 				SCR_HEIGHT - 195.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "T ................... TOGGLE TORCH", 			25.0f, 				SCR_HEIGHT - 225.0f, 	0.5f, glm::vec3(WHITE));
	}
	else
	{
		RenderText(textShader, "RUBIK'S MODE", 									25.0f, 				SCR_HEIGHT - 40.0f, 	0.7f, glm::vec3(YELLOW));
		RenderText(textShader, "TIME: " + std::to_string(timer), 				SCR_WIDTH - 350.0f, SCR_HEIGHT - 40.0f, 	0.7f, glm::vec3(WHITE));
		RenderText(textShader, "U .... UP", 									25.0f, 				SCR_HEIGHT - 75.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "R .... RIGHT", 									25.0f, 				SCR_HEIGHT - 105.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "F .... FRONT", 									25.0f, 				SCR_HEIGHT - 135.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "L .... LEFT", 									25.0f, 				SCR_HEIGHT - 165.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "B .... BACK", 									25.0f, 				SCR_HEIGHT - 195.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "D .... DOWN", 									25.0f, 				SCR_HEIGHT - 225.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "H .... HORIZONTAL", 							25.0f, 				SCR_HEIGHT - 255.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "V .... VERTICAL", 								25.0f, 				SCR_HEIGHT - 285.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "M .... MIDDLE", 								25.0f, 				SCR_HEIGHT - 315.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LSHIFT + MOVE .... INVERSE MOVE", 				25.0f, 				SCR_HEIGHT - 360.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LALT + MOVE .......... TWO SLICE MOVE", 		25.0f, 				SCR_HEIGHT - 390.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "CTRL + R ................... RIGHT ALGORITHM", 	25.0f, 				SCR_HEIGHT - 420.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "CTRL + L ................... LEFT ALGORITHM", 	25.0f, 				SCR_HEIGHT - 450.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "CTRL + Z ................... UNDO LAST MOVE", 	25.0f, 				SCR_HEIGHT - 480.0f, 	0.5f, glm::vec3(WHITE));
		std::string undoString = "N/A";
		if (undoStack.size()) 
		{
			switch (undoStack.back()[0])
			{
			case MUP:
				undoString = "UP ";
				break;
			case MDOWN:
				undoString = "DOWN ";
				break;
			case MLEFT:
				undoString = "LEFT ";
				break;
			case MRIGHT:
				undoString = "RIGHT ";
				break;
			case MFRONT:
				undoString = "FRONT ";
				break;
			case MBACK:
				undoString = "BACK ";
				break;
			case DUP:
			case DDOWN:
				undoString = "X ";
				break;
			case DLEFT:
			case DRIGHT:
				undoString = "Y ";
				break;
			case DROLLR:
			case DROLLL:
				undoString = "Z ";
				break;
			}
			switch (undoStack.back()[0])
			{
			case MUP:
			case MDOWN:
			case MLEFT:
			case MRIGHT:
			case MFRONT:
			case MBACK:
				switch (undoStack.back()[2])
				{
				case INVERSE:
					undoString += "NORMAL";
					break;
				case NORMAL:
					undoString += "INVERSE";
					break;
				}
				break;
			case DUP:
			case DRIGHT:
			case DROLLL:
				undoString += "INVERSE";
				break;
			case DDOWN:
			case DLEFT:
			case DROLLR:
				undoString += "NORMAL";
				break;
			}
		}
		RenderText(textShader, "PREVIOUS MOVE = " + undoString, 								25.0f, 				225.0f, 0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LEFT ARROW .............................. ROTATE LEFT", 		25.0f, 				180.0f, 0.5f, glm::vec3(WHITE));
		RenderText(textShader, "RIGHT ARROW ........................... ROTATE RIGHT", 		25.0f, 				150.0f, 0.5f, glm::vec3(WHITE));
		RenderText(textShader, "UP ARROW .................................... ROTATE UP", 	25.0f, 				120.0f, 0.5f, glm::vec3(WHITE));
		RenderText(textShader, "DOWN ARROW .............................. ROTATE DOWN", 		25.0f, 				90.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LSHIFT + LEFT ARROW ....... ROLL LEFT", 						25.0f, 				60.0f, 	0.5f, glm::vec3(WHITE));
		RenderText(textShader, "LSHIFT + RIGHT ARROW .... ROLL RIGHT", 						25.0f, 				30.0f, 	0.5f, glm::vec3(WHITE));
	}
	glEnable(GL_DEPTH_TEST);

}

void Draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	DrawLines(projection, view, model);

	DrawCube(lightingShader, projection, view, model);

	DrawModel(modelShader, projection, view, model);

	DrawSkybox(projection, view, model);

	DrawTextToScreen();

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (!freeCam)
	{
		if (!animating && !scrambling)
		{
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				sPress = true;
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && sPress)
			{
				Scramble();
				sPress = false;
			}

			else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				upPress = true;
			else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && upPress)
			{
				SetFace(DUP, 400.0f);
				upPress = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				downPress = true;
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE && downPress)
			{
				SetFace(DDOWN, 400.0f);
				downPress = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				leftPress = true;
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE && leftPress)
			{
				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				{
					SetFace(DROLLL, 400.0f);
				}
				else
				{
					SetFace(DLEFT, 400.0f);
				}
				leftPress = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				rightPress = true;
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && rightPress)
			{
				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				{
					SetFace(DROLLR, 400.0f);
				}
				else
				{
					SetFace(DRIGHT, 400.0f);
				}

				rightPress = false;
			}

			else if ((timerActive && timer > 0.0f) || !timerActive)
			{

				if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
					zPress = true;
				else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE && zPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
					{
						UndoMove();
					}
					zPress = false;
				}


				else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
					hPress = true;
				else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE && hPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						PerformMove(MUP, NORMAL, 600.0f, true);
						PerformMove(MDOWN, INVERSE, 600.0f, true);
						SetFace(DLEFT, 600.0f, true);
					}
					else
					{

						SetFace(DRIGHT, 600.0f, true);
						PerformMove(MUP, INVERSE, 600.0f, true);
						PerformMove(MDOWN, NORMAL, 600.0f, true);
					}
					hPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
					vPress = true;
				else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE && vPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						PerformMove(MRIGHT, NORMAL, 600.0f, true);
						PerformMove(MLEFT, INVERSE, 600.0f, true);
						SetFace(DUP, 600.0f, true);
					}
					else
					{
						PerformMove(MRIGHT, INVERSE, 600.0f, true);
						PerformMove(MLEFT, NORMAL, 600.0f, true);
						SetFace(DDOWN, 600.0f, true);
					}
					vPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
					mPress = true;
				else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE && mPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						PerformMove(MBACK, INVERSE, 600.0f, true);
						PerformMove(MFRONT, NORMAL, 600.0f, true);
						SetFace(DROLLR, 600.0f, true);
					}
					else
					{
						PerformMove(MBACK, NORMAL, 600.0f, true);
						PerformMove(MFRONT, INVERSE, 600.0f, true);
						SetFace(DROLLL, 600.0f, true);
					}
					mPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
					uPress = true;
				else if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE && uPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MDOWN, INVERSE, 600.0f, true);
							SetFace(DLEFT, 600.0f, true);
						}
						else
						{
							PerformMove(MUP, INVERSE, 200.0f);
						}
					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							SetFace(DRIGHT, 600.0f, true);
							PerformMove(MDOWN, NORMAL, 600.0f, true);
						}
						else
						{
							PerformMove(MUP, NORMAL, 200.0f);
						}
					}
					uPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
					rPress = true;
				else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && rPress)
				{

					if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
						{
							PerformMove(MUP, NORMAL, 150.0f);
							PerformMove(MRIGHT, NORMAL, 150.0f);
							PerformMove(MUP, INVERSE, 150.0f);
							PerformMove(MRIGHT, INVERSE, 150.0f);
						}
						else
						{
							PerformMove(MRIGHT, NORMAL, 150.0f);
							PerformMove(MUP, NORMAL, 150.0f);
							PerformMove(MRIGHT, INVERSE, 150.0f);
							PerformMove(MUP, INVERSE, 150.0f);

						}
					}
					else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MLEFT, INVERSE, 600.0f, true);
							SetFace(DUP, 600.0f, true);
						}
						else
						{
							PerformMove(MRIGHT, INVERSE, 200.0f);
						}
					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MLEFT, NORMAL, 600.0f, true);
							SetFace(DDOWN, 600.0f, true);
						}
						else
						{
							PerformMove(MRIGHT, NORMAL, 200.0f);
						}
					}
					rPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
					fPress = true;
				else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && fPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MBACK, INVERSE, 600.0f, true);
							SetFace(DROLLR, 600.0f, true);
						}
						else
						{
							PerformMove(MFRONT, INVERSE, 200.0f);
						}
					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MBACK, NORMAL, 600.0f, true);
							SetFace(DROLLL, 600.0f, true);
						}
						else
						{
							PerformMove(MFRONT, NORMAL, 200.0f);
						}
					}
					fPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && uPress == false)
					lPress = true;
				else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE && lPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
						{
							PerformMove(MUP, INVERSE, 150.0f);
							PerformMove(MLEFT, INVERSE, 150.0f);
							PerformMove(MUP, NORMAL, 150.0f);
							PerformMove(MLEFT, NORMAL, 150.0f);
						}
						else
						{
							PerformMove(MLEFT, INVERSE, 150.0f);
							PerformMove(MUP, INVERSE, 150.0f);
							PerformMove(MLEFT, NORMAL, 150.0f);
							PerformMove(MUP, NORMAL, 150.0f);
						}
					}

					else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MRIGHT, INVERSE, 600.0f, true);
							SetFace(DDOWN, 600.0f, true);

						}
						else
						{
							PerformMove(MLEFT, INVERSE, 200.0f);
						}
					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MRIGHT, NORMAL, 600.0f, true);
							SetFace(DUP, 600.0f, true);

						}
						else
						{
							PerformMove(MLEFT, NORMAL, 200.0f);
						}
					}
					lPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
					bPress = true;
				else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && bPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MFRONT, INVERSE, 600.0f, true);
							SetFace(DROLLL, 600.0f, true);
						}
						else
						{
							PerformMove(MBACK, INVERSE, 200.0f);
						}

					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							PerformMove(MFRONT, NORMAL, 600.0f, true);
							SetFace(DROLLR, 600.0f, true);
						}
						else
						{
							PerformMove(MBACK, NORMAL, 200.0f);
						}

					}
					bPress = false;
				}
				else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					dPress = true;
				else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && dPress)
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{
							SetFace(DRIGHT, 600.0f, true);
							PerformMove(MUP, INVERSE, 600.0f, true);
						}
						else
						{
							PerformMove(MDOWN, INVERSE, 200.0f);
						}
					}
					else
					{
						if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
						{


							PerformMove(MUP, NORMAL, 600.0f, true);
							SetFace(DLEFT, 600.0f, true);
						}
						else
						{
							PerformMove(MDOWN, NORMAL, 200.0f);
						}
					}
					dPress = false;
				}

				else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
					spacePress = true;
				else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && spacePress)
				{
					timerActive = !timerActive;
					spacePress = false;
				}
			}
		}
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			camera.MovementSpeed = 10.0f;
		}
		else
		{
			camera.MovementSpeed = 5.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_5) || glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
		{
			camera.ProcessKeyboard(FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_KP_1) || glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
		{
			camera.ProcessKeyboard(LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_KP_2) || glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
		{
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_KP_3) || glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
		{
			camera.ProcessKeyboard(RIGHT, deltaTime);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS )
	{
		qPress = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE && qPress)
	{
		
		if (freeCam)
		{
			std::cout << "Rubiks Mode" << std::endl;
			InitCamera();
			freeCam = false;
		}
		else
		{
			std::cout << "Camera Mode" << std::endl;
			freeCam = true;
		}
		qPress = false;
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		tPress = true;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && tPress)
	{
		spotlight = !spotlight;
		tPress = false;
	}
}

void Animate()
{
	if (scrambling && moveQueue.size() == 0)
	{
		scrambling = false;
		timerActive = true;
		timer = -15.0f;
	}

	if (currentMoves.size() == 0 && moveQueue.size() > 0)
	{
		int i = 0;
		do
		{
			switch (moveQueue.back()[0])
			{
			case MUP:
				Up(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case MRIGHT:
				Right(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case MFRONT:
				Front(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case MLEFT:
				Left(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case MBACK:
				Back(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case MDOWN:
				Down(moveQueue.back()[2], moveQueue.back()[3], moveQueue.back()[1]);
				break;
			case DUP:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, moveQueue.back()[1], false));
				break;
			case DRIGHT:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, moveQueue.back()[1], false));
				break;
			case DROLLR:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, moveQueue.back()[1], false));
				break;
			case DLEFT:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, moveQueue.back()[1], false));
				break;
			case DROLLL:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), -90.0f, moveQueue.back()[1], false));
				break;
			case DDOWN:
				animating = true;
				currentMoves.push_back(new Move(0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, moveQueue.back()[1], false));
				break;
			}
			if ((moveQueue.size() > 1 && moveQueue[moveQueue.size() - 1][4] != true) || moveQueue.size() <= 1)
			{
				i = 1;
			}

			moveQueue.pop_back();

		} while (i != 1);
	}

	if (animating)
	{
		for (int i = 0; i < currentMoves.size(); i++)
		{
			if (currentMoves[i] != nullptr)
			{
				animationTimer += deltaTime * 1000;
				if (animationTimer >= currentMoves[i]->length)
				{
					if (currentMoves[i]->child)
					{
						cube->children[currentMoves[i]->face]->Rotate(-currentMoves[i]->animationCurrentAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->children[currentMoves[i]->face]->transformation);
						cube->children[currentMoves[i]->face]->Rotate(currentMoves[i]->angle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->children[currentMoves[i]->face]->transformation);
					}
					else
					{
						cube->Rotate(-currentMoves[i]->animationCurrentAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->transformation);
						cube->Rotate(currentMoves[i]->angle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->transformation);
					}
					if (currentMoves.size() == 1)
					{
						animating = false;
						animationTimer = 0.0f;
					}
					currentMoves.erase(currentMoves.begin() + i);
					break;
				}
				else
				{
					if (currentMoves[i]->child)
					{
						cube->children[currentMoves[i]->face]->Rotate(-currentMoves[i]->animationCurrentAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->children[currentMoves[i]->face]->transformation);
					}
					else
					{
						cube->Rotate(-currentMoves[i]->animationCurrentAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->transformation);
					}

					animationNewAngle = (animationTimer / currentMoves[i]->length) * currentMoves[i]->angle;

					if (currentMoves[i]->child)
					{
						cube->children[currentMoves[i]->face]->Rotate(animationNewAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->children[currentMoves[i]->face]->transformation);
					}
					else
					{
						cube->Rotate(animationNewAngle, currentMoves[i]->axis, currentMoves[i]->axis2, cube->transformation);
					}
					currentMoves[i]->animationCurrentAngle = animationNewAngle;
				}
			}
		}
	}
}

void UndoMove()
{
	if (undoStack.size() > 0)
	{
		switch (undoStack.back()[0])
		{
		case MUP:
		case MRIGHT:
		case MLEFT:
		case MDOWN:
		case MFRONT:
		case MBACK:
			PerformMove(undoStack.back()[0], undoStack.back()[2], undoStack.back()[1], true, true);
			break;
		case DUP:
		case DDOWN:
		case DLEFT:
		case DRIGHT:
		case DROLLR:
		case DROLLL:
			SetFace(undoStack.back()[0], undoStack.back()[1], true, true);
			break;
		}
		undoStack.pop_back();
	}
}

void PerformMove(int moveType, int moveDirection, float moveSpeed, bool multiple, bool thisUndo)
{
	int face = 0;

	for (size_t i = 0; i < cube->children.size(); i++)
	{
		switch (moveType)
		{
		case MUP:
			if (FloatComparison(cube->children[i]->position.y, 2.1f, 0.1f)) face = i;
			break;											 
		case MRIGHT:										 
			if (FloatComparison(cube->children[i]->position.x, 2.1f, 0.1f)) face = i;									 
			break;											 
		case MFRONT:										 
			if (FloatComparison(cube->children[i]->position.z, 2.1f, 0.1f)) face = i;
			break;
		case MLEFT:
			if (FloatComparison(cube->children[i]->position.x, -2.1f, 0.1f)) face = i;
			break;
		case MBACK:
			if (FloatComparison(cube->children[i]->position.z, -2.1f, 0.1f)) face = i;
			break;
		case MDOWN:
			if (FloatComparison(cube->children[i]->position.y, -2.1f, 0.1f)) face = i;
			break;
		}
	}

	moveQueue.insert(moveQueue.begin(), { moveType, static_cast<int>(moveSpeed), moveDirection, face, multiple });
	if (!thisUndo)
	{
		if (moveDirection == NORMAL)
			undoStack.push_back({ moveType, static_cast<int>(100.0f), INVERSE, face, multiple });
		else
			undoStack.push_back({ moveType, static_cast<int>(100.0f), NORMAL, face, multiple });
	}
}

void SetFace(int direction, float moveSpeed, bool multiple, bool thisUndo)
{
	moveQueue.insert(moveQueue.begin(), { direction, static_cast<int>(moveSpeed), NULL, NULL, multiple });

	if (!thisUndo)
	{
		switch (direction)
		{
		case DUP:
			undoStack.push_back({ DDOWN, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		case DRIGHT:
			undoStack.push_back({ DLEFT, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		case DROLLR:
			undoStack.push_back({ DROLLL, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		case DLEFT:
			undoStack.push_back({ DRIGHT, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		case DROLLL:
			undoStack.push_back({ DROLLR, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		case DDOWN:
			undoStack.push_back({ DUP, static_cast<int>(moveSpeed), NULL, NULL, multiple });
			break;
		}
	}
	
	std::vector<std::vector<std::vector<std::string>>> lookUpTable =
	{
		{{"00", "OG", "GR", "RB", "RG", "00"}, 
		 {"00", "RG", "BR", "OB", "GO", "00"}, 
		 {"00", "YO", "YG", "YR", "YB", "00"}, 
		 {"00", "YR", "YB", "YO", "YG", "00"}, 
		 {"00", "GW", "RW", "BW", "OW", "00"}, 
		 {"00", "BY", "OY", "GY", "RY", "00"}}, 
		{{"RY", "00", "WR", "00", "YO", "OW"}, 
		 {"OY", "00", "YR", "00", "WO", "RW"}, 
		 {"GR", "00", "GW", "00", "GY", "GO"}, 
		 {"GO", "00", "GY", "00", "GW", "GR"}, 
		 {"YB", "00", "RB", "00", "OB", "WB"}, 
		 {"WG", "00", "OG", "00", "RG", "YG"}}, 
		{{"BY", "YG", "00", "WB", "00", "GW"}, 
		 {"GY", "WG", "00", "YB", "00", "BW"}, 
		 {"RB", "RY", "00", "RW", "00", "RG"}, 
		 {"RG", "RW", "00", "RY", "00", "RB"}, 
		 {"YO", "GO", "00", "BO", "00", "WO"}, 
		 {"WR", "BR", "00", "GR", "00", "YR"}}, 
		{{"OY", "00", "YR", "00", "WO", "RW"}, 
		 {"RY", "00", "WR", "00", "YO", "OW"}, 
		 {"BO", "00", "BY", "00", "BW", "BR"}, 
		 {"BR", "00", "BW", "00", "BY", "BO"}, 
		 {"YG", "00", "RG", "00", "OG", "WB"}, 
		 {"WB", "00", "OB", "00", "RB", "YB"}}, 
		{{"GY", "WG", "00", "YB", "00", "BW"}, 
		 {"BY", "YG", "00", "WB", "00", "GW"}, 
		 {"OG", "OW", "00", "OY", "00", "OB"}, 
		 {"OB", "OY", "00", "OW", "00", "OG"}, 
		 {"YR", "GR", "00", "BR", "00", "WR"}, 
		 {"WO", "BO", "00", "GO", "00", "YO"}}, 
		{{"00", "RG", "BR", "OB", "GO", "00"}, 
		 {"00", "OG", "GR", "RB", "BO", "00"}, 
		 {"00", "WR", "WB", "WO", "WG", "00"}, 
		 {"00", "WO", "WG", "WR", "WB", "00"}, 
		 {"00", "GY", "RY", "BY", "OY", "00"}, 
		 {"00", "BW", "OW", "GW", "RW", "00"}}, 
	};

	int newLAF, newLAFT;

	if (lookUpTable[laf][direction][laft][0] == 'Y')
		newLAF = FYELLOW;
	else if (lookUpTable[laf][direction][laft][0] == 'G')
		newLAF = FGREEN;
	else if (lookUpTable[laf][direction][laft][0] == 'R')
		newLAF = FRED;
	else if (lookUpTable[laf][direction][laft][0] == 'B')
		newLAF = FBLUE;
	else if (lookUpTable[laf][direction][laft][0] == 'O')
		newLAF = FORANGE;
	else if (lookUpTable[laf][direction][laft][0] == 'W')
		newLAF = FWHITE;
	if (lookUpTable[laf][direction][laft][1] == 'Y')
		newLAFT = FYELLOW;
	else if (lookUpTable[laf][direction][laft][1] == 'G')
		newLAFT = FGREEN;
	else if (lookUpTable[laf][direction][laft][1] == 'R')
		newLAFT = FRED;
	else if (lookUpTable[laf][direction][laft][1] == 'B')
		newLAFT = FBLUE;
	else if (lookUpTable[laf][direction][laft][1] == 'O')
		newLAFT = FORANGE;
	else if (lookUpTable[laf][direction][laft][1] == 'W')
		newLAFT = FWHITE;

	laf = newLAF;
	laft = newLAFT;

	switch (laf)
	{
	case FYELLOW:
	case FWHITE:
		switch (laft)
		{
		case FBLUE:
		case FGREEN:
			xAxis = ZAXIS;
			yAxis = XAXIS;
			break;
		case FRED:
		case FORANGE:
			xAxis = XAXIS;
			yAxis = ZAXIS;
			break;
		}
		zAxis = YAXIS;
		break;
	case FRED:
	case FORANGE:
		switch (laft)
		{
		case FGREEN:
		case FBLUE:
			xAxis = YAXIS;
			yAxis = XAXIS;
			break;
		case FWHITE:
		case FYELLOW:
			xAxis = XAXIS;
			yAxis = YAXIS;
			break;
		}
		zAxis = ZAXIS;
		break;
	case FGREEN:
	case FBLUE:

		switch (laft)
		{
		case FRED:
		case FORANGE:
			xAxis = YAXIS;
			yAxis = ZAXIS;
			break;
		case FYELLOW:
		case FWHITE:
			xAxis = ZAXIS;
			yAxis = YAXIS;
			break;
		}
		zAxis = XAXIS;
		break;
	}
}

void Scramble()
{
	scrambling = true;
	timer = -15.0f;
	timerActive = false;
	for (int i = 0; i < 20; i++)
	{
		int moveID = rand() % 12;
		switch (moveID)
		{
		case 0:
			PerformMove(MUP, NORMAL, 100.0f);
			break;
		case 1:
			PerformMove(MRIGHT, NORMAL, 100.0f);
			break;
		case 2:
			PerformMove(MFRONT, NORMAL, 100.0f);
			break;
		case 3:
			PerformMove(MLEFT, NORMAL, 100.0f);
			break;
		case 4:
			PerformMove(MBACK, NORMAL, 100.0f);
			break;
		case 5:
			PerformMove(MDOWN, NORMAL, 100.0f);
			break;
		case 6:
			PerformMove(MUP, INVERSE, 100.0f);
			break;									 
		case 7:										 
			PerformMove(MRIGHT, INVERSE, 100.0f);
			break;
		case 8:
			PerformMove(MFRONT, INVERSE, 100.0f);
			break;
		case 9:
			PerformMove(MLEFT, INVERSE, 100.0f);
			break;
		case 10:
			PerformMove(MBACK, INVERSE, 100.0f);
			break;
		case 11:
			PerformMove(MDOWN, INVERSE, 100.0f);
			break;
		}
	}
}

void Up(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].x - cube->children[faceIndex]->position.x) * (positions[i].x - cube->children[faceIndex]->position.x)) + ((positions[i].z - cube->children[faceIndex]->position.z) * (positions[i].z - cube->children[faceIndex]->position.z)));
		if (displacement != 0.0f && FloatComparison(positions[i].y, cube->children[faceIndex]->position.y, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}

	UpdateChildren(faceIndex, positions, cubePositions);

	glm::vec3 axis = GetAxis(yAxis, faceIndex);

	animating = true;
	if (direction == NORMAL)
	{
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, speed));
		std::cout << "UN" << std::endl;
	}
	else
	{
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, speed));
		std::cout << "UI" << std::endl;
	}
}

void Right(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].y - cube->children[faceIndex]->position.y) * (positions[i].y - cube->children[faceIndex]->position.y)) + ((positions[i].z - cube->children[faceIndex]->position.z) * (positions[i].z - cube->children[faceIndex]->position.z)));
		if (displacement != 0.0f && FloatComparison(positions[i].x, cube->children[faceIndex]->position.x, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}


	UpdateChildren(faceIndex, positions, cubePositions);

	glm::vec3 axis = GetAxis(xAxis, faceIndex);

	animating = true;
	if (direction == NORMAL)
	{
		std::cout << "RN" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, speed));
	}
	else
	{
		std::cout << "RI" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, speed));
	}
}

void Front(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].y - cube->children[faceIndex]->position.y) * (positions[i].y - cube->children[faceIndex]->position.y)) + ((positions[i].x - cube->children[faceIndex]->position.x) * (positions[i].x - cube->children[faceIndex]->position.x)));
		if (displacement != 0.0f && FloatComparison(positions[i].z, cube->children[faceIndex]->position.z, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}


	UpdateChildren(faceIndex, positions, cubePositions);


	glm::vec3 axis = GetAxis(zAxis, faceIndex);

	animating = true;
	if (direction == NORMAL)
	{
		std::cout << "FN" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), -90.0f, speed));
	}
	else
	{
		std::cout << "FI" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, speed));
	}
}

void Left(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].y - cube->children[faceIndex]->position.y) * (positions[i].y - cube->children[faceIndex]->position.y)) + ((positions[i].z - cube->children[faceIndex]->position.z) * (positions[i].z - cube->children[faceIndex]->position.z)));
		if (displacement != 0.0f && FloatComparison(positions[i].x, cube->children[faceIndex]->position.x, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}


	UpdateChildren(faceIndex, positions, cubePositions);

	glm::vec3 axis = GetAxis(xAxis, faceIndex);

	animating = true;
	if (direction == NORMAL)
	{
		std::cout << "LN" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, speed));
	}
	else
	{
		std::cout << "LI" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, speed));
	}
}

void Back(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].y - cube->children[faceIndex]->position.y) * (positions[i].y - cube->children[faceIndex]->position.y)) + ((positions[i].x - cube->children[faceIndex]->position.x) * (positions[i].x - cube->children[faceIndex]->position.x)));
		if (displacement != 0.0f && FloatComparison(positions[i].z, cube->children[faceIndex]->position.z, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}


	UpdateChildren(faceIndex, positions, cubePositions);
	glm::vec3 axis = GetAxis(zAxis, faceIndex);

	animating = true;
	if (direction == NORMAL)
	{
		std::cout << "BN" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, speed));
	}
	else
	{
		std::cout << "BI" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), -90.0f, speed));
	}
}

void Down(int direction, int faceIndex, float speed)
{
	std::vector<glm::vec3> positions = {};
	cube->GetPosition(&positions);

	std::vector<int> cubePositions = {};
	for (size_t i = 0; i < positions.size(); i++)
	{
		float displacement = sqrt(((positions[i].x - cube->children[faceIndex]->position.x) * (positions[i].x - cube->children[faceIndex]->position.x)) + ((positions[i].z - cube->children[faceIndex]->position.z) * (positions[i].z - cube->children[faceIndex]->position.z)));
		if (displacement != 0.0f && FloatComparison(positions[i].y, cube->children[faceIndex]->position.y, 0.1f))
		{
			cubePositions.push_back(i);
		}
	}

	UpdateChildren(faceIndex, positions, cubePositions);
	glm::vec3 axis = GetAxis(yAxis, faceIndex);
	animating = true;
	if (direction == NORMAL)
	{
		std::cout << "DN" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, speed));
	}
	else
	{
		std::cout << "DI" << std::endl;
		currentMoves.push_back(new Move(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, speed));
	}
}

glm::vec3 GetAxis(int axisID, int faceIndex)
{
	glm::vec3 axis = glm::vec3(0, 0, 0);

	switch (axisID)
	{
	case XAXIS:
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case YAXIS:
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case ZAXIS:
		axis = glm::vec3(0.0f, 0.0f, 1.0f);
		break;

	}

	if (faceIndex != FYELLOW && faceIndex != FGREEN && faceIndex != FRED)
	{
		axis *= -1;
	}

	return axis;
}

void UpdateChildren(int faceIndex, std::vector<glm::vec3> positions, std::vector<int> cubePositions)
{
	if (cubePositions.size() > 0)
	{
		for (int i = 0; i < 8; i++) // Always a ring of 8 cubes around any given center
		{
			for (size_t j = 0; j < cube->children.size(); j++)
			{
				if (j != faceIndex)
				{
					for (size_t k = 0; k < cube->children[j]->children.size(); k++)
					{
						if (FloatComparison(cube->children[j]->children[k]->position.x, positions[cubePositions[i]].x, 0.1f) && FloatComparison(cube->children[j]->children[k]->position.y, positions[cubePositions[i]].y, 0.1f) && FloatComparison(cube->children[j]->children[k]->position.z, positions[cubePositions[i]].z, 0.1f))
						{
							cube->children[faceIndex]->CopyChild(*cube->children[j]->children[k]);
							cube->children[j]->RemoveChild(*cube->children[j]->children[k]);
							break;
						}
					}
				}
			}
		}
	}
}

void RenderText(Shader* textShader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	textShader->Use();
	textShader->SetVector3("textColor", glm::vec3(color.x, color.y, color.z));

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	for (std::string::const_iterator c = text.begin(); c != text.end(); c++) // Loop through text
	{
		Character currentCharacter = Characters[*c]; // Set character from map based on the char in the text

		GLfloat xpos = x + currentCharacter.Bearing.x * scale;
		GLfloat ypos = y - (currentCharacter.Size.y - currentCharacter.Bearing.y) * scale;

		GLfloat width  = currentCharacter.Size.x * scale;
		GLfloat height = currentCharacter.Size.y * scale;

		GLfloat quadVertices[6][4] = { // Character Quad
			{ xpos, 		ypos + height, 	0.0f, 0.0f }, // BL
			{ xpos, 		ypos, 			0.0f, 1.0f }, // TL
			{ xpos + width, ypos, 			1.0f, 1.0f }, // TR						 		 	 	 
			{ xpos, 		ypos + height, 	0.0f, 0.0f }, // BL
			{ xpos + width, ypos, 			1.0f, 1.0f }, // TR
			{ xpos + width, ypos + height, 	1.0f, 0.0f } // BR
		};

		glBindTexture(GL_TEXTURE_2D, currentCharacter.TextureID);

		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices), quadVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (currentCharacter.Advance >> 6) * scale; // Adjust for next character
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

std::vector<float> LoadVertexData(std::string filepath)
{
	std::ifstream vertexFile;
	vertexFile.open(filepath);
	if (vertexFile.is_open())
	{

		std::vector<float> data;
		float tmp;
		while (vertexFile >> tmp)
		{
			data.push_back(tmp);
		}
		return data;
	}
	vertexFile.close();
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (freeCam)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (freeCam)
		camera.ProcessMouseScroll(yoffset);
}