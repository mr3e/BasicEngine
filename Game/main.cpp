#include "InputManager.h"
// #include "../DisplayGLFW/display.h"
#include "game.h"

#include "input.h"
#include <string>
#include <stb_image.h>

using namespace std;

int main(int argc, char* argv[])
{
	const int DISPLAY_WIDTH = 800;
	const int DISPLAY_HEIGHT = 800;
	const float CAMERA_ANGLE = 0.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game* scn = new Game(CAMERA_ANGLE, (float)DISPLAY_WIDTH / DISPLAY_HEIGHT, NEAR, FAR);

	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

	Init(display);

	scn->Init();

	display.SetScene(scn);

	// Loading the file
	input in = input("../scene.txt");

	// Calculating the image ray casting 

	scn->AddTexture(DISPLAY_WIDTH, DISPLAY_HEIGHT, in.getPicture(DISPLAY_WIDTH, DISPLAY_HEIGHT));

	while (!display.CloseWindow())
	{
		scn->Draw(1, 0, scn->BACK, true, false);
		scn->Motion();
		display.SwapBuffers();
		display.PollEvents();

	}
	delete scn;
	return 0;
}