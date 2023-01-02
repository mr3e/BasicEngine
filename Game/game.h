#pragma once
#include "scene.h"
#include <array>
#include <queue>
class Game;
class RubikCube
{
public:
	// Constructor
	RubikCube() {};
	RubikCube(Game* game);
	void rotateWall(int* wall[3][3], glm::vec3 delta);
	int direction;
	float rotationCount;
	void setArray(int* from[3][3], int* (&to)[3][3], int shouldCopy);
	void RotateFaceClockWise(int* (&face)[3][3]);
	void RotateFaceCounterClockWise(int* (&face)[3][3]);
	Game* game;
	// Array representing the cube
	int* front[3][3];
	int* back[3][3];
	int* top[3][3];
	int* bottom[3][3];
	int* left[3][3];
	int* right[3][3];
};

class Game : public Scene
{
public:
	Game();
	Game(float angle, float relationWH, float near, float far);
	void Init();
	void Update(const glm::mat4& MVP, const glm::mat4& Model, const int  shaderIndx);
	void WhenRotate();
	void WhenTranslate();
	void Motion();
	~Game(void);
	RubikCube rCube;
	int* rotatingWall[3][3];
	glm::vec3 delta;
	float isAnimating;
	std::queue<char> moves;
};
