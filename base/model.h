#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "vertex.h"
#include "object3d.h"

class Model : public Object3D {
public:
	Model(const std::string& filepath);

	Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	~Model();

	Model(Model&& model) noexcept = default;

	GLuint getVertexArrayObject() const;

	size_t getVertexCount() const;

	size_t getFaceCount() const;

	void addFace(std::vector<Vertex>& vertices, int pd);

	void draw() const;

	// vertices of the table represented in model's own coordinate
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	//std::vector< std::vector< std::vector< int > > > _faceIndices;
	//std::vector<Vertex> _verticesWithIndex;
	float minx=10000.0f, miny= 10000.0f, minz= 10000.0f, maxx=-10000.0f, maxy=-10000.0f, maxz=-10000.0f;
	bool inside(glm::vec3 pos);

private:

	// opengl objects
	GLuint _vao = 0;
	GLuint _vbo = 0;
	GLuint _ebo = 0;

	void initGLResources();
};