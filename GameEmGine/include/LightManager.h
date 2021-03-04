#pragma once
#include <vector>
//#include "ResourceManager.h"
#include "Transformer.h"
#include "Model.h"
#include "Camera.h"



struct Light:public Transformer
{
	enum TYPE
	{
		NONE,
		DEFAULT,
		POINT,
		DIRECTIONAL,
		SPOTLIGHT
	};

	Light():Transformer("LIGHT") {}
	Light(Light& lit):Transformer(lit, "LIGHT")
	{
		*this = lit;
	}

	void setLightType(Light::TYPE type);

	void setAmbient(ColourRGBA diff);

	void setDiffuse(ColourRGBA diff);

	void setSpecular(ColourRGBA spec);

	void setRamp(Texture2D*);

	void setSpecularExponent(float specEx);

	void setAttenuationConstant(float attenConst);

	void setAttenuationLinear(float attenLinear);

	void setAttenuationQuadratic(float attenQuad);

	void setAngleConstraint(float angle);

	void enableLight(bool enable);
	void enableAmbiant(bool enable);
	void enableDiffuse(bool enable);
	void enableSpecular(bool enable);

	//private: //maybe later 
	Light::TYPE type = DEFAULT;


	ColourRGBA ambient, diffuse, specular;
	float
		angleConstraint = 45,
		specularExponent = 25,
		attenuationConst = 1.f,
		attenuationLinear = 0.1f,
		attenuationQuadratic = .01f;

	bool lightEnable = true;
	bool shadowEnable = true;
	bool ambiantEnable = true;
	bool diffuseEnable = true;
	bool specularEnable = true;
	bool rampActiveDiff = false;
	bool rampActiveSpec = false;
private:
	Texture2D* m_ramp;
};

class LightManager
{
public:

	static void addLight(Light* lit);
	static void removeLight(Light* lit);
	static void removeLight(unsigned index);

	static Light* getLight(unsigned index);

	static void setCamera(Camera* cam);

	static void setShader(Shader* shad);


	static void setFramebuffer(FrameBuffer* buff);

	static void shadowRender(unsigned w, unsigned h, FrameBuffer* to,GLuint positionID, std::unordered_map<void*, Model*>&);
	//Updates and renders each light
	static void update();
	static void clear();
private:


};

