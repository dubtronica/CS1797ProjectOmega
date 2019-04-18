#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <array>
#include "glm/ext.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323
#endif

struct Mat3 {
	GLfloat v[9];
};

Mat3 rotateX(float theta) {
	const auto s = sin(theta);
	const auto c = cos(theta);
	return Mat3{
		1, 0, 0,
		0, c, -s,
		0, s, c
	};
}

Mat3 operator*(const Mat3 &lhs, const Mat3 &rhs) {
	Mat3 ret = { 0 };

	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			for (size_t k = 0; k < 3; ++k) {
				ret.v[i * 3 + j] += lhs.v[i * 3 + k] * rhs.v[k * 3 + j];
			}
		}
	}
	return ret;
}

std::array<GLfloat, 16> rotateZ(GLfloat theta) {
	using namespace std;
	return array<GLfloat, 16>{
		cos(theta), -sin(theta), 0, 0,
			sin(theta), cos(theta), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
	};
};
std::array<GLfloat, 16> rotateY(GLfloat theta) {
	using namespace std;
	return array<GLfloat, 16>{
		cos(theta), 0, sin(theta), 0,
			0, 1, 0, 0,
			-sin(theta), 0, cos(theta), 0,
			0, 0, 0, 1
	};
};

//=======================================================================

struct VPosNormal {
	glm::vec3 pos;
	glm::vec3 normal;
};

struct VPosTexNormalTangent {
	glm::vec3 pos;
	glm::vec2 tex;
	glm::vec3 normal;
	glm::vec3 tangent;
};

size_t mirror(size_t i, size_t n) {
	if (i < n) {
		return i;
	}
	else if (i == n) {
		return n - 2;
	}
	else if (i == n + 1) {
		return n - 3;
	}
}

VPosNormal fuv(float u, float v, float r) {
	float x = cos(u) * sin(v) * r;
	float y = cos(v) * r;
	float z = sin(u) * sin(v) * r;

	glm::vec3 a{ x, y, z };

	return VPosNormal{ a, glm::normalize(a) };
}

vector<VPosNormal> genSphere(float r, int lats, int longs) {

	vector<VPosNormal> vertices;

	float startU = 0;
	float startV = 0;

	float endU = M_PI * 2;
	float endV = M_PI;

	float stepU = (endU - startU) / longs;
	float stepV = (endV - startV) / lats;

	for (int i = 0; i < longs; i++) {
		for (int j = 0; j < lats; j++) {
			float u = i * stepU + startU;
			float v = j * stepV + startV;

			float un = (i + 1 == lats) ? endU : (i + 1)*stepU + startU;
			float vn = (j + 1 == longs) ? endV : (j + 1)*stepV + startV;

			VPosNormal p0 = fuv(u, v, r);
			VPosNormal p1 = fuv(u, vn, r);
			VPosNormal p2 = fuv(un, v, r);
			VPosNormal p3 = fuv(un, vn, r);

			vertices.push_back(p0);
			vertices.push_back(p2);
			vertices.push_back(p1);
			vertices.push_back(p3);
			vertices.push_back(p1);
			vertices.push_back(p2);
		}
	}

	return vertices;
}

vector<VPosTexNormalTangent> genSpherePosTexNormalTangent(float r, int lats, int longs) {
	auto sphere = genSphere(r, lats, longs);

	std::vector<VPosTexNormalTangent> data(sphere.size());
	for (size_t i = 0; i < sphere.size(); ++i) {
		data[i].pos = sphere[i].pos;
		data[i].normal = sphere[i].normal;
		data[i].tex.y = asin(sphere[i].normal.y) / M_PI + 0.5;
		data[i].tex.x = atan2(sphere[i].normal.z, sphere[i].normal.x) / (2.0 * M_PI) + 0.5;
	}

	for (size_t i = 0; i < data.size(); ++i) {
		const auto pa = data[i];
		const auto pb = data[mirror(i + 1, data.size())];
		const auto pc = data[mirror(i + 2, data.size())];

		const auto wb = pb.pos - pa.pos;
		const auto wc = pc.pos - pa.pos;

		const auto rb = pb.tex - pa.tex;
		const auto rc = pc.tex - pa.tex;

		const auto TB = glm::mat2x3(wb, wc) * glm::inverse(glm::mat2(rb, rc));
		const auto B = glm::cross(data[i].normal, TB[0]);
		auto T = glm::normalize(cross(B, data[i].normal));

		T = glm::normalize(cross(glm::vec3(0.0, 1.0, 0.0), pa.pos - glm::vec3(0.0, 0.0, 0.0)));

		data[i].tangent = T;
	}

	return data;
}

vector<VPosNormal> genPlane(glm::vec3 u, glm::vec3 v, const glm::vec3 &start, const unsigned int resolution) {
	u /= resolution;
	v /= resolution;
	vector<VPosNormal> plane;
	const auto normal = glm::normalize(glm::cross(u, v));

	for (unsigned int row = 0; row < resolution; ++row) {
		if (row != 0) {
			plane.push_back(VPosNormal{ v*(row + 1.0f) + start, normal });
		}
		for (unsigned int col = 0; col < resolution; ++col) {
			plane.push_back(VPosNormal{ u*float(col) + v * (row + 1.0f) + start, normal });
			plane.push_back(VPosNormal{ u*float(col) + v * float(row) + start, normal });
		}
		plane.push_back(VPosNormal{ u*float(resolution) + v * (row + 1.0f) + start, normal });
		plane.push_back(VPosNormal{ u*float(resolution) + v * float(row) + start, normal });
		if (row + 1 != resolution) {
			plane.push_back(plane.back());
		}
	}
	return plane;
}

//=======================================================================

string loadFile(string path) {
	string s = "";
	string out = "";
	ifstream f(path);
	if (f.is_open()) {
		while (getline(f, out)) {
			s.append(out + "\n");
		}
		f.close();
	}
	else {
		cerr << "Unable to open " + path + " shader file.";
	}
	return s;
}

GLuint createShader(GLuint type, const string source) {
	auto shader = glCreateShader(type);
	auto content = loadFile(source);
	auto c = content.c_str();
	GLint sz = content.size();
	glShaderSource(shader, 1, &c, &sz);
	glCompileShader(shader);
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		char infoLog[512];
		GLsizei sz = sizeof(infoLog);
		glGetShaderInfoLog(shader, sz, &sz, infoLog);
		throw runtime_error(string("shader compilation error: ") + infoLog);
	}

	return shader;
}

GLuint loadProgram(string vs, string fs) {

	auto vsh = createShader(GL_VERTEX_SHADER, vs);
	auto fsh = createShader(GL_FRAGMENT_SHADER, fs);
	auto program = glCreateProgram();

	glAttachShader(program, vsh);
	glAttachShader(program, fsh);

	glLinkProgram(program);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei sz = sizeof(infoLog);
		glGetProgramInfoLog(program, sz, &sz, infoLog);
		cerr << "program link compilation error: " << infoLog;
		return -1;
	}

	glDeleteShader(vsh);
	vsh = 0;
	glDeleteShader(fsh);
	fsh = 0;

	return program;
}