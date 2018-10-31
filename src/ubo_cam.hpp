#ifndef UBOCAM_HPP_INCLUDED
#define UBOCAM_HPP_INCLUDED

#include <string>
#include "vecmath.hpp"
#include "shader.hpp"

//Shader part
/*
layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};
*/

struct UBOData {
	Vec4i viewport;
	Vec4i viewport_center;
	Vec4f main_clipping_fov;
	Mat4f MVP2D;
	float ambient;
	float time;
};

class UBOCam {
private:
	UBOData UBOdata;
	std::string  UBOName;
	GLuint block_buffer;
	GLuint block_index;
	GLuint block_id;

public:
	UBOCam(const std::string &UBOName_);
	~UBOCam();
	void IndexAndBinding(GLuint program);
	void update();

	void setClippingFov(const Vec3f &v) {
		UBOdata.main_clipping_fov= v;
	}

	void setViewportCenter(const Vec3i &v) {
		UBOdata.viewport_center = v;
	}

	void setViewport(const Vec4i &v) {
		UBOdata.viewport = v;
	}

	void setMVP2D(const Mat4f &v) {
		UBOdata.MVP2D = v;
	}

	void setTime(float time) {
		UBOdata.time = time;
	}

	void setAmbientLight(float _ambient) {
		UBOdata.ambient = _ambient;
	}

	float getAmbientLight() {
		return UBOdata.ambient;
	}

};

#endif // UBOCAM_HPP_INCLUDED
