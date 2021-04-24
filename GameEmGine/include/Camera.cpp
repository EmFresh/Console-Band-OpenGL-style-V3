#include "Camera.h"

Camera::Camera(CAM_TYPE type, Vec3 size)
	:Transformer("CAMERA"), m_scale(1), m_projMat(1), m_viewMat(1), m_cameraUpdate(true)
{
	//m_position = new Coord3D{-.25,-.5,0};
	init(size, type, nullptr);
}
Camera::Camera(ProjectionPeramiters* peram, Vec3 size)
	: Transformer("CAMERA"), m_scale(1), m_projMat(1), m_viewMat(1), m_cameraUpdate(true)
{
	init(size, peram ? peram->type : CAM_TYPE::NONE, peram);
}

Camera::~Camera()
{
	delete m_projData;
}

void Camera::init(Vec3 size, CAM_TYPE type, ProjectionPeramiters* peram)
{
	Component::m_type = "CAMERA";

	m_size = size;

	setType(type, peram);
}

void Camera::setType(CAM_TYPE type, ProjectionPeramiters* peram)
{
	if(peram)
	{
		if(m_projData)
			delete m_projData;
		m_projData = new ProjectionPeramiters(*peram);
		memset(m_projData, 0, sizeof(ProjectionPeramiters));
	}
	OrthoPeramiters* peram1 = reclass(OrthoPeramiters*, peram);
	FrustumPeramiters* peram2 = reclass(FrustumPeramiters*, peram);
	switch(m_type = type)
	{
	case ORTHOGRAPHIC:
		if(!peram)
			m_projMat = glm::ortho(-m_size.width * .5f, m_size.width * .5f,
								   -m_size.height * .5f, m_size.height * .5f, 0.f, m_size.depth);
		else
			m_projMat = glm::ortho(peram1->left, peram1->right,
								   peram1->bottom, peram1->top, peram1->zNear, peram1->zFar);

		break;
	case FRUSTUM:
		if(!peram)
			m_projMat = glm::perspective(glm::radians(45.f), m_size.width / m_size.height, .001f, m_size.depth);
		else
			m_projMat = glm::perspective(glm::radians(peram2->angle),
										 peram2->aspect ? peram2->aspect : m_size.width / m_size.height, peram2->zNear, peram2->zFar);
		break;
	default:
		m_projMat = glm::mat4(1);
	}
	m_cameraUpdate = true;
}

void Camera::setType(ProjectionPeramiters* peram)
{
	OrthoPeramiters* peram1 = reclass(OrthoPeramiters*, peram);
	FrustumPeramiters* peram2 = reclass(FrustumPeramiters*, peram);
	switch(peram->type)
	{
	case ORTHOGRAPHIC:
		if(!peram)
			m_projMat = glm::ortho(-m_size.width * .5f, m_size.width * .5f,
								   -m_size.height * .5f, m_size.height * .5f, 0.0f, m_size.depth);
		else
			m_projMat = glm::ortho(peram1->left, peram1->right,
								   peram1->bottom, peram1->top, peram1->zNear, peram1->zFar);

		break;
	case FRUSTUM:
		if(!peram)
			m_projMat = glm::perspective(glm::radians(45.f), m_size.width / m_size.height, .001f, m_size.depth);
		else
			m_projMat = glm::perspective(glm::radians(peram2->angle),
										 peram2->aspect ? peram2->aspect : m_size.width / m_size.height, peram2->zNear, peram2->zFar);
		break;
	default:
		m_projMat = glm::mat4(1);
	}
	m_cameraUpdate = true;
}

Camera::CAM_TYPE Camera::getType()
{
	return m_type;
}

ProjectionPeramiters* Camera::getProjectionData()
{
	return m_projData;
}

bool Camera::update()
{
	if(m_cameraUpdate)
	{
		if(m_isTranslate)
			Transformer::translate(m_position);
		m_position += m_positionBy;

		if(m_isTranslateBy)
			Transformer::translateBy(m_positionBy);

		if(m_isRotate)
			Transformer::rotate(m_rotate);
		m_rotate += m_isRotateBy;

		if(m_isRotateBy)
			Transformer::rotateBy(m_rotateBy);


		Transformer::setScale(m_scale);
		m_viewMat = m_worldTranslate * m_worldRotate * glm::inverse(m_localTranslate * m_localRotate);

		m_cameraMat = m_projMat * m_viewMat;

		m_rotateBy = m_positionBy = Vec3{0,0,0};


		m_isRotate = m_isRotateBy =
			m_isTranslate = m_isTranslateBy =
			m_cameraUpdate = false;

		m_camRotation = Transformer::getLocalRotation() * Vec3 { 1, -1, 1 };

		return true;
	}
	return false;
}

void Camera::translate(float x, float y, float z)
{
	translate({x,y,z});
}

void Camera::translate(Vec3 position)
{

	m_position = position;
	m_positionBy = {0,0,0};
	m_isTranslate = m_cameraUpdate = true;
}

void Camera::translateBy(float x, float y, float z)
{
	translateBy({x,y,z});
}

void  Camera::translateBy(Vec3 position)
{

	m_positionBy += position;
	m_isTranslateBy = m_cameraUpdate = true;
}

void Camera::setScale(const float scale)
{
	m_scale = scale;
	m_cameraUpdate = true;
}

void Camera::rotate(Vec3 angle)
{
	m_rotate = angle;
	m_rotateBy = {0,0,0};

	m_isRotate = m_cameraUpdate = true;
}

void Camera::rotate(float x, float y, float z)
{
	rotate({x,y,z});
}

void Camera::rotateBy(Vec3 angle)
{
	m_rotateBy += angle;
	m_isRotateBy = m_cameraUpdate = true;
}

void Camera::rotateBy(float x, float y, float z)
{
	rotateBy({x,y,z});
}

bool Camera::cull(Model* mod)
{
	mod->boundingBoxUpdate();
	auto center = mod->getCenter();
	auto dim = mod->getDimentions();


	glm::vec4 tmpTrans = glm::vec4(center.toVec3(), 1);

	tmpTrans = getCameraMatrix() * tmpTrans;
	tmpTrans /= tmpTrans.w;

	glm::vec4 tmpScale = glm::vec4(dim.toVec3(), 1);
	tmpScale = getProjectionMatrix() * tmpScale;
	tmpScale /= tmpScale.w;

	//ushort count=0;
	if(dim.x)
		if(abs(tmpTrans.x) >= 1 &&
		   (abs(tmpTrans.x) - abs(tmpScale.x)) >= 1)
			return true;
	if(dim.y)
		if(abs(tmpTrans.y) >= 1 &&
		   (abs(tmpTrans.y) - abs(tmpScale.y)) >= 1)
			return true;
	if(dim.z)
		if(abs(tmpTrans.z) >= 1 &&
		   (abs(tmpTrans.z) - abs(tmpScale.z)) >= 1)
			return true;


	return false;

}

const glm::mat4& Camera::getLocalRotationMatrix()
{
	return m_camLocalRotate = (Transformer::getLocalRotationMatrix());
}

const glm::mat4& Camera::getLocalScaleMatrix()
{
	return m_camLocalScale = (Transformer::getLocalScaleMatrix());
}

const glm::mat4& Camera::getLocalTranslationMatrix()
{
	return m_camLocalTranslate = (Transformer::getLocalTranslationMatrix());
}

const glm::mat4& Camera::getWorldRotationMatrix()
{
	return m_camWorldRotate = (Transformer::getWorldRotationMatrix());
}

const glm::mat4& Camera::getWorldScaleMatrix()
{
	return m_camWorldScale = (Transformer::getWorldScaleMatrix());
}

const glm::mat4& Camera::getWorldTranslationMatrix()
{
	return m_camWorldTranslate = (Transformer::getWorldTranslationMatrix());
}

glm::mat4 Camera::getLocalTransformation()
{
	return glm::inverse(Transformer::getLocalTransformation());
}

glm::mat4 Camera::getWorldTransformation()
{
	return glm::inverse(Transformer::getWorldTransformation());
}


#include <algorithm>
void Camera::render(Shader* shader, const std::unordered_map<void*, Model*>& models, bool trans, bool shadow)
{

	std::vector<std::pair<void*, Model*>> models2(models.begin(), models.end());
	Camera* tmpCam = this;
	std::sort(models2.begin(), models2.end(),
			  [tmpCam](std::pair<void*, Model*>a, std::pair<void*, Model*>b)->bool
	{
		return
			(a.second->getLocalPosition() - tmpCam->getLocalPosition()) >
			(b.second->getLocalPosition() - tmpCam->getLocalPosition());
	});

	if(shader)
	{
		shader->enable();
		shader->sendUniform("isTrans", trans);
		shader->disable();
	}

	Shader* shader2 = ResourceManager::getShader("shaders/freetype.vtsh", "shaders/freetype.fmsh");
	for(auto& a : models2)
		if(a.second->getCompType() == "TEXT")
		{
			Text* tmp = reclass(Text*, a.second);
			if(trans == tmp->isTransparent())
				tmp->render(*shader2, this);
		}
		else//if(a.second->getCompType() == "MODEL")
		{
			if(shadow)
				if(!a.second->isCastingShadow())continue;
			if(shader)
				if(!cull(a.second))
					if(trans == a.second->isTransparent())
						a.second->render(*shader, this);
		}

}

Vec3 Camera::getLocalRotation()
{
	return m_camRotation;
}

glm::mat4& Camera::getProjectionMatrix()
{
	return m_projMat;
}

glm::mat4& Camera::getViewMatrix()
{
	return m_viewMat;
}

glm::mat4& Camera::getCameraMatrix()
{
	return m_cameraMat;
}
