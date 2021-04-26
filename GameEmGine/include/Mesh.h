#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include "Quat.h"
#include "Shader.h"
#include "Utilities.h"
#include "ResourceManager.h"
#include "Transformer.h"


#define CHAR_BUFF_SIZE 1000

enum PRIMITIVE_TYPE
{
	NONE,
	PLANE,
	CUBE,
	SPHERE
};

struct PrimitiveMesh:public Transformer
{
	PrimitiveMesh():Transformer("PRIMITIVE MESH") {}
	PrimitiveMesh(Vec3 dim, Vec3 offset = {}, Vec3 rot = {}):Transformer("PRIMITIVE MESH"),
		m_dim(dim)
	{
		translate(offset);
		rotate(rot);
	}
	PrimitiveMesh(float width, float height, float depth, Vec3 offset = {}, Vec3 rot = {}):Transformer("PRIMITIVE MESH"),
		m_dim({width,height,depth})
	{
		translate(offset);
		rotate(rot);
	}

	virtual ~PrimitiveMesh() {}

	virtual void setDimentions(Vec3 dim, Vec3 offset = {}, Vec3 rot = {})
	{
		m_dim = dim;
		translate(offset);
		rotate(rot);
	}
	virtual void setDimentions(float w, float h, float d = 0, Vec3 offset = {}, Vec3 rot = {})
	{
		setDimentions({w,h,d}, offset, rot);
	}
	virtual void createMesh() = 0;

	Vec3 getDimentions() { return m_dim; }

	PRIMITIVE_TYPE& getType() { return type; }

	std::vector<Vertex3D>& getData() { return m_unpackedData; };
	std::vector<unsigned>& getIndices() { return m_indices; };


	Vec3 m_top, m_bottom, m_left, m_right, m_front, m_back;

protected:
	Vec3 m_dim;

	PRIMITIVE_TYPE type = NONE;
	std::vector<Vertex3D> m_unpackedData;
	std::vector<unsigned> m_indices;

	//adds indices to the existing structure
	template<class _iter>
	void indiceAdder(_iter begin, _iter end, std::vector<Vertex3D> verts)
	{
		for(auto& i = begin; i < end; ++i)
		{
			std::vector<Vertex3D>::iterator tmp;
			if((tmp = std::find(m_unpackedData.begin(), m_unpackedData.end(), verts[*i])) != m_unpackedData.end())
			{
				m_indices.push_back(unsigned(tmp - m_unpackedData.begin()));
			}
			else
			{
				m_unpackedData.push_back(verts[*i]);
				m_indices.push_back(m_unpackedData.size() - 1);
			}
		}
	}

	//clears all mesh data before adding indices
	void indiceCreator(std::vector<Vertex3D> verts)
	{
		m_unpackedData.clear();
		m_indices.clear();

		for(auto i = verts.begin(); i < verts.end(); ++i)
		{
			std::vector<Vertex3D>::iterator tmp;
			if((tmp = std::find(m_unpackedData.begin(), m_unpackedData.end(), *i)) != m_unpackedData.end())
			{
				m_indices.push_back(unsigned(tmp - m_unpackedData.begin()));
			}
			else
			{
				m_unpackedData.push_back(*i);
				m_indices.push_back(m_unpackedData.size() - 1);
			}
		}
	}

	void calculateBounds()
	{
		//m_unpackedData[0].norm *= -1;
		m_top = m_bottom = m_left = m_right = m_front = m_back = m_unpackedData[0].coord;

		for(unsigned int a = 1; a < m_unpackedData.size(); a++)
		{
			//m_unpackedData[a].norm *= -1;
			m_top = m_unpackedData[a].coord.y > m_top.y ? m_unpackedData[a].coord : m_top;
			m_bottom = m_unpackedData[a].coord.y < m_bottom.y ? m_unpackedData[a].coord : m_bottom;
			m_left = m_unpackedData[a].coord.x < m_left.x ? m_unpackedData[a].coord : m_left;
			m_right = m_unpackedData[a].coord.x > m_right.x ? m_unpackedData[a].coord : m_right;
			m_front = m_unpackedData[a].coord.z < m_front.z ? m_unpackedData[a].coord : m_front;
			m_back = m_unpackedData[a].coord.z > m_back.z ? m_unpackedData[a].coord : m_back;
		}
	}
};

struct PrimitivePlane:public PrimitiveMesh
{
	PrimitivePlane():PrimitiveMesh() { type = PLANE; }

	PrimitivePlane(Vec3 dim, Vec3 offset = {}, Vec3 rot = {}):PrimitiveMesh(dim, offset, rot) { type = PLANE; createMesh(); }

	PrimitivePlane(float w, float h, float d = 0, Vec3 offset = {}, Vec3 rot = {})
		:PrimitiveMesh(w, h, d, offset, rot)
	{
		type = PLANE; createMesh();
	}

	~PrimitivePlane() {};

	void createMesh()
	{
		float
			halfW = m_dim.width * .5f,
			halfH = m_dim.height * .5f,
			halfD = m_dim.depth * .5f;

		//coords
		std::vector<Vertex3D> tmp;
		glm::vec4 holder{};
		//bottom left tri
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{-halfW,-halfH, halfD}).toVec3(), 1)
			)});//bottom left
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{ halfW,-halfH,-halfD}).toVec3(), 1)
			)});//bottom right*
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{-halfW, halfH,-halfD}).toVec3(), 1)
			)});//top left*

		  //top right tri
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{ halfW, halfH,-halfD}).toVec3(), 1)
			)});//top right
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{-halfW, halfH, halfD}).toVec3(), 1)
			)});//top left*
		tmp.push_back({getLocalPosition() + reclass(Vec3,holder = getLocalRotationMatrix() *
			glm::vec4((Vec3{ halfW,-halfH, halfD}).toVec3(), 1)
			)});//bottom right*


		m_top = m_bottom = m_left = m_right = m_front = m_back = tmp[0].coord;

		for(unsigned int a = 1; a < tmp.size(); a++)
		{
			m_top = tmp[a].coord.y > m_top.y ? tmp[a].coord : m_top;
			m_bottom = tmp[a].coord.y < m_bottom.y ? tmp[a].coord : m_bottom;
			m_left = tmp[a].coord.x < m_left.x ? tmp[a].coord : m_left;
			m_right = tmp[a].coord.x > m_right.x ? tmp[a].coord : m_right;
			m_front = tmp[a].coord.z < m_front.z ? tmp[a].coord : m_front;
			m_back = tmp[a].coord.z > m_back.z ? tmp[a].coord : m_back;
		}


		//normal
		Vec3 norm = Vec3::crossProduct(tmp[0].coord - tmp[1].coord, tmp[0].coord - tmp[2].coord).normal();

		tmp[0].norm = norm;
		tmp[1].norm = norm;
		tmp[2].norm = norm;

		tmp[3].norm = norm;
		tmp[4].norm = norm;
		tmp[5].norm = norm;

		Coord3D x, y;
		if(abs(norm) == Vec3{0, 0, 1})
		{
			x = {1,0,0};
			y = {0,1,0};
		}
		else if(abs(norm) == Vec3{0, 1, 0})
		{
			x = {1,0,0};
			y = {0,0,1};
		}
		else if(abs(norm) == Vec3{1, 0, 0})
		{
			x = {0,0,1};
			y = {0,1,0};
		}

		//uv
		Vec3 val;

		val = ((tmp[0].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[0].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//bottom left 
		val = ((tmp[1].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[1].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//bottom right*
		val = ((tmp[2].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[2].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//top left*

		val = ((tmp[3].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[3].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//top right
		val = ((tmp[4].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[4].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//top left*
		val = ((tmp[5].coord - getLocalPosition()) + Vec3{halfW, halfH, halfD});
		tmp[5].uv = {float((val * x).length() > 0),float((val * y).length() > 0)};//bottom right*


		indiceCreator(tmp);

		calculateBounds();
	}
};

struct PrimitiveCube: public PrimitiveMesh
{
public:
	PrimitiveCube():PrimitiveMesh() { type = CUBE; }

	PrimitiveCube(Vec3 dim, bool invert = false, Vec3 offset = {}, Vec3 rot = {}):PrimitiveMesh(dim, offset, rot)
	{
		type = CUBE; m_invert = invert; createMesh();
	}

	PrimitiveCube(float w, float h, float d = 0, bool invert = false, Vec3 offset = {}, Vec3 rot = {})
		:PrimitiveMesh(w, h, d, offset, rot)
	{
		type = CUBE; m_invert = invert; createMesh();
	}

	void setDimentions(Vec3 dim, bool invert = false, Vec3 offset = {}, Vec3 rot = {})
	{
		PrimitiveMesh::setDimentions(dim, offset, rot);
		m_invert = invert;
	}

	void setDimentions(float x, float y, float z, bool invert = false, Vec3 offset = {}, Vec3 rot = {})
	{
		setDimentions({x,y,z}, invert, offset, rot);
	}

	~PrimitiveCube() {}

	void createMesh()
	{
		static PrimitivePlane left, right, top, bottom, front, back;
		m_indices.clear();
		m_unpackedData.clear();

		float
			halfW = m_dim.width * .5f,
			halfH = m_dim.height * .5f,
			halfD = m_dim.depth * .5f;

		right.setDimentions({0, m_dim.height, m_dim.depth}, getLocalPosition() + Vec3{halfW, 0, 0}), left.setDimentions(0, m_dim.height, m_dim.depth, getLocalPosition() + Vec3{-halfW, 0, 0});
		top.setDimentions(m_dim.width, 0, m_dim.depth, getLocalPosition() + Vec3{0, halfH, 0}), bottom.setDimentions(m_dim.width, 0, m_dim.depth, getLocalPosition() + Vec3{0, -halfH, 0});
		front.setDimentions({m_dim.width, m_dim.height, 0}, getLocalPosition() + Vec3{0, 0, -halfD}), back.setDimentions({m_dim.width, m_dim.height, 0}, getLocalPosition() + Vec3{0, 0, halfD});

		front.createMesh(), back.createMesh(), left.createMesh(), right.createMesh(), top.createMesh(), bottom.createMesh();

		for(auto& a : left.getData())
			a.norm *= -1;
		for(auto& a : bottom.getData())
			a.norm *= -1;
		for(auto& a : front.getData())
			a.norm *= -1;

		indiceAdder(front.getIndices().rbegin(), front.getIndices().rend(), front.getData());
		indiceAdder(back.getIndices().begin(), back.getIndices().end(), back.getData());

		indiceAdder(left.getIndices().rbegin(), left.getIndices().rend(), left.getData());
		indiceAdder(right.getIndices().begin(), right.getIndices().end(), right.getData());

		indiceAdder(bottom.getIndices().rbegin(), bottom.getIndices().rend(), bottom.getData());
		indiceAdder(top.getIndices().begin(), top.getIndices().end(), top.getData());

		for(auto& a : m_unpackedData)
		{
			glm::mat4 rot = getLocalRotationMatrix();
			glm::vec4 holder;
			a.coord = reclass(Vec3, holder = rot * glm::vec4(a.coord.toVec3(), 1));
			a.norm = reclass(Vec3, holder = rot * glm::vec4(a.norm.toVec3(), 1));
		}

		if(m_invert)
			for(unsigned a = 0; a <unsigned(m_indices.size() * .5f); ++a)
				std::swap(m_indices[a], m_indices[m_indices.size() - 1 - a]);


		calculateBounds();
	}

private:
	bool m_invert;
};

struct PrimitiveSphere:public PrimitiveMesh
{
public:
	PrimitiveSphere():PrimitiveMesh() { type = SPHERE; }

	PrimitiveSphere(Coord2D<> dim, int segments, int divisions, Vec3 offset = {}, Vec3 rot = {}, bool invert = false):
		PrimitiveMesh(dim, offset, rot),
		m_segments(segments),
		m_divisions(divisions),
		m_invert(invert)
	{
		type = SPHERE; createMesh();
	}

	PrimitiveSphere(float w, float h, int segments, int divisions, Vec3 offset = {}, Vec3 rot = {}, bool invert = false):
		PrimitiveMesh(w, h, w, offset, rot),
		m_segments(segments),
		m_divisions(divisions),
		m_invert(invert)
	{
		type = SPHERE;  createMesh();
	}

	~PrimitiveSphere() {}

	void createMesh()
	{
		std::vector<Vertex3D>tmp;
		Quat tmp2;
		Vec3 norm;

		//calculate segments
		for(int b = 0; b <= m_divisions; ++b)
		{
			for(int a = 0; a <= m_segments; ++a)
			{
				//cap
				if(b == 0)
				{
					//bottom Coord
					tmp.push_back({{0,-m_dim.h * .5f * cos(0.f),0}});
					tmp.back().uv = {.5f,0};

					tmp.push_back({Quat(Vec3 {0, -(m_dim.h * .5f) * (sin(1.f)), (m_dim.w * .5f) * (cos(1.f))}).rotation(360.f / m_segments * (a + 1),0,1,0).getCoord()});
					tmp.back().uv = {float(a + 1) / m_segments,sin(1.f)};
					tmp.push_back({Quat(Vec3 {0, -(m_dim.h * .5f) * (sin(1.f)), (m_dim.w * .5f) * (cos(1.f))}).rotation(360.f / m_segments * a,0,1,0).getCoord()});
					tmp.back().uv = {float(a) / m_segments,sin(1.f)};


					norm = Vec3::crossProduct(tmp[tmp.size() - 1].coord - tmp[tmp.size() - 3].coord, tmp[tmp.size() - 1].coord - tmp[tmp.size() - 2].coord).normal();
					tmp[tmp.size() - 1].norm =
						tmp[tmp.size() - 2].norm =
						tmp[tmp.size() - 3].norm = norm;



				}
				else
				{
					//bottom left
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0, -(m_dim.h * .5f) * (sin((b - 1) / (float)m_divisions)),(m_dim.w * .5f) * (cos((b - 1) / (float)m_divisions))}).rotation(360.f / m_segments * a,0,1,0))});
					tmp.back().uv = {a / (float)m_segments,(b - 1) / (float)m_divisions};

					//top left
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0, -(m_dim.h * .5f) * (sin(b / (float)m_divisions)), (m_dim.w * .5f) * (cos(b / (float)m_divisions))}).rotation(360.f / m_segments * a,0,1,0))});
					tmp.back().uv = {a / (float)m_segments,b / (float)m_divisions};

					//top right
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0, -(m_dim.h * .5f) * (sin(b / (float)m_divisions)), (m_dim.w * .5f) * (cos(b / (float)m_divisions))}).rotation(360.f / m_segments * (a + 1),0,1,0))});
					tmp.back().uv = {(a + 1) / (float)m_segments,b / (float)m_divisions};

					norm = Vec3::crossProduct(tmp[tmp.size() - 2].coord - tmp[tmp.size() - 1].coord, tmp[tmp.size() - 2].coord - tmp[tmp.size() - 3].coord).normal();
					tmp[tmp.size() - 1].norm =
						tmp[tmp.size() - 2].norm =
						tmp[tmp.size() - 3].norm = norm;


					//bottom left
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0,-(m_dim.h * .5f) * (sin((b - 1) / (float)m_divisions)), (m_dim.w * .5f) * (cos((b - 1) / (float)m_divisions))}).rotation(360.f / m_segments * a,0,1,0))});
					tmp.back().uv = {a / (float)m_segments,(b - 1) / (float)m_divisions};
					//top right
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0,-(m_dim.h * .5f) * (sin(b / (float)m_divisions)),(m_dim.w * .5f) * (cos(b / (float)m_divisions))}).rotation(360.f / m_segments * (a + 1),0,1,0))});
					tmp.back().uv = {(a + 1) / (float)m_segments,b / (float)m_divisions};
					//bottom right
					tmp.push_back({reclass(Vec3,tmp2 = Quat(Vec3 {0,-(m_dim.h * .5f) * (sin((b - 1) / (float)m_divisions)), (m_dim.w * .5f) * (cos((b - 1) / (float)m_divisions))}).rotation(360.f / m_segments * (a + 1),0,1,0))});
					tmp.back().uv = {(a + 1) / (float)m_segments,(b - 1) / (float)m_divisions};

					norm = Vec3::crossProduct(tmp[tmp.size() - 1].coord - tmp[tmp.size() - 3].coord, tmp[tmp.size() - 1].coord - tmp[tmp.size() - 2].coord).normal();
					tmp[tmp.size() - 1].norm =
						tmp[tmp.size() - 2].norm =
						tmp[tmp.size() - 3].norm = norm;
				}

			}
		}

		//Second Half of Sphere
		tmp.insert(tmp.end(), tmp.rbegin(), tmp.rend());
		for(auto a = tmp.rbegin(); a < tmp.rbegin() + tmp.size() / 2; a++)
		{
			a[0].coord.y *= -1;
			a[0].norm.y *= -1;
			a[0].uv.v = 1 - a[0].uv.v;
		}

		for(auto& a : tmp)
			a.coord += getLocalPosition();

		indiceCreator(tmp);

		calculateBounds();

	}
private:
	int m_segments = 1, m_divisions;
	bool m_invert;
};

class Mesh:public Component
{
public:
	Mesh();
	Mesh(Mesh& mesh);
	virtual ~Mesh();

	void init();

	bool loadPrimitive(PrimitiveMesh* mesh);

	void editVerts(Mesh* verts1, Mesh* verts2);

	void render(Shader& shader, bool enableTex);

	void unload();

	void replaceTexture(int index, GLuint tex);

	Vec3 top = {}, bottom = {}, left = {}, right = {}, front = {}, back = {};
	std::string meshName = "";
	std::vector<std::string> matNames;

	std::vector<Vertex3D>& getUnpackedData() { return m_unpackedData; }
	std::vector<unsigned>& getIndicieData() { return m_indicieData; }
	std::vector<Texture2D>& getTextures() { return m_textures; }

	std::vector<GLuint>& getReplaceTex() { return m_replaceTex; }
private:


	std::vector<GLuint> m_replaceTex;

	bool ani = false;

	GLuint m_vaoID = 0;
	std::pair<GLuint, GLuint> m_vboID = {0,0};
	GLuint m_elemID = 0;

	std::vector<Vertex3D> m_unpackedData;
	std::vector<unsigned> m_indicieData;
	std::vector<Texture2D>m_textures;


};

