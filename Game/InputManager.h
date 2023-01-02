#pragma once
#include "display.h"
#include "game.h"


	void mouse_callback(GLFWwindow* window,int button, int action, int mods)
	{	
		if(action == GLFW_PRESS )
		{
			Game *scn = (Game*)glfwGetWindowUserPointer(window);
			double x2,y2;
			glfwGetCursorPos(window,&x2,&y2);
			scn->Picking((int)x2,(int)y2);
		}
	}
	
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);
		scn->MyTranslate(glm::vec3(0,0,xoffset),0);
		
	}
	
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);
		glm::vec3 yAxis(0, 1, 0);
		glm::vec3 xAxis(1, 0, 0);
		char rotations[] = { 'U','D','L','R','F','B',' ','Z','A'};
		if(action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch (key)
			{case GLFW_KEY_R:
				scn->moves.push('R');
				break;
			case GLFW_KEY_L:
				scn->moves.push('L');
				break;
			case GLFW_KEY_U:
				scn->moves.push('U');
				break;
			case GLFW_KEY_D:
				scn->moves.push('D');
				break;
			case GLFW_KEY_B:
				scn->moves.push('B');
				break;
			case GLFW_KEY_F:
				scn->moves.push('F');
				break;
			case GLFW_KEY_SPACE:
				scn->moves.push(' ');
				break;
			case GLFW_KEY_Z:
				scn->moves.push('Z');
				break;
			case GLFW_KEY_A:
				scn->moves.push('A');
				break;
			case GLFW_KEY_M:
				for (int i = 0; i < 100; i++) {
					scn->moves.push(rotations[rand() % 9]);
				}
				break;

			case GLFW_KEY_ESCAPE:			
				glfwSetWindowShouldClose(window,GLFW_TRUE);
			break;

			case GLFW_KEY_UP:
				scn->MyRotate(-5, yAxis, 0);
				break;
			case GLFW_KEY_DOWN:
				scn->MyRotate(5, yAxis, 0);
				break;
			case GLFW_KEY_RIGHT:
				scn->MyRotate(-5, xAxis, 0);
				break;
			case GLFW_KEY_LEFT:
				scn->MyRotate(5, xAxis, 0);
				break;

			default:
				break;
			}
		}
	}
	
	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);

		scn->UpdatePosition((float)xpos,(float)ypos);
		if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			scn->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);
		}
		else if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			scn->MouseProccessing(GLFW_MOUSE_BUTTON_LEFT);
		}

	}

	void window_size_callback(GLFWwindow* window, int width, int height)
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);

		scn->Resize(width,height);
		
	}

	void Init(Display &display)
	{
		display.AddKeyCallBack(key_callback);
		display.AddMouseCallBacks(mouse_callback,scroll_callback,cursor_position_callback);
		display.AddResizeCallBack(window_size_callback);
	}
