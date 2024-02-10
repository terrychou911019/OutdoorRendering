#include "src/RenderWidgets/RenderingOrderExp.h"

// the include of "glad" must be before the "glfw"
#include <GLFW\glfw3.h>
#include <iostream>

#define IMGUI_ENABLED
#define VSYNC_DISABLED

#define STB_IMAGE_IMPLEMENTATION
#include "../../../externals/include/stb_image.h"

#include "../externals/include/assimp/cimport.h"
#include "../externals/include/assimp/scene.h"
#include "../externals/include/assimp/postprocess.h"
#include "../externals/include/assimp/Importer.hpp"

#ifdef IMGUI_ENABLED
#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#endif // IMGUI_ENABLED

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

const int FRAME_WIDTH = 1344;
const int FRAME_HEIGHT = 756;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool initializeGL();
void resizeGL(GLFWwindow* window, int w, int h);
void paintGL();
void updateState();

INANOA::RenderingOrderExp* m_renderWidget = nullptr;

double PROGRAM_FPS = 0.0;
double FRAME_MS = 0.0;

int cameraDir = 0;
float cameraRad = 0;

float cameraYaw = 0;
float cameraPitch = 0;
float lastCameraYaw = 0;
float lastCameraPitch = 0;

bool mousePressed = false;
bool mouseFirstClicked = false;
float lastMouseX = 0;
float lastMouseY = 0;
float mouseClickX = 0;
float mouseClickY = 0;
float mouseReleaseX = 0;
float mouseReleaseY = 0;


int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// load OpenGL function pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(window, resizeGL);

	if (initializeGL() == false) {
		glfwTerminate();
		return 0;
	}

#ifdef IMGUI_ENABLED
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
#endif // IMGUI_ENABLED


#ifdef VSYNC_DISABLED
	glfwSwapInterval(0);
#endif // VSYNC_DISABLED

	double previousTimeStamp = glfwGetTime();
	int frameCounter = 0;
	while (!glfwWindowShouldClose(window)) {
		// ===============================
		const double timeStamp = glfwGetTime();
		const double deltaTime = timeStamp - previousTimeStamp;
		if (deltaTime >= 1.0) {
			PROGRAM_FPS = frameCounter / deltaTime;

			// reset
			frameCounter = 0;
			previousTimeStamp = timeStamp;
		}
		frameCounter = frameCounter + 1;
		// ===============================

		glfwPollEvents();

		updateState();
		paintGL();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

bool initializeGL() {

	// initialize render widget
	m_renderWidget = new INANOA::RenderingOrderExp();
	if (m_renderWidget->init(FRAME_WIDTH, FRAME_HEIGHT) == false) {
		return false;
	}
	
	return true;
}
void resizeGL(GLFWwindow* window, int w, int h) {
	m_renderWidget->resize(w, h);
}
// m_renderWidget has god camera, player camera
void updateState() {
	m_renderWidget->update(cameraDir, cameraRad, cameraYaw, cameraPitch);
}

//draw the FPS
void paintGL() {

#ifdef IMGUI_ENABLED
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
#endif // IMGUI_ENABLED

	m_renderWidget->render();

	static char fpsBuf[] = "fps: 000000000.000000000";
	static char msBuf[] = "ms: 000000000.000000000";
	sprintf_s(fpsBuf + 5, 16, "%.5f", PROGRAM_FPS);
	sprintf_s(msBuf + 4, 16, "%.5f", (1000.0 / PROGRAM_FPS));

#ifdef IMGUI_ENABLED	
	ImGui::Begin("Information");
	ImGui::Text(fpsBuf);
	ImGui::Text(msBuf);
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // IMGUI_ENABLED
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		mouseClickX = mouseX;
		mouseClickY = mouseY;
		mousePressed = true;
	}

	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		mouseReleaseX = mouseX;
		mouseReleaseY = mouseY;
		mousePressed = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {}

	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {}
}
void cursorPosCallback(GLFWwindow* window, double x, double y) {
	if (mousePressed) {
		float xOffset;
		float yOffset;
		lastMouseX = mouseClickX;
		lastMouseY = mouseClickY;
		xOffset = x - lastMouseX;
		yOffset = y - lastMouseY;
		xOffset *= 0.1f;
		yOffset *= 0.1f;

		cameraYaw = lastCameraYaw + xOffset;
		cameraPitch = std::max(-89.0f, std::min(89.0f, lastCameraPitch + yOffset));

		lastCameraYaw = cameraYaw;
		lastCameraPitch = cameraPitch;
		mouseClickX = x;
		mouseClickY = y;
	}
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W) {
			cameraDir = 1;
		}
		else if (key == GLFW_KEY_S) {
			cameraDir = -1;
		}
		else if (key == GLFW_KEY_A) {
			cameraRad = 0.0005;
		}
		else if (key == GLFW_KEY_D) {
			cameraRad = -0.0005;
		}
		else {
			cout << key << "\n";
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			cameraDir = 0;
		}
		else if (key == GLFW_KEY_S) {
			cameraDir = 0;
		}
		else if (key == GLFW_KEY_A) {
			cameraRad = 0;
		}
		else if (key == GLFW_KEY_D) {
			cameraRad = 0;
		}
		else {
			cout << key << "\n";
		}
	}
}
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {}