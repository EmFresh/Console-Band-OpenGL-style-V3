#include "FrameBuffer.h"
GLuint FrameBuffer::m_fsQuadVAO_ID, FrameBuffer::m_fsQuadVBO_ID;

FrameBuffer::FrameBuffer(unsigned numColorAttachments, std::string tag)
{
	m_tag = tag;
	glGenFramebuffers(1, &m_fboID);
	m_numColorAttachments = numColorAttachments;

	m_colorAttachments = new Texture2D[m_numColorAttachments];
	memset(m_colorAttachments, 0, sizeof(Texture2D) * numColorAttachments);
	m_depthAttachment = 0;

	//Buffs is required as a parameter for glDrawBuffers()
	m_buffs = new GLenum[m_numColorAttachments];
	for(unsigned i = 0; i < m_numColorAttachments; i++)
	{
		m_buffs[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	initFullScreenQuad();
}

FrameBuffer::~FrameBuffer()
{
	unload();
}

void FrameBuffer::initColourTexture(unsigned index, unsigned width, unsigned height, GLint internalFormat, GLint filter, GLint wrap)
{
	if(!m_colorAttachments[index].id)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
		//create colour texture
		glGenTextures(1, &m_colorAttachments[index].id);
		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

		resizeColour(index, width, height, internalFormat, filter, wrap);
	}
}

void FrameBuffer::initDepthTexture(unsigned width, unsigned height)
{
	if(!m_depthAttachment)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

		//create depth texture
		glGenTextures(1, &m_depthAttachment);

		resizeDepth(width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	}
}

void FrameBuffer::resizeColour(unsigned index, unsigned width, unsigned height, GLint internalFormat, GLint filter, GLint wrap)
{
	if(!(width && height) ||
	   (m_colorAttachments[index].width == (int)width &&
	   m_colorAttachments[index].height == (int)height))return;

	if(m_colorAttachments[index].id)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

		static GLuint tmpAttachment; tmpAttachment = m_colorAttachments[index].id;

		glGenTextures(1, &m_colorAttachments[index].id);

		m_internalFormat = internalFormat;
		m_filter = filter;
		m_wrap = wrap;
		m_colorAttachments[index].width = width;
		m_colorAttachments[index].height = height;

		glBindTexture(GL_TEXTURE_2D, m_colorAttachments[index].id);
		glTexStorage2D(GL_TEXTURE_2D, 1, m_internalFormat, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap);

		//Bind texture to the fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, m_colorAttachments[index].id, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tmpAttachment);
	}
	if(!checkFBO())
		puts("error resizing colour texture");
}

void FrameBuffer::resizeDepth(unsigned width, unsigned height)
{
	if(!(width && height))return;
	if(m_depthAttachment)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

		static GLuint tmpAttachment; tmpAttachment = m_depthAttachment;
		glGenTextures(1, &m_depthAttachment);

		m_width = width;
		m_height = height;
		glBindTexture(GL_TEXTURE_2D, m_depthAttachment);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//Bind texture to the fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tmpAttachment);
	}

	if(!checkFBO())
		puts("error resizing depth texture");
}

bool FrameBuffer::checkFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		unload();
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	return true;
}

// Clears all OpenGL memory
void FrameBuffer::unload()
{
	if(m_buffs != nullptr)
	{
		delete[] m_buffs;
		m_buffs = nullptr;
	}

	if(m_colorAttachments != nullptr)
	{
		for(unsigned i = 0; i < m_numColorAttachments; i++)
		{
			glDeleteTextures(1, &m_colorAttachments[i].id);
		}

		delete[] m_colorAttachments;
		m_colorAttachments = nullptr;
	}

	if(m_depthAttachment != GL_NONE)
	{
		glDeleteTextures(1, &m_depthAttachment);
		m_depthAttachment = GL_NONE;
	}

	m_numColorAttachments = 0;
}

void FrameBuffer::setClearColour(ColourRGBA col)
{
#define TO_BYTE_MULTI 0.0039215686274509803921568627451f
	setClearColour(col.r * TO_BYTE_MULTI, col.g * TO_BYTE_MULTI, col.b * TO_BYTE_MULTI, col.a * TO_BYTE_MULTI);
}

void FrameBuffer::setClearColour(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{

	glClearColor(r, g, b, a);//BG colour
}

// Clears all attached textures
void FrameBuffer::clear(GLbitfield clearBit)
{
	GLbitfield temp = 0;

	if(m_depthAttachment != GL_NONE)
	{
		temp |= GL_DEPTH_BUFFER_BIT;
	}

	if(m_colorAttachments != nullptr)
	{
		temp |= GL_COLOR_BUFFER_BIT;
	}

	enable();
	glClear(clearBit ? clearBit : temp);
	disable();
}


void FrameBuffer::clearBackBuffer(bool clearCol, bool clearDep)
{
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glClear((GL_COLOR_BUFFER_BIT * clearCol) | (GL_COLOR_BUFFER_BIT * clearDep));
}

void FrameBuffer::enable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
	glDrawBuffers(m_numColorAttachments, m_buffs);
}

void FrameBuffer::disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::setViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void FrameBuffer::setViewport(int x, int y, uint index)
{
	glViewport(x, y, getColourWidth(index), getColourHeight(index));
}

void FrameBuffer::copyColourToBackBuffer(int windowWidth, int windowHeight)
{
	copyColourToBuffer(windowWidth, windowHeight, GL_NONE);
}

void FrameBuffer::copyColourToBuffer(int windowWidth, int windowHeight, FrameBuffer* fbo)
{
	if(!m_numColorAttachments)
	{
		puts("This FrameBuffer has NO colour attachments");
		return;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo ? fbo->m_fboID : GL_NONE);

	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::copySingleColourToBackBuffer(int w, int h, uint from)
{
	copySingleColourToBuffer(w, h, nullptr, from, 0);
}

void FrameBuffer::copySingleColourToBuffer(int w, int h, FrameBuffer* fbo, uint from, uint to)
{
	if(!m_numColorAttachments)
	{
		puts("This FrameBuffer has NO colour attachments");
		return;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo ? fbo->m_fboID : GL_NONE);

	glReadBuffer(GL_COLOR_ATTACHMENT0 + from);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + to);

	glBlitFramebuffer(0, 0, m_colorAttachments[from].width, m_colorAttachments[from].height, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	//glReadBuffer(GL_NONE);
	//glDrawBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::copyDepthToBackBuffer(int windowWidth, int windowHeight)
{
	copyDepthToBuffer(windowWidth, windowHeight, GL_NONE);
}

void FrameBuffer::copyDepthToBuffer(int windowWidth, int windowHeight, GLuint fboID)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, windowWidth, windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::takeFromBackBufferColour(int windowWidth, int windowHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);

	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void FrameBuffer::takeFromBackBufferDepth(int windowWidth, int windowHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);

	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

GLuint FrameBuffer::getDepthHandle() const
{
	return m_depthAttachment;
}

GLuint FrameBuffer::getColourHandle(unsigned m_index) const
{
	return m_colorAttachments[m_index].id;
}
Texture2D& FrameBuffer::getColorTexture(unsigned m_index) const
{
	return m_colorAttachments[m_index];
}

void FrameBuffer::setPostProcess(std::function<void()>post, unsigned layer)
{
	m_postProcess = post;
	m_layer = layer;
}

std::function<void()> FrameBuffer::getPostProcess()
{
	return m_postProcess;
}

unsigned FrameBuffer::getNumColourAttachments()
{
	return m_numColorAttachments;
}

GLuint FrameBuffer::getFrameBufferID()
{
	return m_fboID;
}

void FrameBuffer::initFullScreenQuad()
{
	float vboData[] =
	{
		-1.0f,-1.0f,0.0f,
		 1.0f,-1.0f,0.0f,
		-1.0f, 1.0f,0.0f,

		 1.0f, 1.0f,0.0f,
		-1.0f, 1.0f,0.0f,
		 1.0f,-1.0f,0.0f,

		0.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,

		1.0f,1.0f,
		0.0f,1.0f,
		1.0f,0.0f
	};

	if(!m_fsQuadVAO_ID)
		glGenVertexArrays(1, &m_fsQuadVAO_ID);

	glBindVertexArray(m_fsQuadVAO_ID);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	if(!m_fsQuadVBO_ID)
		glGenBuffers(1, &m_fsQuadVBO_ID);

	glBindBuffer(GL_ARRAY_BUFFER, m_fsQuadVBO_ID);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 + sizeof(float) * 12, vboData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float) * 18));

	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindVertexArray(GL_NONE);
}

void FrameBuffer::drawFullScreenQuad()
{
	glBindVertexArray(m_fsQuadVAO_ID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(GL_NONE);

}

uint FrameBuffer::getColourWidth(int index)
{
	return m_colorAttachments[index].width;
}

uint FrameBuffer::getColourHeight(int index)
{
	return m_colorAttachments[index].height;
}

unsigned FrameBuffer::getDepthWidth()
{
	return m_width;
}

unsigned FrameBuffer::getDepthHeight()
{
	return m_height;
}

std::string FrameBuffer::getTag()
{
	return m_tag;
}

unsigned FrameBuffer::getLayer()
{
	return m_layer;
}
