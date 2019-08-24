#include "Text.h"

float Text::m_initY;

Text::Text():Transformer(), m_vaoID(0), m_vboID(0)
{

	m_type = TEXT;
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

Text::Text(const char* font):Transformer(), m_vaoID(0), m_vboID(0)
{

	m_type = TEXT;
	m_font = font;

	m_texture = new FrameBuffer(1);
	m_texture->initColourTexture(0, 0, 0, GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);

}

Text::~Text()
{
	if(m_texture)
		delete m_texture;
}

void Text::setText(std::string text)
{
	m_text = text;
}

void Text::textSize(short s)
{
	setScale(s * .01f);
}

void Text::setColour(float r, float g, float b)
{
	m_colour = {GLubyte(r * 255),GLubyte(g * 255),GLubyte(b * 255)};
}

void Text::setColour(ColourRGBA colour)
{
	m_colour = colour;
}

unsigned int Text::size()
{
	return m_length;
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

	if(!m_vaoID || !m_vboID)
		renderInit();

	// Activate corresponding render state	
	s.enable();

	glUniformMatrix4fv(s.getUniformLocation("uModel"), 1, GL_FALSE, &((texture ? glm::mat4(1) : getTransformation())[0][0]));
	glUniformMatrix4fv(s.getUniformLocation("uView"), 1, GL_FALSE, &((cam->getObjectMatrix() * cam->getViewMatrix())[0][0]));
	glUniformMatrix4fv(s.getUniformLocation("uProj"), 1, GL_FALSE, &(cam->getProjectionMatrix()[0][0]));
	s.sendUniform("flip", texture);

	float colour[4]{(float)m_colour.r / 255,(float)m_colour.g / 255,(float)m_colour.b / 255,(float)m_colour.a / 255};

	s.sendUniform("textColor", colour[0], colour[1], colour[2], colour[3]);
	//glUniform3f(glGetUniformLocation(s.Program, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_vaoID);

	float
		xpos,
		ypos,

		w,
		h,
		x = texture ? 0 : getPosition().x,
		y = texture ? 0 : getPosition().y,
		aspect;
	Character ch;

	int winW, winH;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &winW, &winH);
	aspect = (float)winH;

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
			switch(a->getType())
			{
			case MODEL:
				reclass(Model*, a)->render(s, cam);
				break;
			case TEXT:
				reclass(Text*, a)->render(s, cam);
				break;
			}
}

void Text::toFramebufferTexture(unsigned int width)
{
	width;
	static Coord3D<> tmpSize;
	tmpSize = getScale();

	float
		ypos = 0,
		h = 0,
		x = 0;

	Character ch;
	for(auto& c : m_text)
	{
		ch = ResourceManager::getCharacter(c, m_font.c_str());

		ypos = std::min(-float(ch.size.y - ch.bearing.y), ypos);

		h = std::max((float)ch.size.y, h);


		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	if(width)
		setScale(width / x);


	x *= getScale().x;
	ypos *= getScale().x;
	h *= getScale().x;

	static Camera cam; cam.setType(ORTHOGRAPHIC, &OrthoPeramiters{0.f,(float)x,0.f,(float)(h - ypos),0.f,1.f});

	m_texture->clear();
	m_texture->resizeColour(0, (int)x, int(h - ypos), GL_RGBA8);

	glViewport(0, 0, (int)x, int(h - ypos));

	m_texture->enable();

	m_initY = h;
	render(*ResourceManager::getShader("shaders/freetype.vtsh", "shaders/freetype.fmsh"), &cam, true);

	m_texture->disable();
	setScale(tmpSize);
}

FrameBuffer* Text::getFramebuffer() { return m_texture; }

bool Text::isTransparent()
{
	return true;
}


