#pragma once

#include <vector>
#include "Shader.h"
#include "Camera.h"
#include "Transformer.h"
#include "ResourceManager.h"
#include "FrameBuffer.h"
//#include "Utilities.h"



class Camera;

class Text:public Transformer
{
public:
	Text();
	Text(cstring font);

	~Text();

	void setText(std::string text);

	void textSize(short s);

	void setColour(float r, float g, float b, float a = 1);

	void setColour(ColourRGBA colour);

	unsigned int size();

	float getWidth();

	float getHeight();

	void render(Shader& s, Camera* cam, bool texture = false);

	void toTexture(unsigned int width = 720);

	GLuint getTexture();

	bool isTransparent();
	//static Character loadCharacter(cstring c, cstring font);

private:
	void testSize();

	/*SET SCALE*/

	void scaleBy(float scale) { Transformer::scaleBy(scale); }
	void scaleBy(float x, float y, float z) { Transformer::scaleBy(x, y, z); }
	void scale(Vec3 scale) { Transformer::scale(scale); }
	void scale(float scale) { Transformer::scale(scale); }
	void scale(float x, float y, float z) { Transformer::scale(x, y, z); }

	void renderInit();

	FrameBuffer* m_texture;
	ColourRGBA m_colour;
	std::string m_font, m_text;
	unsigned int m_length;
	GLuint m_vaoID, m_vboID;
	Coord2D<float> m_size;

	float m_initY;
};

