#pragma once
#include "InputManager.h"//needs to be at top
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <ctime>
#include <vector>
#include <string>
#include <thread>
#include <functional>
#include <utility>
#include "Scene.h"
#include "Shader.h"
#include "WindowCreator.h"
#include "Camera.h"
#include "Model.h"
#include "ExtraMath.h"
#include "FrameBuffer.h"
#include "LightManager.h"
#include "Text.h"
#include "SkyBox.h"


//#define sleep(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))

class GameEmGine
{
public:
	GameEmGine() = delete;
	static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	static void init(std::string name, int width, int height, int x = 0, int y = 0, int monitor = 0, bool fullScreen = false, bool visable = true);


	/*
	Creates a new window
	*/
	static void createNewWindow(std::string name, int width, int height, int x = 0, int y = 0, int monitor = 0, bool fullScreen = false, bool visable = true);

	/*
	Runs the engine.
	does not exit loop until window is exited or void exit(); is called
	*/
	static void run();

	/*
	Exists the game
	*/
	static void exit();


	static void setScene(Scene* scene);

	/*
	Set background colour in sigle bite (255 per colour) format
	*/
	static void setBackgroundColour(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1);


	/*
	Gets window width in pixels
	*/
	static int getWindowWidth();
	/*
	Gets window height in pixels
	*/
	static int getWindowHeight();


	/*
	Gets window size in pixels
	*/
	static Coord2D<int> getWindowSize();

	static Camera* getMainCamera();

	static bool mouseCollision(Model* model);

	static void setCameraType(Camera::CAM_TYPE type);
	static void setCameraType(ProjectionPeramiters* proj);

	/*
	moves the camera position in pixels
	*/
	static void translateCameraBy(Vec3 pos);
	/*
	sets the camera position in pixels
	*/
	static void translateCamera(Vec3 pos);

	/*
	moves the camera angle
	*/
	static void rotateCameraBy(Vec3 direction);

	/*
		sets the camera angle
	*/
	static void rotateCamera(Vec3 direction);

	//Adds a new model to the draw list
	static void addModel(Model* model);

	//Adds new text to the draw list
	static void addText(Text* text);

	static void removeModel(Model* model);

	static void removeText(Text* text);

	static void addCamera(Camera* camera);
	
	static std::unordered_map<void*, Model*>& getObjectList();

	static void setFPSLimit(short limit);
	static short getFPSLimit();

	/*enables/disables V-sync*/
	static void vsync(bool enable);

	static void updateControllerConnections();

	static WindowCreator* getWindow();

	/*Controller input*/

	static int controllersConnected();

	static bool isControllerConnected(int m_index);

	static XinputDevice* getController(int m_index);

	
	static Texture2D* m_LUT;

	static bool lutActive;
	static Texture3D tmpLUT;

private:
	static void initShader();
	static void calculateFPS();
	static void fpsLimiter();


	/*
	static void InitOpenGlCallback ();
	static void OpenGLDebugCallback (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * msg, const void * data);
	*/

	/*Custom effects*/
	static void customRenderCallback(std::function<void(FrameBuffer* gbuff, FrameBuffer* post,float dt)>);
	static void update();
	static void changeViewport(GLFWwindow* win, int w, int h);



	
	//static GLuint colorCustom;
	//static int LUTsize;
};

