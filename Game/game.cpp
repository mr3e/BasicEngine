#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "../Engine3D/stb_image.h"
#include <stdio.h>
#include <iostream>
#include "scene.h"

static void printMat(const glm::mat4 mat)
{
	std::cout << " matrix:" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << mat[j][i] << " ";
		std::cout << std::endl;
	}
}

Game::Game() : Scene(){}

Game::Game(float angle, float relationWH, float near1, float far1) : Scene(angle, relationWH, near1, far1)
{
	rCube = RubikCube(this);
	isAnimating = 0.0f;
}

void Game::Init()
{
	AddShader("../res/shaders/pickingShader");
	AddShader("../res/shaders/basicShader");
	AddTexture("../res/textures/plane.png", false);
	int counter = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				if (i != 0 || j != 0 || k != 0) {
					AddShape(Cube, -1, TRIANGLES);
					shapes[counter]->MyScale({ 0.5,0.5, 0.5 });
					shapes[counter]->MyTranslate({ i,j,k }, 0);
					SetShapeTex(counter, 0);
					counter++;
				}
			}
		}
	}

	printf("%d", counter);
	pickedShape = 1;
	MoveCamera(0, zTranslate, 20);
	pickedShape = -1;

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4& MVP, const glm::mat4& Model, const int  shaderIndx)
{
	if (!isAnimating && !moves.empty()) {
		char currentMove = moves.front();
		printf("pressed %c\n", currentMove);
		if (currentMove == 'R' || currentMove == 'L' || currentMove == 'U' || currentMove == 'D' ||  currentMove == 'F' || currentMove == 'B')
			isAnimating = 1;
		moves.pop();
		if (currentMove == 'R') {
			rCube.setArray(rCube.right, rotatingWall, 1);
			delta = glm::vec3(-1, 0, 0);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.right);
			else rCube.RotateFaceCounterClockWise(rCube.right);
		}
		else if (currentMove == 'L') {
			rCube.setArray(rCube.left, rotatingWall, 1);
			delta = glm::vec3 (1, 0, 0);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.left);
			else rCube.RotateFaceCounterClockWise(rCube.left);
		}
		else if (currentMove == 'U') {
			rCube.setArray(rCube.top, rotatingWall, 1);
			delta = glm::vec3 (0, -1, 0);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.top);
			else rCube.RotateFaceCounterClockWise(rCube.top);
		}
		else if (currentMove == 'D') {
			rCube.setArray(rCube.back, rotatingWall, 1);
			delta = glm::vec3 (0, 0, 1);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.back);
			else rCube.RotateFaceCounterClockWise(rCube.back);
		}
		else if (currentMove == 'F') {
			rCube.setArray(rCube.front, rotatingWall, 1);
			delta = glm::vec3 (0, 0, -1);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.front);
			else rCube.RotateFaceCounterClockWise(rCube.front);
		}
		else if (currentMove == 'B') {
			rCube.setArray(rCube.bottom, rotatingWall, 1);
			delta = glm::vec3 (0, 1, 0);
			if (rCube.direction == 1) rCube.RotateFaceClockWise(rCube.bottom);
			else rCube.RotateFaceCounterClockWise(rCube.bottom);
		}
		else if (currentMove == ' ') {
			if (rCube.direction == 1) rCube.direction = -1;
			else rCube.direction = 1;
		}
		else if (currentMove == 'Z') {
			if (rCube.rotationCount == 2.0f) rCube.rotationCount = 1.0f;
		}
		else if (currentMove == 'A') {
			if (rCube.rotationCount == 1.0f) rCube.rotationCount = 2.0f;
		}
	}

	if (isAnimating) {
		rCube.rotateWall(rotatingWall, delta);
		isAnimating = isAnimating + 0.01f;
		if (rCube.rotationCount == 1) {
			if (isAnimating >= 91.0f) {
				isAnimating = 0;
			}
		}
		else {
			if (isAnimating >= 181.0f) {
				isAnimating = 0;
			}
		}
	}
	Shader* s = shaders[shaderIndx];
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal", Model);
	s->Unbind();
}

void Game::WhenRotate(){}
void Game::WhenTranslate(){}
void Game::Motion() {if (isActive){}}

Game::~Game(void) {}
void RubikCube::setArray(int* from[3][3], int* (&to)[3][3], int shouldCopy) {
	if (!shouldCopy)
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
				to[i][j] = from[i][j];
		}
	else {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				int* temp = new int;
				*temp = *from[i][j];
				to[i][j] = temp;
			}
		game->isAnimating = 1;
	}
}

RubikCube::RubikCube(Game* game)
{
	this->game = game;
	direction = 1;			//0 is clockwise, 1 is counter clockwise
	rotationCount = 1.0f;		//amount of rotations
	int** arr = (int**)malloc(26*sizeof(int*));
	for (int i = 0; i < 26; i++) {
		arr[i] = (int*)malloc(sizeof(int));
		*arr[i] = i;
	}
	int* tempFront[3][3] = {{arr[2],arr[11],arr[19]},{arr[5],arr[13],arr[22]},{arr[8],arr[16],arr[25]}};
	int* tempBack[3][3] = {{arr[17],arr[9],arr[0]},{arr[20],arr[12],arr[3]},{arr[23],arr[14],arr[6]}};
	int* tempTop[3][3] = {{arr[8],arr[16],arr[25]},{arr[7],arr[15],arr[24]},{arr[6],arr[14],arr[23]}};
	int* tempBottom[3][3] = {{arr[0],arr[9],arr[17]},{arr[1],arr[10],arr[18]},{arr[2],arr[11],arr[19]}};
	int* tempLeft[3][3] = {{arr[0],arr[1],arr[2]},{arr[3],arr[4],arr[5]},{arr[6],arr[7],arr[8]}};
	int* tempRight[3][3] = {{arr[19],arr[18],arr[17]},{arr[22],arr[21],arr[20]},{arr[25],arr[24],arr[23]}};
	setArray(tempFront, front, 0); setArray(tempBack, back, 0); setArray(tempTop, top,0);
	setArray(tempBottom, bottom, 0); setArray(tempLeft, left, 0); setArray(tempRight, right, 0);
}

void RubikCube::rotateWall(int* wall[3][3], glm::vec3 delta) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
		{
			game->getShapes()[*wall[i][j]]->MyRotate(direction * 0.01f, delta, 0);
		}
	}
}

void RubikCube::RotateFaceClockWise(int* (&face)[3][3]) {
	for (int i = 0; i < rotationCount; i++) {
		int prev[] = { *face[1][0], *face[2][1], *face[2][0] };
		*face[2][0] = *face[0][0];
		*face[0][0] = *face[0][2];
		*face[1][0] = *face[0][1];

		*face[0][1] = *face[1][2];
		*face[2][1] = prev[0];

		*face[0][2] = *face[2][2];
		*face[1][2] = prev[1];
		*face[2][2] = prev[2];
	}
}
void RubikCube::RotateFaceCounterClockWise(int* (&face)[3][3]) {
	for(int i = 0; i < 3; i ++) RotateFaceClockWise(face);
}
