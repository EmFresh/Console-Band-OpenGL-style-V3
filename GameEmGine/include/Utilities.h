#pragma once
//#define clamp(val, min, max) (val > max ? max : val < min ? min : val)
#include <GL/glew.h>
#include <glm/common.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#define reclass(a_class,a_val) (*(a_class*)&(a_val))
typedef const char* cstring;
typedef unsigned int   uint;
typedef unsigned short ushort;
//#define unsigned int unsigned int
//#define ushort unsigned short

static cstring cDir(char* dir)
{
	char* tmp;
	if(strlen(dir) > 0)
		while(bool(tmp = strchr(dir, '\\')))
		{
			tmp[0] = '/';
		}

	if(strlen(dir) > 1)
		while(bool(tmp = strstr(dir, "//")))
		{
			memmove_s(tmp, strlen(tmp), tmp + 1, strlen(tmp + 1));
			dir[strlen(dir) - 1] = 0;
		}

	return dir;
}

static std::string tolower(char* dir)
{
	std::string tmp;
	for(unsigned i = 0; i < strlen(dir); i++)
	{
		tmp += (char)tolower(*(dir + i));
	}
	return tmp;
}

template <typename T>
static inline T bezier(const T& p0, const T& t0, const T& t1, const T& p1, const float t)
{
	return
		t * t * t * (-p0 + 3.0f * t0 - 3.0f * t1 + p1) +
		t * t * (3.0f * p0 - 6.0f * t0 + 3.0f * t1) +
		t * (-3.0f * p0 + 3.0f * t0) +
		p0;
}

template <typename T>
static inline T catmull(T p0, T p1, T p2, T p3, float t)
{
	return 0.5f *
		t * t * t * (-p0 + p1 * 3.0f + p2 * -3.0f + p3) +
		t * t * (p0 * 2.0f + p1 * -5.0f + p2 * 4.0f - p3) +
		t * (-p0 + p2) +
		(2.0f * p1);

	//return p0;
}

template<typename T>
static inline T lerp(const T& v0, const T& v1, const float& t)
{
	return (T)((1.0f - t) * v0 + (t)*v1);
}

template<typename T>
static inline T clamp(T mini, T maxi, T val) { return std::min(mini, std::max(maxi, val)); }

//?
static inline int vectorWrap(int num, int mod) { return (num + mod) % mod; }

//template<class Enum, class Base>
//class _EnumCreator
//{
//public:
//	static Base* create(Enum a_enum)
//	{
//		typename std::map<Base, _EnumCreator<Enum, Base>*>::const_iterator const it = lookup().find(a_enum);
//		if(it == lookup().end())
//			return NULL;
//		return it->second->create();
//	}
//protected:
//	static std::map<Base, _EnumCreator<Enum, Base>*> lookup()
//	{
//		static std::map<Base, EnumCreator<Enum, Base>*> list;
//		return list;
//	}
//private:
//	virtual Base* create() = 0;
//};
//
//template<class Enum, class Base, class Der>
//class EnumCreator:public _EnumCreator
//{
//public:
//	EnumCreator(Enum val):
//		pos(this->lookup().insert(std::make_pair<Enum, _EnumCreator<Enum, Base>*>(key, this)).first)
//	{}
//private:
//	Base* create() { return new Der(); }
//	typename std::map<Enum, EnumFactory<Enum, Base>*>::iterator pos;
//};

#include <cmath>
#include <memory>
template<class T>
class ObjectPool
{
	uint capacity = 0;
	uint count = 0;
	uint current = 0;
	std::vector<std::shared_ptr<T>> objects;



public:

	ObjectPool(uint cap = 10) { setCapacity(cap); }

	/// <summary>
	/// sets the pool capacity
	/// </summary>
	/// <param name="cap"></param>
	void setCapacity(uint cap) { capacity = std::max(cap, (uint)1); count = 0; capacity < objects.size() ? objects.resize(cap) : void(); }

	/// <summary>
	/// gets the next object in the pool and sets it to default
	/// </summary>
	/// <returns></returns>
	T& getNewObject()
	{
		count %= capacity;

		//if(objects.size() > count)
		//	if(is_pointer<T>::value)
		//		if(objects[count])
		//			delete objects[count];
		//
		//typename remove_pointer<T>::type type;

		if(count >= objects.size())
		{
			//if(is_pointer::value)
			//	objects.push_back(new type());
			//else

			objects.push_back(std::shared_ptr<T>(new T()));

		}
		else
		{
			//	if(is_pointer::value)
			//		objects[count] = new type());
			//	else
			*objects[count] = T();
		}

		return *objects[current = count++];
	}

	/// <summary>
	/// gets the reference of the current object in the pool
	/// </summary>
	/// <returns></returns>
	T& getCurrentObject() { return *objects[current]; }

	void deleteObject(uint index) { objects.erase(objects.begin() + index); }
	void deleteObject(T& index) { objects.erase(std::find(objects.begin(), objects.end(), index)); }

	std::vector<std::shared_ptr<T>>& getObjectList() { return objects; }
};



template<class T = float>
struct Coord2D
{

	union
	{
		struct { T x, y; };
		struct { T u, v; };
		struct { T w, h; };
		struct { T width, height; };
	};

	Coord2D() = default;
	Coord2D(T a_x, T a_y)
	{
		x = a_x, y = a_y;
	}

	glm::vec2 toVec2()
	{
		return glm::vec2(x, y);
	}

	T length()
	{
		return sqrtf(x * x + y * y);
	}
	static T distance(Coord2D v1, Coord2D v2)
	{
		Coord2D v3 = v2 - v1;
		return sqrtf(v3.x * v3.x + v3.y * v3.y);
	}

	T distanceSquare()
	{
		return (x * x + y * y);
	}

	Coord2D normal()
	{
		return *this / length();
	}

	T& operator[](int m_index)
	{
		T* error = nullptr;
		switch(m_index)
		{
		case 0:
			return static_cast<T&>(x);
		case 1:
			return static_cast<T&>(y);
		}
		return *error;
	}

	Coord2D operator+(Coord2D coord)
	{
		return {x + coord.x, y + coord.y};
	}
	Coord2D operator-(Coord2D coord)
	{
		return {x - coord.x, y - coord.y};
	}
	Coord2D operator/(Coord2D coord)
	{
		return {x / coord.x,y / coord.y};
	}
	Coord2D operator/(T coord)
	{
		return {x / coord,y / coord};
	}
	void operator-=(Coord2D coord)
	{
		x -= coord.x;
		y -= coord.y;
	}
	void operator+=(Coord2D coord)
	{
		x += coord.x;
		y += coord.y;
	}
	void operator/=(Coord2D coord)
	{
		x /= coord.x, y /= coord.y;
	}
	void operator/=(T coord)
	{
		x /= coord, y /= coord;
	}


	bool operator==(Coord2D<T> coord)const
	{
		return
			x == coord.x &&
			y == coord.y;
	}

	bool operator!=(Coord2D<T> coord)const
	{
		return !(*this == coord);
	}

	//based on distance
	bool operator>(Coord2D<T> coord)
	{
		return this->distanceSquare() > coord.distanceSquare();
	}
	//based on distance
	bool operator<=(Coord2D<T> coord)const
	{
		return !(*this > coord);
	}
	//based on distance
	bool operator<(Coord2D<T> coord)
	{
		return distanceSquare() < coord.distanceSquare();
	}
	//based on distance
	bool operator>=(Coord2D<T> coord)const
	{
		return !(*this < coord);
	}
};

template<class T = float>
struct Coord3D
{
	union
	{
		struct { T x, y, z; };
		struct { T w, h, d; };
		struct { T width, height, depth; };
	};


	Coord3D():x(0), y(0), z(0) {};

	Coord3D(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
		z = 0;
	}
	template<class P = float>
	Coord3D(const Coord2D<P> coord)
	{
		x = (T)coord.x;
		y = (T)coord.y;
		z = (T)0;
	}
	Coord3D(glm::vec4 coord)
	{
		x = (T)coord.x;
		y = (T)coord.y;
		z = (T)coord.z;
	}

	void operator=(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
	}

	Coord3D(Coord2D<T> coord, T last)
	{
		x = coord.x;
		y = coord.y;
		z = last;
	}

	Coord3D(T scale)
	{
		this->x = scale;
		this->y = scale;
		this->z = scale;
	}

	Coord3D(T m_x, T m_y, T m_z)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = m_z;
	}

	Coord3D(T m_x, T m_y)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = 0;
	}

	glm::vec3 toVec3()
	{
		return glm::vec3{x,y,z};
	}

	static glm::vec3 toVec3(Coord3D<float> a0)
	{
		return reclass(glm::vec3, a0);
	}

	static glm::vec3 toVec3(Coord3D<int> a0)
	{
		return reclass(glm::vec3, a0);
	}

	static T distance(Coord3D<T> v1, Coord3D<T> v2)
	{
		v1 -= v2;
		return v1.length();
	}

	T length()
	{
		return (T)sqrtf(x * x + y * y + z * z);
	}

	T distanceSquare()
	{
		return (x * x + y * y + z * z);
	}

	static T dotProduct(Coord3D<T> a, Coord3D<T> b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Coord3D<T> crossProduct(Coord3D<T> a, Coord3D<T> b)
	{
		return
		{
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}
	Coord3D<T> crossProduct(Coord3D<T> b)
	{
		return
		{
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	friend static Coord3D<T> abs(Coord3D<T> val)
	{
		return {sqrtf(val.x * val.x),sqrtf(val.y * val.y),sqrtf(val.z * val.z)};
	}


	Coord3D<T> normal()
	{
		return *this / length();
	}


	void set(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
	}

	void set(T m_x, T m_y, T m_z)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = m_z;
	}

	void set(T m_x, T m_y)
	{
		this->x = m_x;
		this->y = m_y;
	}

	void normalize()
	{
		T norm = sqrtf(x * x + y * y + z * z);
		x /= norm;
		y /= norm;
		z /= norm;
	}

	T& operator[] (int m_index)
	{
		T* error = nullptr;
		switch(m_index)
		{
		case 0:
			return const_cast<T&>(x);
		case 1:
			return const_cast<T&>(y);
		case 2:
			return const_cast<T&>(z);
		}
		return *error;
	}

	Coord3D<T> operator+(Coord3D<T> coord)const
	{
		return {T(x + coord.x), T(y + coord.y), T(z + coord.z)};
	}

	Coord3D<T> operator-(Coord3D<T> coord)const
	{
		return {T(x - coord.x), T(y - coord.y), T(z - coord.z)};
	}

	friend Coord3D<T> operator-(T val, const Coord3D<T> coord)
	{
		return {T(val - coord.x), T(val - coord.y), T(val - coord.z)};
	}

	friend Coord3D<T> operator*(T scaler, const Coord3D<T> coord)
	{
		return {scaler * coord.x, scaler * coord.y, scaler * coord.z};
	}

	Coord3D<T> operator*(Coord3D<T> coord)const
	{
		return {x * coord.x, y * coord.y, z * coord.z};
	}

	Coord3D<T> operator*(T coord)const
	{
		return {x * coord, y * coord, z * coord};
	}

	Coord3D<T> operator/(Coord3D<T> coord)const
	{
		return {x / coord.x,y / coord.y,z / coord.z};
	}

	Coord3D<T> operator/(T coord)const
	{
		return {x / coord,y / coord,z / coord};
	}
	Coord3D<T> operator%(Coord3D<T> coord)const
	{
		return	{
			(T)coord.x ? std::fmodf(x , coord.x) : 0,
			(T)coord.y ? std::fmodf(y , coord.y) : 0,
			(T)coord.z ? std::fmodf(z , coord.z) : 0
		};
	}

	Coord3D<T> operator%(T coord)const
	{
		return {
			(T)coord ? std::fmodf(x , coord) : 0,
			(T)coord ? std::fmodf(y , coord) : 0,
			(T)coord ? std::fmodf(z , coord) : 0
		};
	}

	Coord3D<T> operator-()const
	{

		return *this * -1;
	}

	void operator-=(Coord3D<T> coord)
	{
		x -= coord.x;
		y -= coord.y;
		z -= coord.z;
	}

	void operator+=(Coord3D<T> coord)
	{
		x += coord.x;
		y += coord.y;
		z += coord.z;
	}

	void operator*=(Coord3D<T> coord)
	{
		x *= coord.x;
		y *= coord.y;
		z *= coord.z;
	}

	void operator*=(T val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	void operator/=(T val)
	{
		x /= val;
		y /= val;
		z /= val;
	}


	bool operator==(Coord3D<T> coord)const
	{
		return
			x == coord.x &&
			y == coord.y &&
			z == coord.z;
	}

	bool operator!=(Coord3D<T> coord)const
	{
		return !(*this == coord);
	}

	//based on distance
	bool operator>(Coord3D<T> coord)
	{
		return this->distanceSquare() > coord.distanceSquare();
	}
	//based on distance
	bool operator<=(Coord3D<T> coord)const
	{
		return !(*this > coord);
	}
	//based on distance
	bool operator<(Coord3D<T> coord)
	{
		return distanceSquare() < coord.distanceSquare();
	}
	//based on distance
	bool operator>=(Coord3D<T> coord)const
	{
		return !(*this < coord);
	}

	cstring toString()
	{
		static char toStr[60]{};
		sprintf_s(toStr, "(%f, %f, %f)", x, y, z);
		return toStr;
	}
private:

};

typedef Coord3D<float> Vec3;
typedef Coord2D<float> Vec2;

struct ColourRGBA
{
	GLubyte r, g, b, a;
	//1/255 = 0.0039215686274509803921568627451
#define BYTE_TO_FLOAT_MULTI 0.0039215686274509803921568627451

	ColourRGBA():r((GLubyte)255), g((GLubyte)255), b((GLubyte)255), a((GLubyte)255)
	{}

	ColourRGBA(GLubyte a_r, GLubyte a_g, GLubyte a_b, GLubyte a_a = (GLubyte)255):r(a_r), g(a_g), b(a_b), a(a_a)
	{}
	glm::vec4 toVec4()
	{
		return glm::vec4(r * BYTE_TO_FLOAT_MULTI, g * BYTE_TO_FLOAT_MULTI, b * BYTE_TO_FLOAT_MULTI, a * BYTE_TO_FLOAT_MULTI);
	}

	void set(ColourRGBA rgba)
	{
		set(
			rgba.r,
			rgba.g,
			rgba.b,
			rgba.a);
	}

	void set(GLubyte a_r, GLubyte a_g, GLubyte a_b)
	{
		this[0][0] = a_r;
		this[0][1] = a_g;
		this[0][2] = a_b;
	}

	void set(GLubyte a_r, GLubyte a_g, GLubyte a_b, GLubyte a_a)
	{
		this[0][0] = a_r;
		this[0][1] = a_g;
		this[0][2] = a_b;
		this[0][3] = a_a;
	}

	void set(float a_r, float a_g, float a_b)
	{
		this[0][0] = GLubyte(a_r * 255);
		this[0][1] = GLubyte(a_g * 255);
		this[0][2] = GLubyte(a_b * 255);
	}

	void set(float a_r, float a_g, float a_b, float a_a)
	{
		this[0][0] = GLubyte(a_r * 255);
		this[0][1] = GLubyte(a_g * 255);
		this[0][2] = GLubyte(a_b * 255);
		this[0][3] = GLubyte(a_a * 255);
	}

	ColourRGBA operator+(ColourRGBA rgba)
	{
		return ColourRGBA{
			GLubyte(r + rgba[0]),
			GLubyte(g + rgba[1]),
			GLubyte(b + rgba[2]),
			GLubyte(a + rgba[3])};
	}

	ColourRGBA operator*(float rgba)
	{
		return ColourRGBA{
			GLubyte(r * rgba),
			GLubyte(g * rgba),
			GLubyte(b * rgba),
			GLubyte(a * rgba)};
	}

	ColourRGBA operator/(float rgba)
	{
		return ColourRGBA{
			GLubyte(r / rgba),
			GLubyte(g / rgba),
			GLubyte(b / rgba),
			GLubyte(a / rgba)};
	}

	friend ColourRGBA operator*(float rgba, ColourRGBA colour)
	{
		return ColourRGBA{
			GLubyte(colour[0] * rgba),
			GLubyte(colour[1] * rgba),
			GLubyte(colour[2] * rgba),
			GLubyte(colour[3] * rgba)};
	}
	friend ColourRGBA operator/(float rgba, ColourRGBA colour)
	{
		return ColourRGBA{
			GLubyte(colour[0] / rgba),
			GLubyte(colour[1] / rgba),
			GLubyte(colour[2] / rgba),
			GLubyte(colour[3] / rgba)};
	}
	ColourRGBA operator*(ColourRGBA rgba)
	{
		return ColourRGBA{
			GLubyte(r * rgba.r / 255.f),
			GLubyte(g * rgba.g / 255.f),
			GLubyte(b * rgba.b / 255.f),
			GLubyte(a * rgba.a / 255.f)};
	}

	ColourRGBA operator/(ColourRGBA rgba)
	{
		return ColourRGBA{
			GLubyte(r / (float)rgba.r / 255.f),
			GLubyte(g / (float)rgba.g / 255.f),
			GLubyte(b / (float)rgba.b / 255.f),
			GLubyte(a / (float)rgba.a / 255.f)};
	}

	void operator*=(ColourRGBA rgba)
	{
		*this = *this * rgba;
	}

	void operator/=(ColourRGBA rgba)
	{
		*this = *this / rgba;
	}

	GLubyte& operator[](int index)
	{
		GLubyte* error = nullptr;
		switch(index)
		{
		case 0:
			return static_cast<GLubyte&>(r);
		case 1:
			return static_cast<GLubyte&>(g);
		case 2:
			return static_cast<GLubyte&>(b);
		case 3:
			return static_cast<GLubyte&>(a);
		}
		return *error;
	}
};

struct UV
{
	float u = 0, v = 0;

	void set(float a_u, float a_v)
	{
		this->u = a_u;
		this->v = a_v;
	}

	float& operator[](int m_index)
	{
		switch(m_index)
		{
		case 0:
			return static_cast<float&>(u);
		case 1:
			return static_cast<float&>(v);
		}
		float* error = nullptr;
		return *error;
	}

	bool operator==(UV uv)
	{
		return u == uv.u && v == uv.v;
	}

	void operator/=(float scale)
	{
		u /= scale;
		v /= scale;
	}
};

struct Vertex2D
{
	Coord2D<> coord;
	ColourRGBA	colour;
	UV uv;

	void setCoord2D(float x, float y)
	{
		coord.x = x;
		coord.y = y;
	}
	//uses the 0-255 representation instead of 0-1
	void setColour(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255)
	{
		colour.r = r;
		colour.g = g;
		colour.b = b;
		colour.a = a;
	}
	//sets uv
	void setUV(float u, float v)
	{
		uv.u = u;
		uv.v = v;
	}

	void print()
	{
		printf("Coord2D: (%f, %f)\n", coord.x, coord.y);
		printf("Colour : (%d, %d, %d, %d)\n", colour.r, colour.g, colour.b, colour.a);
		printf("UV     : (%f, %f)\n\n", uv.u, uv.v);
	}
};

struct Vertex3D
{
	Coord3D<float> coord, norm;
	UV uv;
	ColourRGBA	colour;

	void setCoord(float x, float y, float z)
	{
		coord.x = x;
		coord.y = y;
		coord.z = z;
	}
	//uses the 0-255 representation instead of 0-1
	void setColour(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255)
	{
		colour.r = r;
		colour.g = g;
		colour.b = b;
		colour.a = a;
	}
	//sets uv
	void setUV(float u, float v)
	{
		uv.u = u;
		uv.v = v;
	}

	void setNorm(float x, float y, float z)
	{
		norm.x = x;
		norm.y = y;
		norm.z = z;
	}

	bool operator==(Vertex3D vert)
	{
		return coord == vert.coord && norm == vert.norm && uv == vert.uv;
	}

	void print()
	{
		printf("Coord3D: (%f, %f, %f)\n", coord.x, coord.y, coord.z);
		printf("Colour : (%d, %d, %d, %d)\n", colour.r, colour.g, colour.b, colour.a);
		printf("UV     : (%f, %f)\n\n", uv.u, uv.v);
	}
};

struct Indicie
{

	unsigned coord = 0;
	unsigned uv = 0;
	unsigned norm = 0;

	unsigned& operator[](int m_index)
	{
		switch(m_index)
		{
		case 0:
			return static_cast<unsigned&>(coord);
		case 1:
			return static_cast<unsigned&>(uv);
		case 2:
			return static_cast<unsigned&>(norm);
		}
		unsigned* error = nullptr;
		return *error;
	}

	friend bool operator<(const Indicie& in1, const Indicie& in2)
	{
		return in1.coord < in2.coord;
	}

	bool operator==(Indicie in)
	{
		for(short i = 0; i < 3; ++i)
			if((*this)[i] != in[i])
				return false;
		return true;

		//return (*this)[0] != in[0];
	}

	void correct()
	{
		coord -= 1;
		norm -= 1;
		uv -= 1;
	}
};

