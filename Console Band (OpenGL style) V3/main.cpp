//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <GameEmGine.h>
#include <memory>
#include <cmath>
#include "GameObjects.h"
#include "Song.h"
#include "Menu.h"
#include "BeatMapReader.h"

using std::vector;
using std::shared_ptr;

static std::string lutPath = "textures/hot.cube";

class ObjectSpawner
{
	vector<shared_ptr<Model>> m_objects;
	Model* m_obj;
	uint m_amount;
	Vec3 m_spawnRange;
	float m_minScale, m_maxScale;
public:
	ObjectSpawner() = default;
	ObjectSpawner(Model* obj, uint amount, Vec3 spawnRange, float minScale = 1, float maxScale = 1):
		m_obj(obj), m_amount(amount), m_spawnRange(spawnRange),
		m_minScale(minScale), m_maxScale(maxScale)
	{}

	void spawn()
	{
		//	m_objects.clear();
		for(int a = 0; a < m_amount; ++a)
		{

			Vec3 location =
				(Vec3(std::rand(), std::rand(), std::rand()) % m_spawnRange +
				 Vec3(std::rand() % 1000 * .001f, std::rand() % 1000 * .001f, std::rand() % 1000 * .001f)) *
				Vec3(std::rand() % 2 ? -1 : 1, std::rand() % 2 ? -1 : 1, std::rand() % 2 ? -1 : 1) *
				Vec3(bool(m_spawnRange.x), bool(m_spawnRange.y), bool(m_spawnRange.z));

			float scale =
				std::min(m_maxScale,
						 (m_minScale + std::rand() % int(m_maxScale - m_minScale)) +
						 (std::rand() % 1000 * .001f)
				);

			auto tmp = shared_ptr<Model>(new Model(*m_obj, "spawned obj"));

			tmp->scale(scale);
			tmp->translate(location);
			m_objects.push_back(tmp);
			Game::addModel(tmp.get());
		}
	}
};

class Test: public Scene
{
	enum Switches
	{
		DefaultScene = 0,
		AmbiantLight,
		SpecularLight,
		AmbiSpecLight,
		DoF,

	};

#pragma region Variables

	float speed = 20, angle = 1, bloomThresh = 0.1f;
	Animation ani;

	Switches toggle = DefaultScene;

	Model models[10];
	Transformer trans[10];
	Model bigBoss[2];
	Model rocket;

	float focusPlane = 10, focalLength = 5, aperture = 1;

	Text testText;
	Light lit;
	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown, tiltLeft, tiltRight,
		tab = false, lutActive = false, enableBloom = false, pause = false;
	Shader
		* m_lutNGrayscaleShader, * m_bloomHighPass,
		* m_blurHorizontal, * m_blurVertical,
		* m_blurrComposite, * m_sobel;
	FrameBuffer
		* m_buffer1, * m_buffer2,
		* m_greyscaleBuffer, * m_outline;
#pragma endregion

public:
	int blurPasses = 2;

	void cameraReset()
	{
		Game::getMainCamera()->reset();
		Game::translateCamera({0,5,-15});
		Game::getMainCamera()->enableFPSMode();
		focusPlane = 10, focalLength = 5, aperture = 1;
	}

	void init()
	{
		// TODO: change shader names


	#pragma region Init Setup

		Game::setBackgroundColour(.15f, .15f, .15f);
		FrustumPeramiters frustum{65,(float)Game::getWindowWidth() / Game::getWindowHeight(),0.001f,500};
		Game::setCameraType(&frustum);

		cameraReset();

		setSkyBox("Skyboxes/space/");
		enableSkyBox(true);
	#pragma endregion

	#pragma region Init Shaders & Framebuffers 

		m_bloomHighPass = ResourceManager::getShader("Shaders/Passthrough.vtsh", "Shaders/BloomHighPass.fmsh");
		m_blurHorizontal = ResourceManager::getShader("Shaders/Passthrough.vtsh", "Shaders/BlurHorizontal.fmsh");
		m_blurVertical = ResourceManager::getShader("Shaders/Passthrough.vtsh", "Shaders/BlurVertical.fmsh");
		m_blurrComposite = ResourceManager::getShader("Shaders/Passthrough.vtsh", "Shaders/BloomComposite.fmsh");

		m_lutNGrayscaleShader = ResourceManager::getShader("Shaders/Passthrough.vtsh", "Shaders/GrayscalePost.fmsh");
		m_sobel = ResourceManager::getShader("Shaders/Passthrough.vtsh", "shaders/Sobel.fmsh");


		m_greyscaleBuffer = new FrameBuffer(1, "Greyscale");
		m_buffer1 = new FrameBuffer(1, "Test1");
		m_buffer2 = new FrameBuffer(1, "Test2");
		m_outline = new FrameBuffer(1, "Sobel Outline");


		m_greyscaleBuffer->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_greyscaleBuffer->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}

		m_buffer1->initColourTexture(0, Game::getWindowWidth() / blurPasses, Game::getWindowHeight() / blurPasses, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_buffer1->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}

		m_buffer2->initColourTexture(0, Game::getWindowWidth() / blurPasses, Game::getWindowHeight() / blurPasses, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_buffer2->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}

		m_outline->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);
		if(!m_outline->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
	#pragma endregion


		//Create post effects
		customPostEffects =
			[&](FrameBuffer* gbuff, FrameBuffer* postBuff, float dt)->void
		{
			m_buffer1->clear();
			m_buffer2->clear();

			static float timer = 0;
			Shader* CoC = ResourceManager::getShader("Shaders/Passthrough.vtsh", "shaders/CoC.fmsh");
			postBuff->setViewport(0, 0, 0);

			switch(toggle)
			{
			case Switches::DefaultScene:
				break;
			case Switches::DoF:

			#pragma region Scene Blur  

				m_buffer1->setViewport(0, 0, 0);
				postBuff->copySingleColourToBuffer(m_buffer1->getColourWidth(0), m_buffer1->getColourHeight(0), m_buffer1);

				for(int a = 0; a < blurPasses; a++)
				{
					m_buffer1->enable();
					m_blurHorizontal->enable();
					m_blurHorizontal->sendUniform("uTex", 0);
					m_blurHorizontal->sendUniform("uPixleSize", 1.0f / m_buffer1->getColourWidth(0));
					m_buffer1->getColorTexture(0).bindTexture(0);
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurHorizontal->disable();


					m_buffer1->enable();
					m_blurVertical->enable();
					m_blurVertical->sendUniform("uTex", 0);
					m_blurVertical->sendUniform("uPixleSize", 1.0f / m_buffer1->getColourHeight(0));
					m_buffer1->getColorTexture(0).bindTexture(0);
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurVertical->disable();
				}

				FrameBuffer::disable();//return to base frame buffer 
			#pragma endregion 

			#pragma region Bokah
				//haha psych  
			#pragma endregion

			#pragma region DoF
				postBuff->setViewport(0, 0, 0);
				postBuff->enable();
				CoC->enable();

				CoC->sendUniform("uFragPos", 0);
				CoC->sendUniform("uFragDepth", 1);
				CoC->sendUniform("uScene", 2);
				CoC->sendUniform("uBlurScene", 3);

				gbuff->getColorTexture(0).bindTexture(0);
				Texture2D::bindTexture(1, gbuff->getDepthHandle());
				postBuff->getColorTexture(0).bindTexture(2);
				m_buffer1->getColorTexture(0).bindTexture(3);

				CoC->sendUniform("uCamPos", Game::getMainCamera()->getLocalPosition());
				CoC->sendUniform("uCamForw", Game::getMainCamera()->getForward());
				CoC->sendUniform("aperture", aperture);
				CoC->sendUniform("focalLength", focalLength);
				CoC->sendUniform("planeInFocus", focusPlane);

				FrameBuffer::drawFullScreenQuad();


				//unbind used Textures
				for(int a = 4 - 1; a >= 0; --a)
					Texture2D::unbindTexture(a);

				CoC->disable();
				postBuff->disable();
			#pragma endregion

			#pragma region Text Display
				static Text txtFocalLength, txtFocusPlane, txtAperture;
				static OrthoPeramiters ortho{0,(float)Game::getWindowWidth(),(float)Game::getWindowHeight(),0,0,500};
				static Camera cam(&ortho);

				if(((OrthoPeramiters*)cam.getProjectionData())->getBounds() !=
				   Vec2{(float)Game::getWindowWidth(), (float)Game::getWindowHeight()})
				{
					OrthoPeramiters orthonew{0,(float)Game::getWindowWidth(),(float)Game::getWindowHeight(),0,0,500};
					cam.setType(&orthonew);
				}

				cam.update();

				char str[50];
				sprintf_s(str, "Aperture= %.2f", aperture);
				txtAperture.setColour(1, 0, 0);
				txtAperture.setText(str);
				txtAperture.textSize(35);
				txtAperture.rotate(180, 0, 0);
				txtAperture.translate(0, Game::getWindowHeight() * .5f - txtAperture.getHeight() * 1.25f, 0);

				sprintf_s(str, "Focus Plane= %.2f", focusPlane);
				txtFocusPlane.setColour(1, 0, 0);
				txtFocusPlane.setText(str);
				txtFocusPlane.textSize(35);
				txtFocusPlane.rotate(180, 0, 0);
				txtFocusPlane.translate(0, Game::getWindowHeight() * .5f - txtFocusPlane.getHeight() * .5f, 0);

				sprintf_s(str, "Focal Length= %.2f", focalLength);
				txtFocalLength.setColour(1, 0, 0);
				txtFocalLength.setText(str);
				txtFocalLength.textSize(35);
				txtFocalLength.rotate(180, 0, 0);
				txtFocalLength.translate(0, Game::getWindowHeight() * .5f + txtFocalLength.getHeight() * .5f, 0);

				static std::unordered_map<void*, Model*> tmp;
				tmp[&txtFocalLength] = (Model*)&txtFocalLength;
				tmp[&txtFocusPlane] = (Model*)&txtFocusPlane;
				tmp[&txtAperture] = (Model*)&txtAperture;

				postBuff->enable();
				glClearDepth(1.f);
				glClear(GL_DEPTH_BUFFER_BIT);
				cam.render(nullptr, tmp, true);
				postBuff->disable();

			#pragma endregion
				break;
			}

		};


	#pragma region Scene Setup

		//models[0].create("models/solar system/Sun/Sun.obj", "Sun");
		//models[1].create("models/solar system/Mercury/Mercury.obj", "Mercury");
		//models[2].create("models/solar system/Venus/Venus.obj", "Venus");
		//models[3].create("models/solar system/Earth/Earth.obj", "Earth");
		//models[4].create("models/solar system/Mars/Mars.obj", "Mars");
		//models[5].create("models/solar system/Jupiter/Jupiter.obj", "Jupiter");
		//models[6].create("models/solar system/Saturn/Saturn.obj", "Saturn");
		//models[7].create("models/solar system/Uranus/Uranus.obj", "Uranus");
		//models[8].create("models/solar system/Neptune/Neptune.obj", "Neptune");
		//
		//for(int a = 0; a < 9; ++a)
		//{
		//	if(a)
		//		models[a].setParent(&trans[a]);
		//
		//	Game::addModel(&models[a]);
		//	models[a].setScale(.1f);
		//	models[a].translateBy({25.f * a,0,0});
		//}
		//models[1].scaleBy(.2f);
		//models[2].scaleBy(.2f);
		//models[3].scaleBy(.2f);
		//models[4].scaleBy(.2f);
		//models[5].scaleBy(.8f);
		//models[6].scaleBy(.7f);
		//models[7].scaleBy(.5f);
		//models[8].scaleBy(.4f);
		//
		////(161.874771, 74.611961, 82.858345)
		//Game::setCameraPosition({161.874771f, 74.611961f, -82.858345f});
		//Game::getMainCamera()->enableFPSMode();


		rocket.create("Models/rocket-ship/rocket ship.obj", "ship");
		Game::addModel(&rocket);

		models[1].create(new PrimitivePlane(Vec3{50, 0, 50}*5), "moon");
		models[1].replaceTexture(0, 0, ResourceManager::getTexture2D("Textures/moon.jpg").id);
		Game::addModel(&models[1]);

		models[2].create(new PrimitiveCube({10,10,10}/*, 10, 10*/), "trans Box");
		models[2].setColour(0, .5f, 0, .75f);
		models[2].translate(5, 10, 3);
		models[2].rotate(40, 106, 33);
		//models[2].setTransparent(true);
		Game::addModel(&models[2]);


		lit.setLightType(Light::TYPE::DIRECTIONAL);
		//lit.setParent(Game::getMainCamera());
		lit.setAmbient({120,165,120});
		lit.setSpecular({0,0,255});
		LightManager::addLight(&lit);
		LightManager::enableShadows(false);

		Model sphere(new PrimitiveSphere({1,1}, 20, 20, {0,.5,0}));
		//sphere.setColour(0, 1, 0);
		sphere.replaceTexture(0, 0, ResourceManager::getTexture2D("Textures/moon.jpg").id);
		static ObjectSpawner thestuff(&sphere, 20, {125,0,125}, 3, 5);

		thestuff.spawn();

		//Game::addModel(&sphere);

		//static Light tester;
		//tester.setLightType(Light::TYPE::DIRECTIONAL);
		//tester.rotate({45,-90,0});
		//LightManager::addLight(&tester);

	#pragma endregion

		//Key binds
		keyPressed =
			[&](int key, int mod)->void
		{

			if(key == 'R')
				cameraReset();

			static bool sky = true, frame = false;

			if(key == 'N')
				rocket.setWireframe(frame = !frame);

			if(key == GLFW_KEY_SPACE)
				pause = !pause;
			//enableSkyBox(sky = !sky);

			if(key == GLFW_KEY_F5)
				Shader::refresh();

			//static int count;
			if(key == GLFW_KEY_TAB)
				tab = !tab;//	std::swap(model[0], model[count++]);

			for(int a = 0; a < 5; ++a)
				if(key == GLFW_KEY_1 + a)
					toggle = (Switches)a;


			if(key == GLFW_KEY_1)
			{
				lit.enableAmbiant(false);
				lit.enableSpecular(false);
				lit.enableDiffuse(false);
			}
			else if(key == GLFW_KEY_2)
			{
				lit.enableAmbiant(true);
				lit.enableSpecular(false);
				lit.enableDiffuse(false);
			}
			else if(key == GLFW_KEY_3)
			{
				lit.enableAmbiant(false);
				lit.enableSpecular(true);
				lit.enableDiffuse(false);
			}
			else if(key == GLFW_KEY_4)
			{
				lit.enableAmbiant(true);
				lit.enableSpecular(true);
				lit.enableDiffuse(false);
			}
			else if(key == GLFW_KEY_5)
			{
				lit.enableAmbiant(true);
				lit.enableSpecular(true);
				lit.enableDiffuse(false);
			}
			else if(key == GLFW_KEY_6)
			{
				static bool enable = true;
				auto& list = Game::getObjectList();

				for(auto& a : list)
					if(a.second->getCompType() == "MODEL")
						a.second->enableTexture(!enable);
				enable = !enable;
			}


			static bool fps = 0;
			if(key == 'F')
				rocket.enableFPSMode(fps = !fps);

			if(key == 'A')
				moveLeft = true;

			if(key == 'D')
				moveRight = true;

			if(key == 'W')
				moveForward = true;

			if(key == 'S')
				moveBack = true;

			if(key == 'Q')
				moveDown = true;

			if(key == 'E')
				moveUp = true;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = true;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = true;

			if(key == GLFW_KEY_LEFT)
				rotLeft = true;

			if(key == GLFW_KEY_RIGHT)
				rotRight = true;

			if(key == GLFW_KEY_UP)
				rotUp = true;

			if(key == GLFW_KEY_DOWN)
				rotDown = true;
		};

		keyReleased =
			[&](int key, int mod)->void
		{
			if(key == 'A')
				moveLeft = false;

			if(key == 'D')
				moveRight = false;

			if(key == 'W')
				moveForward = false;

			if(key == 'S')
				moveBack = false;

			if(key == 'Q')
				moveDown = false;

			if(key == 'E')
				moveUp = false;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = false;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = false;

			if(key == GLFW_KEY_LEFT)
				rotLeft = false;

			if(key == GLFW_KEY_RIGHT)
				rotRight = false;

			if(key == GLFW_KEY_UP)
				rotUp = false;

			if(key == GLFW_KEY_DOWN)
				rotDown = false;

			//	puts(Game::getMainCamera()->getLocalPosition().toString());
		};

		//EmGineAudioPlayer::createAudioStream("songs/still alive.mp3");
		//EmGineAudioPlayer::getAudioControl()[0][0]->channel->set3DMinMaxDistance(20, 200);
		//
		//EmGineAudioPlayer::play(true);
	}

	void cameraMovement(float dt)
	{
		// Movement
		if(moveLeft)
			Game::translateCameraBy({-speed * dt,0,0});
		if(moveRight)
			Game::translateCameraBy({speed * dt,0,0});
		if(moveForward)
			Game::translateCameraBy({0,0,speed * dt});
		if(moveBack)
			Game::translateCameraBy({0,0,-speed * dt});
		if(moveUp)
			Game::translateCameraBy({0,speed * dt,0});
		if(moveDown)
			Game::translateCameraBy({0,-speed * dt,0});

		// Rotation
		if(tiltLeft)
			Game::rotateCameraBy({0,0,-angle});
		if(tiltRight)
			Game::rotateCameraBy({0,0,angle});
		if(rotLeft)
			Game::rotateCameraBy({0,-angle,0});
		if(rotRight)
			Game::rotateCameraBy({0,angle,0});
		if(rotUp)
			Game::rotateCameraBy({angle,0,0});
		if(rotDown)
			Game::rotateCameraBy({-angle,0,0});
	}

	void lightMovement(float dt)
	{
		// Movement
		if(moveLeft)
			lit.translateBy({-speed * dt,0.f,0.f});
		if(moveRight)
			lit.translateBy({speed * dt,0,0});
		if(moveForward)
			lit.translateBy({0,0,speed * dt});
		if(moveBack)
			lit.translateBy({0,0,-speed * dt});
		if(moveUp)
			lit.translateBy({0,speed * dt,0});
		if(moveDown)
			lit.translateBy({0,-speed * dt,0});

		// Rotation
		if(rotLeft)
			lit.rotateBy({0,-angle,0});
		if(rotRight)
			lit.rotateBy({0,angle,0});
		if(tiltLeft)
			lit.rotateBy({0,0,-angle});
		if(tiltRight)
			lit.rotateBy({0,0,angle});
		if(rotDown)
			lit.rotateBy({-angle,0,0});
		if(rotUp)
			lit.rotateBy({angle,0,0});


	}

	void DoFMovement(float dt)
	{
		if(moveForward)
			focusPlane += speed * dt * .5;
		if(moveBack)
			focusPlane += -speed * dt * .5;
		if(moveRight)
			focalLength += speed * dt * .5;
		if(moveLeft)
			focalLength += -speed * dt * .5;
		if(moveUp)
			aperture += speed * dt * .5;
		if(moveDown)
			aperture += -speed * dt * .5;

	}

	void update(double dt)
	{
		if(!tab)
			cameraMovement((float)dt);
		else
			DoFMovement((float)dt);

		float maxSpeed = 10;

		//if(!pause)
		//{
		//
		//	trans[1].rotateBy({0,maxSpeed * 1.0f * (float)dt,0});
		//	trans[2].rotateBy({0,maxSpeed * 0.9f * (float)dt,0});
		//	trans[3].rotateBy({0,maxSpeed * 0.8f * (float)dt,0});
		//	trans[4].rotateBy({0,maxSpeed * 0.7f * (float)dt,0});
		//	trans[5].rotateBy({0,maxSpeed * 0.6f * (float)dt,0});
		//	trans[6].rotateBy({0,maxSpeed * 0.5f * (float)dt,0});
		//	trans[7].rotateBy({0,maxSpeed * 0.4f * (float)dt,0});
		//	trans[8].rotateBy({0,maxSpeed * 0.3f * (float)dt,0});
		//	for(int a = 0; a < 9; ++a)
		//		models[a].rotateBy(0, (10 - a) * 5 * dt, 0);
		//}

		//auto tmpOBJPos = models[0].getLocalPosition();
		//EmGineAudioPlayer::getAudioControl()[0][0]->listener->pos = *(FMOD_VEC3*)&tmpOBJPos;
		//
		//auto tmpPos = Game::getMainCamera()->getLocalPosition();
		//auto tmpUp = Game::getMainCamera()->getUp();
		//auto tmpForward = Game::getMainCamera()->getForward();
		//EmGineAudioPlayer::getAudioSystem()->set3DListenerAttributes(0, (FMOD_VEC3*)&tmpPos, nullptr, (FMOD_VEC3*)&tmpForward, (FMOD_VEC3*)&tmpUp);
		//
		//EmGineAudioPlayer::update();


	}

	void onSceneExit() {}
};


int main()
{
	Game::init("Da Game", 1620, 780);
	Test
		test;

	//Song song;//just another scene... move along
	Game::setScene(&test);
	Game::run();

	return 0;
}