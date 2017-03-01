/**
 *	@file	chip8.cpp
 *	@author	Dejan Azinovic (dazinovic)
 *	@date	01.03.2017
 *
 *	A simple OpenGL application that demonstrates the Chip-8 emulator.
 *	Input keys are hard-coded (0-9, A-F). Sound can be toggled off or on
 *	by pressing P. Emulation speed can be changed with the plus and minus
 *	keys (dependant on platform and keyboard layout).
 *
 *	Command line usage:
 *
 *	> Chip8Emulator Chip8Application
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "chip8.h"

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void resize_callback(GLFWwindow* window, int width, int height);
void change_sleep_time(GLFWwindow* window, int newSleepTime);
void process_input();

// Window dimensions
GLuint windowWidth  = 800;
GLuint windowHeight = 600;

// Timing
int sleepTime = 16;

// Input
unsigned char keys[1024];

// Emulator
Chip8 emulator;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: Chip8Emulator Chip8Application" << std::endl << std::endl;
		return -1;
	}

	// Load game
	if (!emulator.LoadApplication(argv[1]))
	{
		return -1;
	}

	// Initialize GLFW
	glfwInit();

	// Set window properties
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Chip-8 Emulator", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Register callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resize_callback);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Default window position
	glfwSetWindowPos(window, 500, 200);

	// Set viewport
	int viewportWidth, viewportHeight;
	glfwGetFramebufferSize(window, &viewportWidth, &viewportHeight);
	glViewport(0, 0, viewportWidth, viewportHeight);

	// Vsync
	glfwSwapInterval(1);

	// Screen data
	std::vector<unsigned char> screen(3 * emulator.SCREEN_WIDTH * emulator.SCREEN_HEIGHT);

	// The texture we're going to render to
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, emulator.SCREEN_WIDTH, emulator.SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Associate the texture with the framebuffer
	GLuint framebufferId;
	glGenFramebuffers(1, &framebufferId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Create main loop
	while (!glfwWindowShouldClose(window))
	{
		// Emulate one cycle
		emulator.EmulateCycle();

		// Copy the black & white emulator screen into the RGB screen
		for (int i = 0; i < emulator.SCREEN_WIDTH * emulator.SCREEN_HEIGHT; i++)
		{
			unsigned char pixelIntensity = (emulator.screen[i] == 0) ? 0 : 255;
			screen[3 * i] = pixelIntensity;
			screen[3 * i + 1] = pixelIntensity;
			screen[3 * i + 2] = pixelIntensity;
		}

		// Draw the screen data into the framebuffer
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, emulator.SCREEN_WIDTH, emulator.SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, screen.data());
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId);
		glBlitFramebuffer(0, emulator.SCREEN_HEIGHT, emulator.SCREEN_WIDTH, 0,
						  0, 0, windowWidth, windowHeight,
						  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		// Swap buffers
		glfwSwapBuffers(window);

		// Allow other processes to run
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

		// Check for input
		glfwPollEvents();
		process_input();
	}

	// Clean up resources
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// Process keyboard input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Exit application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

	// Change emulation speed
	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) // Plus
	{
		change_sleep_time(window, sleepTime / 2);
	}
	else if (key == GLFW_KEY_SLASH && action == GLFW_PRESS) // Minus
	{
		change_sleep_time(window, sleepTime * 2);
	}

	// Toggle sound
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		emulator.ToggleSound();
	}

	// Register which keys are currently pressed
	if (action == GLFW_PRESS)
	{
		keys[key] = 1;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = 0;
	}
}

// Update the window width and height
void resize_callback(GLFWwindow * window, int width, int height)
{
	windowWidth  = width;
	windowHeight = height;
}

// Changes the sleep time to the provided sleep time while ensuring that it
// doesn't go out of bounds. Also sets the appropriate window title.
void change_sleep_time(GLFWwindow* window, int newSleepTime)
{
	if (newSleepTime == 0)
	{
		sleepTime = 1;
	}
	else if (newSleepTime >= 64)
	{
		sleepTime = 64;
	}
	else
	{
		sleepTime = newSleepTime;
	}

	// Set window title
	if (sleepTime != 16)
	{
		std::string number = std::to_string(16.0 / sleepTime);
		while (number[number.length() - 1] == '0' || number[number.length() - 1] == '.')
		{
			number = number.substr(0, number.length() - 1);
		}
		std::string title = "Chip-8 Emulator | Speed: " + number + "x";
		glfwSetWindowTitle(window, title.c_str());
	}
	else
	{
		glfwSetWindowTitle(window, "Chip-8 Emulator");
	}
}

// Set the appropriate emulator keys
void process_input()
{
	emulator.keys[0x0] = keys[GLFW_KEY_0];
	emulator.keys[0x1] = keys[GLFW_KEY_1];
	emulator.keys[0x2] = keys[GLFW_KEY_2];
	emulator.keys[0x3] = keys[GLFW_KEY_3];
	emulator.keys[0x4] = keys[GLFW_KEY_4];
	emulator.keys[0x5] = keys[GLFW_KEY_5];
	emulator.keys[0x6] = keys[GLFW_KEY_6];
	emulator.keys[0x7] = keys[GLFW_KEY_7];
	emulator.keys[0x8] = keys[GLFW_KEY_8];
	emulator.keys[0x9] = keys[GLFW_KEY_9];
	emulator.keys[0xA] = keys[GLFW_KEY_A];
	emulator.keys[0xB] = keys[GLFW_KEY_B];
	emulator.keys[0xC] = keys[GLFW_KEY_C];
	emulator.keys[0xD] = keys[GLFW_KEY_D];
	emulator.keys[0xE] = keys[GLFW_KEY_E];
	emulator.keys[0xF] = keys[GLFW_KEY_F];
}