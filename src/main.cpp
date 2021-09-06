#include "config.h"
#include <GLFW/glfw3.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <Tmx.h>
#include "game/game.h"
#include "DebugColors.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int SCREEN_WIDTH = 800;
unsigned int SCREEN_HEIGHT = 600;

Game* game;

int main()
{
	/* Initialize glfw and create window*/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1);

	/* Load GLAD */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << DC_ERROR " Failed to initialize GLAD\n";
		return 1;
	}

	/* Game */
	game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT);

	// OpenGL configuration
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST | GL_CULL_FACE);

	 try
	 {
			game->Init();

			float deltaTime = 0.0f;
			float lastFrame = 0.0f;

			while (!glfwWindowShouldClose(window))
			{
				float currentFrame = (float)glfwGetTime();
				deltaTime = currentFrame - lastFrame;
				lastFrame = currentFrame;
				// Poll events like key presses, mouse event, ...
				glfwPollEvents();

				game->ProcessInput(deltaTime);
				game->Update(deltaTime);

				// Clear default framebuffer and render game scene.
				glClearColor(game->BackgroundColor.r, game->BackgroundColor.g, game->BackgroundColor.b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				game->Render();

				// Swap back and front buffers.
				glfwSwapBuffers(window);
			}
	 }
	 catch (std::exception e)
	 {
	 std::cerr << DC_ERROR " " << e.what() << std::endl;
	 }

	delete game;
	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCREEN_HEIGHT = height;
	SCREEN_WIDTH = width;
	glViewport(0, 0, width, height);
	game->Width = width;
	game->Height = height;
	game->OnResize();
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			game->Keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			game->Keys[key] = false;
			game->KeysProcessed[key] = false;
		}
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//if (firstmouse) {
	//	lastX = (float)xpos;
	//	lastY = (float)ypos;
	//	firstmouse = false;
	//}

	//float xoffset = (float)xpos - lastX;
	//float yoffset = lastY - (float)ypos;
	//lastX = (float)xpos;
	//lastY = (float)ypos;

	//myCamera.ProccessMouse(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	game->ProcessScroll((float)yoffset);
}