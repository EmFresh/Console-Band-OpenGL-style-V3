#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "Transformer.h"
#include "Utilities.h"
#include "Quat.h"
#include "Shader.h"
#include "Model.h"
#include "Text.h"

class Model;
struct ProjectionPeramiters;

class Camera:public Transformer
{
public:
	enum CAM_TYPE
	{
		NONE,
		ORTHOGRAPHIC,
		FRUSTUM
	};

	Camera(CAM_TYPE = FRUSTUM, Vec3 = {1,1,1});
	Camera(ProjectionPeramiters* peram, Vec3 = {1,1,1});
	~Camera() ;

	void init(Vec3 = {}, CAM_TYPE = FRUSTUM, ProjectionPeramiters* peram = nullptr);
	void setType(CAM_TYPE type, ProjectionPeramiters* peram = nullptr);
	void setType(ProjectionPeramiters* peram = nullptr);
	bool update();

	/*SET POSITION*/

	void translate(float x, float y, float z);
	void translate(Vec3 pos);
	void translateBy(float x, float y, float z);
	void translateBy(Vec3 pos);

	/*SET SCALE*/

	void scale(float scale);

	/*SET ROTATION*/

	void rotate(Vec3 angles);
	void rotate(float x, float y, float z);
	void rotateBy(Vec3 angles);
	void rotateBy(float x, float y, float z);

	bool cull(Model*);

	/*Matricies*/
	virtual const glm::mat4& getLocalRotationMatrix();
	virtual const glm::mat4& getLocalScaleMatrix();
	virtual const glm::mat4& getLocalTranslationMatrix();

	virtual const glm::mat4& getWorldRotationMatrix();
	virtual const glm::mat4& getWorldScaleMatrix();
	virtual const glm::mat4& getWorldTranslationMatrix();

	/*Gets a combination of the rotation, scale, and translation matricies*/

	virtual glm::mat4 getLocalTransformation();

	virtual glm::mat4 getWorldTransformation();

	//render objects
	void render(Shader* shader,const std::unordered_map<void*, Model*>& models,bool transparent = false,bool shadow=false);


	bool isUpdated() { return m_cameraUpdate; }

	/*GETTERS*/
	Vec3 getLocalRotation();

	glm::mat4& getProjectionMatrix();
	glm::mat4& getViewMatrix();
	glm::mat4& getCameraMatrix();

	CAM_TYPE getType();
	ProjectionPeramiters* getProjectionData();

protected:

	bool m_cameraUpdate,
		m_isTranslate, m_isTranslateBy,
		m_isRotate, m_isRotateBy;

	float m_scale;

	Vec3 m_size,
		m_position, m_positionBy,
		m_rotate, m_rotateBy,
		m_camRotation;

	CAM_TYPE m_type = FRUSTUM;

	glm::mat4
		m_cameraMat,
		m_projMat,
		m_viewMat;

	glm::mat4
		m_camLocalTranslate,
		m_camLocalRotate,
		m_camLocalScale,

		m_camWorldTranslate,
		m_camWorldRotate,
		m_camWorldScale;

	ProjectionPeramiters* m_projData;
private:
	/*REMOVED FROM TRANSFORMER*/

	void scaleBy(float scale) { scale; }
	void scaleBy(float x, float y, float z) { x, y, z; }
	void scale(Vec3 scale) { scale; }
	void scale(float x, float y, float z) { x, y, z; }

	bool isScaleUpdated() { return false; }
	bool isRotationUpdated() { return false; }
	bool isTranslatinUpdated() { return false; }


};

#pragma region Structs

struct ProjectionPeramiters
{
	Camera::CAM_TYPE type = Camera::CAM_TYPE::NONE;
	float zNear, zFar;
	ProjectionPeramiters(Camera::CAM_TYPE a_type = Camera::CAM_TYPE::NONE):type(a_type) {}
	ProjectionPeramiters(float znear, float zfar, Camera::CAM_TYPE a_type = Camera::CAM_TYPE::NONE):zNear(znear), zFar(zfar), type(a_type) {}
	virtual ~ProjectionPeramiters()=default;

	virtual void setNear(float znear) { zNear = znear; }
	virtual void setFar(float zfar) { zFar = zfar; }
	virtual void setZExtents(float znear, float zfar) { zNear = znear; zFar = zfar; }


};

struct OrthoPeramiters:public ProjectionPeramiters
{
	OrthoPeramiters(float  a_left, float a_right, float a_bottom, float a_top, float a_near, float a_far):
		ProjectionPeramiters(a_near, a_far, Camera::CAM_TYPE::ORTHOGRAPHIC),
		left(a_left), right(a_right), bottom(a_bottom), top(a_top)
	{}

	void setLeft(float a_left) { left = a_left; }
	void setRight(float a_right) { right = a_right; }
	void setBottom(float a_bottom) { bottom = a_bottom; }
	void setTop(float a_top) { top = a_top; }

	void setXAxis(float a_left, float a_right) { left = a_left; right = a_right; }
	void setYAxis(float a_top, float a_bottom) { top = a_top; bottom = a_bottom; }

	Vec2 getBounds() { return {abs(right-left),abs(top-bottom)}; }
	float  left = 0, right = 0, bottom = 0, top = 0;
};

struct FrustumPeramiters:public ProjectionPeramiters
{
	FrustumPeramiters(float a_angle, float a_aspect, float a_near, float a_far):
		ProjectionPeramiters(a_near, a_far, Camera::CAM_TYPE::FRUSTUM),
		angle(a_angle), aspect(a_aspect)
	{}

	void setAngle(float a_angle) { angle = a_angle; }
	void setAspect(float a_aspect) { aspect = a_aspect; }
	float angle=45, aspect=0;
};
#pragma endregion

