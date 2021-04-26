#include "Text.h"
#include <cmath>
Text::Text():Transformer(), m_vaoID(0), m_vboID(0)
{
	scale(1);

	m_type = "TEXT";
	m_font = "fonts/arial.ttf";

	//printf("%s\n", m_face->style_name);
	m_texture = new FrameBuffer(1);
	m_texture->initColourTexture(0, 1, 1, GL_RGBA8, GL_NEAREST, GL_CLAMP_TO_EDGE);
	if(!m_texture->checkFBO())
	{
		puts("FBO failed Creation");
		system("pause");
		return;
	}
}

Text::Text(cstring font):Transformer(), m_vaoID(0), m_vboID(0)
{
	scale(1);

	m_type = "TEXT";
	m_font = font;

	m_texture = new FrameBuffer(1);
	m_texture->initColourTexture(0, 0, 0, GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);

}

Text::~Text()
{
	if(m_texture)
		delete m_texture;
	m_texture = 0;
}

void Text::setText(std::string text)
{
	m_text = text;
	testSize();
}

void Text::textSize(short s)
{
	scale(s * 0.020834f);// s / 48 = s * 0.020834f 
	testSize();
}

void Text::setColour(float r, float g, float b, float a)
{
	m_colour = {GLubyte(r * 255),GLubyte(g * 255),GLubyte(b * 255),GLubyte(a * 255)};
}

void Text::setColour(ColourRGBA colour)
{
	m_colour = colour;
}

unsigned int Text::size()
{
	return m_length;
}

float Text::getWidth()
{
	return m_size.width;
}

float Text::getHeight()
{
	return m_size.height;
}



void Text::renderInit()
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);

	glBindVertexArray(m_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Text::render(Shader& s, Camera* cam, bool texture)
{
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!texture)
		getScale();

	if(!m_vaoID || !m_vboID)
		renderInit();

	// Activate corresponding render state	
	s.enable();

	s.sendUniform("uLocalModel", texture ? glm::mat4(1) : getLocalTransformation());
	s.sendUniform("uWorldModel", texture ? glm::mat4(1) : getWorldTransformation());

	glUniformMatrix4fv(s.getUniformLocation("uView"), 1, GL_FALSE, &(cam->getViewMatrix()[0][0]));
	glUniformMatrix4fv(s.getUniformLocation("uProj"), 1, GL_FALSE, &(cam->getProjectionMatrix()[0][0]));

	s.sendUniform("flip", texture);
	float colour[4]{(float)m_colour.r / 255,(float)m_colour.g / 255,(float)m_colour.b / 255,(float)m_colour.a / 255};
	s.sendUniform("textColor", colour[0], colour[1], colour[2], colour[3]);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_vaoID);

	float
		xpos,
		ypos,

		w,
		h,
		x = 0,
		y = 0;

	static Character ch;


	// Iterate through all characters
	for(auto& c : m_text)
	{
		ch = ResourceManager::getCharacter(c, m_font.c_str());

		xpos = (x + (float)ch.bearing.x * getScale().x);
		ypos = texture ?
			y + (((float(ch.size.y - ch.bearing.y)) * getScale().x) + ((m_initY - ch.size.y * getScale().x))) :
			y - (float(ch.size.y - ch.bearing.y) * getScale().x);

		w = (float)ch.size.x * getScale().x;
		h = (float)ch.size.y * getScale().x;

		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0f },//top left
			{ xpos,     ypos,       0.0, 1.0f },//bottom left
			{ xpos + w, ypos,       1.0, 1.0f },//bottom right

			{ xpos,     ypos + h,   0.0, 0.0f },//top left
			{ xpos + w, ypos,       1.0, 1.0f },//bottom right
			{ xpos + w, ypos + h,   1.0, 0.0f } //top right
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += ((ch.advance >> 6) * getScale().x); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//render child meshes
	if(!texture)
		for(auto& a : getChildren())
			if(a->getCompType() == "MODEL")
				reclass(Model*, a)->render(s, cam);
			else if(a->getCompType() == "TEXT")
				reclass(Text*, a)->render(s, cam);

}

void Text::toTexture(unsigned int width)
{
	Vec3 tmpSize = getScale();

	float
		ypos = 0,
		h = 0,
		x = 0;

	Character ch;
	for(auto& c : m_text)
	{
		ch = ResourceManager::getCharacter(c, m_font.c_str());

		ypos = fmin(-float(ch.size.y - ch.bearing.y), ypos);

		h = fmax((float)ch.size.y, h);


		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	if(width)
		scale(width / x);


	x *= getScale().x;
	ypos *= getScale().x;
	h *= getScale().x;

	static Camera cam;
	OrthoPeramiters perams{0.f,(float)x,0.f,(float)(h - ypos),0.f,1.f};
	cam.setType(Camera::ORTHOGRAPHIC, &perams);

	m_texture->clear();
	m_texture->resizeColour(0, (int)x, int(h - ypos), GL_RGBA8);

	int view[4];
	glGetIntegerv(GL_VIEWPORT, view);

	glViewport(0, 0, (int)x, int(h - ypos));

	m_texture->enable();

	m_initY = h;
	auto a = m_colour;
	setColour(1, 1, 1);
	render(*ResourceManager::getShader("shaders/freetype.vtsh", "shaders/freetype.fmsh"), &cam, true);
	setColour(a);
	m_texture->disable();
	scale(tmpSize);
	glViewport(view[0], view[1], view[2], view[3]);
}

GLuint Text::getTexture() { return m_texture->getColourHandle(0); }

bool Text::isTransparent()
{
	return true;
}

void Text::testSize()
{
	float
		ypos = 0,
		h = 0,
		x = 0;

	Character ch;
	for(auto& c : m_text)
	{
		ch = ResourceManager::getCharacter(c, m_font.c_str());

		ypos = fmin(-float(ch.size.y - ch.bearing.y), ypos);

		h = fmax((float)ch.size.y, h);


		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6) * getScale().x; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	m_size = {(x), ((h - ypos) * getScale().x)};
}


