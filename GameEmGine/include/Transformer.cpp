#include "Transformer.h"


Transformer::Transformer():
	m_localTranslate(1), m_localRotate(1),
	m_localScale(1), m_worldTranslate(1),
	m_worldRotate(1), m_worldScale(1),
	m_fps(false),
	Component("TRANSFORMER")
{}

Transformer::Transformer(Transformer& ref, COMP_TYPE type):Component(type)
{
	*this = ref;
}

Transformer::Transformer(COMP_TYPE type) :
	m_localTranslate(1), m_localRotate(1),
	m_localScale(1), m_worldTranslate(1),
	m_worldRotate(1), m_worldScale(1),
	m_fps(false),
	Component(type)
{}

Transformer::~Transformer()
{}

void Transformer::reset()
{
	m_localTranslate = m_localRotate = m_localScale = glm::mat4(1);
	m_posDat = m_rotDat = {0,0,0};
	m_scaleDat = {1,1,1};

	m_forward = {0,0,1};
	m_up = {0,1,0};
	m_right = {1,0,0};
}

void Transformer::enableFPSMode(bool enable)
{
	m_fps = enable;
}

void Transformer::rotate(Vec3 angles)
{
	m_updatedRot = true;
	m_localRotate = glm::mat4(1);

	if(angles.z)
		m_localRotate *= Quat::quatRotationMat(-angles.z, Vec3{0, 0, 1});

	if(angles.y)
		m_localRotate *= Quat::quatRotationMat(-angles.y, Vec3{0, 1, 0});

	if(angles.x)
		m_localRotate *= Quat::quatRotationMat(angles.x, Vec3{1, 0, 0});


	m_forward = m_localRotate * glm::vec4(0, 0, 1, 1);
	m_right = m_localRotate * glm::vec4(1, 0, 0, 1);
	m_up = m_localRotate * glm::vec4(0, 1, 0, 1);

	m_forward.normalize();
	m_up.normalize();
	m_right.normalize();
	m_rotDat = angles;

	m_rotateBy = false;
}

void Transformer::rotate(float x, float y, float z)
{
	Transformer::rotate({x,y,z});
}

void Transformer::rotateBy(Vec3 angles)
{
	m_rotateBy = true;
	Transformer::rotate(m_rotDat + angles);
}

void Transformer::rotateBy(float x, float y, float z)
{
	Transformer::rotateBy({x,y,z});
}

void Transformer::translateBy(Vec3 pos)
{
	m_updatedTrans = true;

	auto forward = m_forward;
	auto up = m_up;
	auto right = m_right;

	if(!m_fps)
		forward = {0,0,1},
		up = {0,1,0},
		right = {1,0,0};
	Coord3D tmpPos = m_posDat;

	m_posDat += (forward * -pos.z) + (up * pos.y) + (right * pos.x);
	m_localTranslate = glm::translate(glm::mat4(1), reclass(glm::vec3, m_posDat));
}

void Transformer::translateBy(float x, float y, float z)
{
	Transformer::translateBy({x, y, z});
}

void Transformer::translate(float x, float y, float z)
{
	Transformer::translate(Vec3{x, y, z});
}

void Transformer::translate(Vec3 pos)
{
	m_updatedTrans = true;

	auto forward = m_forward;
	auto up = m_up;
	auto right = m_right;

	if(!m_fps)
		forward = {0,0,1},
		up = {0,1,0},
		right = {1,0,0};
	
	m_posDat = pos.x * right + pos.y * up + -pos.z * forward;
	m_localTranslate = glm::translate(glm::mat4(1), reclass(glm::vec3, m_posDat));
}

void Transformer::scaleBy(float scale)
{
	Transformer::scaleBy(m_scaleDat.x * scale, m_scaleDat.y * scale, m_scaleDat.z * scale);
}

void Transformer::scaleBy(float x, float y, float z)
{
	Transformer::scale(m_scaleDat * Vec3(x, y, z));
}

void Transformer::scale(float s)
{
	Transformer::scale(s, s, s);
}

void Transformer::scale(Vec3 s)
{
	Transformer::scale(s.x, s.y, s.z);
}

void Transformer::scale(float x, float y, float z)
{
	m_updatedScale = true;

	auto forward = m_forward;
	auto up = m_up;
	auto right = m_right;

	if(!m_fps)
		forward = {0,0,1},
		up = {0,1,0},
		right = {1,0,0};

	m_scaleDat = x * right + y * up + z * forward;
	m_localScale = glm::scale(glm::mat4(1), reclass(glm::vec3, m_scaleDat));
}

Vec3 Transformer::getLocalPosition()
{
	Coord3D <>tmpPos = m_posDat;
	tmpPos.z *= -1;
	return tmpPos;
}

Vec3 Transformer::getLocalRotation()
{
	return m_rotDat;
}

Vec3 Transformer::getScale()
{
	return m_scaleDat;
}

Vec3 Transformer::getForward()
{

	return m_forward;
}

Vec3 Transformer::getUp()
{
	return m_up;
}

Vec3 Transformer::getRight()
{
	return m_right;
}

const glm::mat4& Transformer::getLocalRotationMatrix()
{
	return m_localRotate;
}

const glm::mat4& Transformer::getLocalScaleMatrix()
{
	return m_localScale;
}

const glm::mat4& Transformer::getLocalTranslationMatrix()
{
	return m_localTranslate;
}

const glm::mat4& Transformer::getWorldRotationMatrix()
{
	calculateWorldRotationMatrix();
	return m_worldRotate;
}

const glm::mat4& Transformer::getWorldScaleMatrix()
{
	calculateWorldScaleMatrix();
	return m_worldScale;
}

const glm::mat4& Transformer::getWorldTranslationMatrix()
{
	calculateWorldTranslationMatrix();
	return m_worldTranslate;
}

void Transformer::calculateWorldRotationMatrix()
{
	for(auto& a : getChildren())
		if(dynamic_cast<Transformer*>(a))
			((Transformer*)a)->m_updatedRot = true;

	m_worldRotate = glm::mat4(1);
	static Transformer* parent; parent = (Transformer*)getParent();

	//	if(parent)
	while(parent)
	{
		if(dynamic_cast<Transformer*>(parent))
			m_worldRotate = parent->m_localRotate * m_worldRotate;
		parent = (Transformer*)parent->getParent();
	}
}

void Transformer::calculateWorldScaleMatrix()
{
	for(auto& a : getChildren())
		if(dynamic_cast<Transformer*>(a))
			((Transformer*)a)->m_updatedScale = true;

	m_worldScale = glm::mat4(1);
	static Transformer* parent; parent = (Transformer*)getParent();

	//if(parent)
	while(parent)
	{
		if(dynamic_cast<Transformer*>(parent))
			m_worldScale = parent->m_localScale * m_worldScale;
		parent = (Transformer*)parent->getParent();
	}
}

void Transformer::calculateWorldTranslationMatrix()
{
	for(auto& a : getChildren())
		if(dynamic_cast<Transformer*>(a))
			((Transformer*)a)->m_updatedTrans = true;

	m_worldTranslate = glm::mat4(1);
	static Transformer* parent; parent = (Transformer*)getParent();

	//if(m_parent)
	while(parent)
	{
		if(dynamic_cast<Transformer*>(getParent()))
			m_worldTranslate = parent->m_localTranslate * m_worldTranslate;
		parent = (Transformer*)parent->getParent();
	}
}

glm::mat4 Transformer::getLocalTransformation()
{
	return getLocalTranslationMatrix() * getLocalRotationMatrix() * getLocalScaleMatrix();
}

glm::mat4 Transformer::getWorldTransformation()
{
	return getWorldTranslationMatrix() * getWorldRotationMatrix() * getWorldScaleMatrix();
}

void Transformer::resetUpdated()
{
	m_updatedRot = m_updatedTrans
		= m_updatedScale = false;
}

bool Transformer::isUpdated()
{
	return m_updatedRot || m_updatedTrans || m_updatedScale;
}

bool Transformer::isScaleUpdated()
{
	return m_updatedScale;
}

bool Transformer::isRotationUpdated()
{
	return m_updatedRot;
}

bool Transformer::isTranslatinUpdated()
{
	return m_updatedTrans;
}