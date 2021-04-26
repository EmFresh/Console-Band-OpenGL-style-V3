#pragma once

#include <GL/glew.h>
#include <functional>
#include <string>
#include "Shader.h"
#include "Texture.h"

class FrameBuffer
{
public:
	FrameBuffer() = delete;
	FrameBuffer(unsigned numColorAttachments, std::string tag = "");
	~FrameBuffer();

	void initColourTexture(unsigned index, unsigned width, unsigned height, GLint internalFormat = GL_RGBA8, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE);
	void initDepthTexture(unsigned width, unsigned height);
	void resizeColour(unsigned index, unsigned width, unsigned height, GLint internalFormat = GL_RGBA8, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE);
	void resizeDepth(unsigned width, unsigned height);

	bool checkFBO();

	// Clears all OpenGL memory
	void unload();


	/// <summary>
	/// Set Clear Colour for the next clear calls
	/// </summary>
	/// <param name="colour"></param>
	static void setClearColour(ColourRGBA colour = {});

	/// <summary>
	/// Set Clear Colour for the next clear calls. Values range 0 -> 1 (inclusive)
	/// </summary>
	/// <param name="r">red channel</param>
	/// <param name="g">green channel</param>
	/// <param name="b">blue channel</param>
	/// <param name="a">alpha channel</param>
	static void setClearColour(GLclampf r, GLclampf g, GLclampf b, GLclampf a);

	// Clears all attached textures
	void clear(GLbitfield = 0);

	static void clearBackBuffer(bool clearCol = true, bool clearDep = true);
	//binds objects to frame buffer/s
	void enable();

	//binds the frame buffer to the default location
	static void disable();


	///~ Helper Functions ~///


	void setViewport(int x, int y, int width, int height);
	void setViewport(int x, int y, uint colourIndex);

	void copyColourToBackBuffer(int windowWidth, int windowHeight);
	void copyColourToBuffer(int windowWidth, int windowHeight, FrameBuffer* fboID);

	void copySingleColourToBackBuffer(int windowWidth, int windowHeight, uint from = 0);
	void copySingleColourToBuffer(int windowWidth, int windowHeight, FrameBuffer* fboID, uint from=0, uint to=0);

	void copyDepthToBackBuffer(int windowWidth, int windowHeight);
	void copyDepthToBuffer(int windowWidth, int windowHeight, GLuint fboID);

	void takeFromBackBufferColour(int windowWidth, int windowHeight);
	void takeFromBackBufferDepth(int windowWidth, int windowHeight);

	GLuint getDepthHandle() const;
	GLuint getColourHandle(unsigned m_index) const;

	Texture2D& getColorTexture(unsigned m_index) const;

	void setPostProcess(std::function<void()>, unsigned layer = 0);
	std::function<void()> getPostProcess();

	uint getNumColourAttachments();

	GLuint getFrameBufferID();

	static void drawFullScreenQuad();

	uint getColourWidth(int index);
	uint getColourHeight(int index);
	uint getDepthWidth();
	uint getDepthHeight();

	std::string getTag();
	uint getLayer();

private:

	static void initFullScreenQuad();

	static GLuint m_fsQuadVAO_ID, m_fsQuadVBO_ID;

	GLuint
		m_layer = GL_NONE,
		m_fboID = GL_NONE,
		m_depthAttachment = GL_NONE;
	Texture2D* m_colorAttachments = nullptr;

	GLint m_internalFormat = GL_RGBA8,
		m_filter = GL_LINEAR,
		m_wrap = GL_CLAMP_TO_EDGE;

	uint m_width, m_height;

	GLenum* m_buffs = nullptr;

	uint m_numColorAttachments = 0;
	std::string m_tag;
	std::function<void()>m_postProcess;
	Shader* m_shader;
};