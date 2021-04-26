#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Component.h"
#include "Quat.h"
#include "Utilities.h"

class Transformer:public Component
{
public:

	Transformer();
	Transformer(Transformer&, COMP_TYPE type = "TRANSFORMER");
	Transformer(COMP_TYPE type);
	virtual ~Transformer();

	void reset();
	void enableFPSMode(bool enable = true);

	/*SET ROTATION*/

	virtual void rotate(Vec3 angles);
	virtual void rotate(float x, float y, float z);
	virtual void rotateBy(Vec3 angles);
	virtual void rotateBy(float x, float y, float z);

	/*SET POSITION*/

	virtual void translate(float x, float y, float z);
	virtual void translate(Vec3 pos);
	virtual void translateBy(float x, float y, float z);
	virtual void translateBy(Vec3 pos);


	/*SET SCALE*/

	virtual void scaleBy(float scale);
	virtual void scaleBy(float x, float y, float z);
	virtual void scale(Vec3 scale);
	virtual void scale(float scale);
	virtual void scale(float x, float y, float z);

	/*GETTERS*/
	virtual Vec3 getLocalPosition();
	virtual Vec3 getLocalRotation();
	virtual Vec3 getScale();
	Vec3 getForward();
	Vec3 getUp();
	Vec3 getRight();

	virtual const glm::mat4& getLocalRotationMatrix();
	virtual const glm::mat4& getLocalScaleMatrix();
	virtual const glm::mat4& getLocalTranslationMatrix();

	virtual const glm::mat4& getWorldRotationMatrix();
	virtual const glm::mat4& getWorldScaleMatrix();
	virtual const glm::mat4& getWorldTranslationMatrix();

	/*Gets a combination of the rotation, scale, and translation matricies*/

	virtual glm::mat4 getLocalTransformation();
	virtual glm::mat4 getWorldTransformation();

	virtual void resetUpdated();
	virtual bool isUpdated();
	virtual bool isScaleUpdated();
	virtual bool isRotationUpdated();
	virtual bool isTranslatinUpdated();



private:

	void calculateWorldRotationMatrix();
	void calculateWorldScaleMatrix();
	void calculateWorldTranslationMatrix();


	Vec3 m_forward = {0,0,1}, m_up = {0,1,0}, m_right = {1,0,0};
	Vec3 m_posDat, m_rotDat, m_scaleDat;
	
	bool  m_updatedRot = true,
		m_updatedTrans = true,
		m_updatedScale = true,
		//first person movement
		m_fps = false,
		m_rotateBy = false;

protected:
	glm::mat4
		m_localTranslate,
		m_localRotate,
		m_localScale,

		m_worldTranslate,
		m_worldRotate,
		m_worldScale;

};

