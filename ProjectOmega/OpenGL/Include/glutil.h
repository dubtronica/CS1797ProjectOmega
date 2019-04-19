#ifndef GLUTIL_H
#define GLUTIL_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLfloat PI = (GLfloat)acos(-1);

struct Vertex {
	GLfloat
		x, y, z, // position
		x1, y1, z1; // normal
};

struct NewVertex {
	GLfloat
		x, y, z, // position
		x1, y1, z1, // normal
		u, v; // texture
};

inline void checkForErrors(unsigned int shader, std::string type);

inline GLuint loadProgram(const GLchar* vsh, const GLchar* fsh) {
	/*
	Loads a shader program. Takes 2 strings as arguments: file name of vertex shader, file name of fragment shader
	*/

	GLuint ID;

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// open files
		vShaderFile.open(vsh);
		fShaderFile.open(fsh);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkForErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkForErrors(fragment, "FRAGMENT");
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkForErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return ID;
}

inline std::vector<Vertex> genPlane(glm::vec3 u, glm::vec3 v, const glm::vec3 &start, const GLuint resolution) {
	/*
	Generates a plane
	u -> one side of the plane
	v -> the other side of the plane
	Size of the plane depends on the magnitude of u and v
	start -> lower left corner of the plane
	resolution -> how many points are in the plane (resolution of 1 generates 4 points, resolution of 2 generates 9 vertices, 3 -> 12, etc
	*/
	u /= resolution; v /= resolution;
	std::vector<Vertex> mesh;
	glm::vec3 normal = glm::normalize(glm::cross(u, v));

	for (GLuint row = 0; row < resolution; ++row) {
		if (row != 0) {
			glm::vec3 p = v * (row + 1.f) + start;
			mesh.push_back(Vertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z });
		}
		for (GLuint col = 0; col < resolution; ++col) {
			glm::vec3 p = u * float(col) + v * (row + 1.f) + start;
			mesh.push_back(Vertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z });

			glm::vec3 p1 = u * float(col) + v * float(row) + start;
			mesh.push_back(Vertex{ p1.x, p1.y, p1.z, normal.x, normal.y, normal.z });
		}
		glm::vec3 p = u * float(resolution) + v * (row + 1.f) + start;
		mesh.push_back(Vertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z });
		glm::vec3 p1 = u * float(resolution) + v * float(row) + start;
		mesh.push_back(Vertex{ p1.x, p1.y, p1.z, normal.x, normal.y, normal.z });

		if (row + 1 != resolution) {
			mesh.push_back(mesh.back());
		}
	}

	return mesh;
}

inline std::vector<Vertex> genCube(const GLfloat size, const unsigned int resolution, const glm::vec3 &offset = glm::vec3(0)) {
	/*
	Uses genPlane 6 times to make a cube
	size -> length of a side
	resolution -> how many points are generated
	offset -> position upon initialization (center)
	*/
	std::vector<Vertex> mesh;
	glm::vec3 u, v, o;
	for (GLuint side = 0; side < 6; ++side) {
		switch (side >> 1) {
		case 0:
			o = glm::vec3(size*0.5, 0, 0);
			u = glm::vec3(0, 0, -size);
			v = glm::vec3(0, size, 0);
			break;
		case 1:
			o = glm::vec3(0, size*0.5, 0);
			u = glm::vec3(size, 0, 0);
			v = glm::vec3(0, 0, -size);
			break;
		case 2:
			o = glm::vec3(0, 0, size*0.5);
			u = glm::vec3(size, 0, 0);
			v = glm::vec3(0, size, 0);
			break;
		default:
			assert(false && "Should never happen.");
		}

		if (side % 2 == 1) {
			o *= -1;
			u *= -1;
		}
		o -= (u + v) / 2.f;

		const auto &p = genPlane(u, v, o + offset, resolution);
		if (!mesh.empty()) {
			mesh.push_back(mesh.back());
			mesh.push_back(p.front());
		}
		mesh.insert(mesh.end(), p.begin(), p.end());
	}

	return mesh;
}

inline std::vector<Vertex> genSphere(GLfloat radius, GLuint resolution, const glm::vec3 &offset = glm::vec3(0)) {
	/*
	Uses genCube to generate a sphere
	Offsets the vertices of each cube by radius relative to a center, and is then offset to position (offset)
	*/
	auto cube = genCube(1.f, resolution);
	for (Vertex &v : cube) {
		glm::vec3 pos(v.x, v.y, v.z);
		pos = pos * (radius / glm::length(pos)) + offset;

		glm::vec3 nor(0.f);
		nor = pos / glm::length(pos);

		v.x = pos.x;
		v.y = pos.y;
		v.z = pos.z;
		v.x1 = nor.x;
		v.y1 = nor.y;
		v.z1 = nor.z;
	}
	return cube;
}

inline void checkForErrors(unsigned int shader, std::string type) {
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") { // link errors
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else { // shader compile errors
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

inline void loadTexture(GLuint* tex, GLuint texUnit, const GLchar * fileName) {
	/*
	Loads 2D textures
	tex -> GLuint where to store the texture
	texUnit -> which texture unit to load the texture into
	fileName -> file name of image to load
	*/
	glGenTextures(1, tex);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//loading images
	int w, h, n;
	auto *data = stbi_load(fileName, &w, &h, &n, 0);
	std::cout << w << " " << h << std::endl;
	if (data) {
		GLuint format = n == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture.\n";
	}
	stbi_image_free(data);
}

struct Matrix4 {
	GLfloat data[16];
	/*
	should be column-major, meaning the matrix looks like
	0  4  8 12
	1  5  9 13
	2  6 10 14
	3  7 11 15
	*/
	Matrix4() {
		// instantiate as an identity matrix
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i + j * 4] = i == j ? 1.f : 0.f;
			}
		}
	}

	void set(GLuint row, GLuint col, GLfloat val) {
		// sets the value of the element in the given index
		assert(row > -1 || row < 4 || col > -1 || col < 4);
		data[col * 4 + row] = val;
	}

	GLfloat get(GLuint row, GLuint col) {
		// retrieves a value in the given index
		assert(row > -1 || row < 4 || col > -1 || col < 4);
		return data[col * 4 + row];
	}

	void print() {
		// for debugging
		std::cout << data[0] << " " << data[4] << " " << data[8] << " " << data[12] << "\n";
		std::cout << data[1] << " " << data[5] << " " << data[9] << " " << data[13] << "\n";
		std::cout << data[2] << " " << data[6] << " " << data[10] << " " << data[14] << "\n";
		std::cout << data[3] << " " << data[7] << " " << data[11] << " " << data[15] << "\n\n";
	}
};

struct Vector4 {
	GLfloat x, y, z, w; // column vector
	Vector4(GLfloat x, GLfloat y, GLfloat z, GLfloat w) : x(x), y(y), z(z), w(w) {

	}

	Vector4(GLfloat n) {
		x, y, z, w = n;
	}

	Vector4() {
		x, y, z, w = 1.f;
	}
};

Vector4 multiply(Matrix4 a, Vector4 b) {
	// returns vector AB
	GLfloat res[4];
	GLfloat vec[4] = { b.x, b.y, b.z, b.w };
	for (int i = 0; i < 4; i++) {
		// for each vector element
		GLfloat ans = 0;
		for (int j = 0; j < 4; j++) {
			ans += a.data[i + 4 * j] * vec[j];
		}
		res[i] = ans;
	}
	return Vector4(res[0], res[1], res[2], res[3]);
}

Matrix4 multiply(Matrix4 a, Matrix4 b) {
	Matrix4 result;
	// returns AB
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			GLfloat val = 0;
			for (int i = 0; i < 4; i++) {
				val += a.get(row, i) * b.get(i, col);
			}
			result.set(row, col, val);
		}
	}
	return result;
}

Matrix4 rotate(Matrix4 mat, GLfloat a, GLfloat x, GLfloat y, GLfloat z) {
	// creates a matrix B that rotaties by amount a in degrees in the given normalized arbitrary axis (x, y, z)
	// returns multiply(B, MAT)

	// convert the angle to radians
	GLfloat angle = a * PI / 180.f;

	// normalize the direction vector
	GLfloat mag = sqrt(x*x + y * y + z * z);
	x /= mag;
	y /= mag;
	z /= mag;

	Matrix4 result;
	result.set(0, 0, x * x * (1 - cos(angle)) + cos(angle));
	result.set(1, 0, y * x * (1 - cos(angle)) + z * sin(angle));
	result.set(2, 0, z * x * (1 - cos(angle)) - y * sin(angle));

	result.set(0, 1, x * y * (1 - cos(angle)) - z * sin(angle));
	result.set(1, 1, y * y * (1 - cos(angle)) + cos(angle));
	result.set(2, 1, z * y * (1 - cos(angle)) + x * sin(angle));

	result.set(0, 2, x * z * (1 - cos(angle)) + y * sin(angle));
	result.set(1, 2, y * z * (1 - cos(angle)) - x * sin(angle));
	result.set(2, 2, z * z * (1 - cos(angle)) + cos(angle));

	return multiply(mat, result);
}

Matrix4 translate(Matrix4 mat, GLfloat x, GLfloat y, GLfloat z) {
	// creates a matrix B that translates in x-, y-, z-axis by amount denoted by inputs x, y, z
	// returns multiply(B, MAT)
	Matrix4 result;
	result.set(0, 3, x);
	result.set(1, 3, y);
	result.set(2, 3, z);
	return multiply(mat, result);
}

Matrix4 scale(Matrix4 mat, GLfloat x, GLfloat y, GLfloat z) {
	// creates a scaling matrix b that scales values in the respective axes by the respective inputs
	// returns multiply(B, MAT)
	Matrix4 result;
	result.set(0, 0, x);
	result.set(1, 1, y);
	result.set(2, 2, z);
	return multiply(mat, result);
}

//================================== with texture ========================================

inline std::vector<NewVertex> genTexPlane(glm::vec3 u, glm::vec3 v, const glm::vec3& start, const GLuint resolution) {
	/*
	 Generates a plane
	 u -> one side of the plane
	 v -> the other side of the plane
	 Size of the plane depends on the magnitude of u and v
	 start -> lower left corner of the plane
	 resolution -> how many points are in the plane (resolution of 1 generates 4 points, resolution of 2 generates 9 vertices, 3 -> 12, etc
	 */
	u /= resolution; v /= resolution;
	std::vector<NewVertex> mesh;
	glm::vec3 normal = glm::normalize(glm::cross(u, v));

	for (GLuint row = 0; row < resolution; ++row) {
		if (row != 0) {
			glm::vec3 p = v * (row + 1.f) + start;
			mesh.push_back(NewVertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z, 0.f , float(row + 1) / resolution });
		}
		for (GLuint col = 0; col < resolution; ++col) {
			glm::vec3 p = u * float(col) + v * (row + 1.f) + start;
			mesh.push_back(NewVertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z, float(col) / resolution , float(row + 1) / resolution });

			glm::vec3 p1 = u * float(col) + v * float(row) + start;
			mesh.push_back(NewVertex{ p1.x, p1.y, p1.z, normal.x, normal.y, normal.z, float(col) / resolution , float(row) / resolution });
		}
		glm::vec3 p = u * float(resolution) + v * (row + 1.f) + start;
		mesh.push_back(NewVertex{ p.x, p.y, p.z, normal.x, normal.y, normal.z, 1.f, float(row + 1) / resolution });
		glm::vec3 p1 = u * float(resolution) + v * float(row) + start;
		mesh.push_back(NewVertex{ p1.x, p1.y, p1.z, normal.x, normal.y, normal.z, 1.f , float(row) / resolution });

		if (row + 1 != resolution) {
			mesh.push_back(mesh.back());
		}
	}

	return mesh;
}

inline std::vector<NewVertex> genTexCube(const GLfloat size, const unsigned int resolution, const glm::vec3& offset = glm::vec3(0)) {
	/*
	 Uses genPlane 6 times to make a cube
	 size -> length of a side
	 resolution -> how many points are generated
	 offset -> position upon initialization (center)
	 */
	std::vector<NewVertex> mesh;
	glm::vec3 u, v, o;
	for (GLuint side = 0; side < 6; ++side) {
		switch (side >> 1) {
		case 0:
			o = glm::vec3(size * 0.5, 0, 0);
			u = glm::vec3(0, 0, -size);
			v = glm::vec3(0, size, 0);
			break;
		case 1:
			o = glm::vec3(0, size * 0.5, 0);
			u = glm::vec3(size, 0, 0);
			v = glm::vec3(0, 0, -size);
			break;
		case 2:
			o = glm::vec3(0, 0, size * 0.5);
			u = glm::vec3(size, 0, 0);
			v = glm::vec3(0, size, 0);
			break;
		default:
			assert(false && "Should never happen.");
		}

		if (side % 2 == 1) {
			o *= -1;
			u *= -1;
		}
		o -= (u + v) / 2.f;

		const auto& p = genTexPlane(u, v, o + offset, resolution);
		if (!mesh.empty()) {
			mesh.push_back(mesh.back());
			mesh.push_back(p.front());
		}
		mesh.insert(mesh.end(), p.begin(), p.end());
	}

	return mesh;
}

#endif