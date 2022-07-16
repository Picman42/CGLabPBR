#include <iostream>
#include <unordered_map>

//#include <tiny_obj_loader.h>
#include "my_obj_loader.h"

#include "model.h"

void Model::addFace(std::vector<Vertex> & vertices,int pd)
{
	Vertex vertex{};
	if (pd == 0) vertex.normal = glm::vec3(0, 0, -1);
	if (pd == 1) vertex.normal = glm::vec3(0, 0, 1);
	if (pd == 2) vertex.normal = glm::vec3(0, -1, 0);
	if (pd == 3) vertex.normal = glm::vec3(0, 1, 0);
	if (pd == 4) vertex.normal = glm::vec3(-1, 0, 0);
	if (pd == 5) vertex.normal = glm::vec3(1, 0, 0);
	for(int i=0;i<2;i++)
		for (int j = 0;j < 2;j++)
		{
			vertex.texCoord = glm::vec2(i, j);
			if (pd == 0) vertex.position = glm::vec3(i, j, 0);
			if (pd == 1) vertex.position = glm::vec3(i, j, 1);
			if (pd == 2) vertex.position = glm::vec3(i, 0, j);
			if (pd == 3) vertex.position = glm::vec3(i, 1, j);
			if (pd == 4) vertex.position = glm::vec3(0, i, j);
			if (pd == 5) vertex.position = glm::vec3(1, i, j);
			vertices.push_back(vertex);
			if (vertex.position.x > maxx) maxx = vertex.position.x;
			if (vertex.position.x < minx) minx = vertex.position.x;
			if (vertex.position.y > maxy) maxy = vertex.position.y;
			if (vertex.position.y < miny) miny = vertex.position.y;
			if (vertex.position.z > maxz) maxz = vertex.position.z;
			if (vertex.position.z < minz) minz = vertex.position.z;
		}

}

Model::Model(const std::string& filepath) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	if(filepath=="Sphere_built")
	{
		int N = 50;
		for(int i=0;i<=N;i++)
			for (int j = 0;j <=N;j++)
			{
				Vertex vertex{};
				float xx = (float)j / N, yy = (float)i / N;
				float Pi = 3.1415926535;
				float x = cos(xx * 2 * Pi) * sin(yy * Pi);
				float y = cos(yy * Pi);
				float z = sin(xx * 2 * Pi) * sin(yy * Pi);
				vertex.position = glm::vec3(x,y,z);
				vertex.normal = glm::normalize(glm::vec3(x, y, z));
				vertex.texCoord = glm::vec2(xx, yy);
				if (vertex.position.x > maxx) maxx = vertex.position.x;
				if (vertex.position.x < minx) minx = vertex.position.x;
				if (vertex.position.y > maxy) maxy = vertex.position.y;
				if (vertex.position.y < miny) miny = vertex.position.y;
				if (vertex.position.z > maxz) maxz = vertex.position.z;
				if (vertex.position.z < minz) minz = vertex.position.z;
				vertices.push_back(vertex);
			}
		for (int i = 0;i < N;i++)
			for (int j = 0;j < N;j++)
			{
				indices.push_back(i * (N + 1) + j);
				indices.push_back((i + 1) * (N + 1) + j);
				indices.push_back((i + 1) * (N + 1) + j + 1);
				indices.push_back(i * (N + 1) + j);
				indices.push_back((i + 1) * (N + 1) + j + 1);
				indices.push_back(i * (N + 1) + j + 1);
			}
	}
	else if (filepath == "Cube_built")
	{
		for (int i = 0;i < 6;i++)
		{
			addFace(vertices, i);
			indices.push_back(i * 4 + 0);
			indices.push_back(i * 4 + 1);
			indices.push_back(i * 4 + 2);
			indices.push_back(i * 4 + 1);
			indices.push_back(i * 4 + 2);
			indices.push_back(i * 4 + 3);
		}
	}
	else
	{
		//tinyobj::attrib_t attrib;
		attrib_t attrib;
		//std::vector<tinyobj::shape_t> shapes;
		std::vector<shape_t> shapes;
		//std::vector<tinyobj::material_t> materials;

		std::string err;
		std::string::size_type index = filepath.find_last_of("/");
		//std::string mtlBaseDir = filepath.substr(0, index + 1);

		//if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), mtlBaseDir.c_str())) {
		//	throw std::runtime_error("load " + filepath + " failure: " + err);
		//}
		if (!LoadObj(&attrib, &shapes, &err, filepath.c_str())) {
			throw std::runtime_error("load " + filepath + " failure: " + err);
		}

		if (!err.empty()) {
			std::cerr << err << std::endl;
		}


		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			//std::vector< std::vector< int > > curFaceIndices;
			for (const auto& index : shape.mesh.indices) {

				Vertex vertex{};

				vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
				vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
				vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];
				if (vertex.position.x > maxx) maxx = vertex.position.x;
				if (vertex.position.x < minx) minx = vertex.position.x;
				if (vertex.position.y > maxy) maxy = vertex.position.y;
				if (vertex.position.y < miny) miny = vertex.position.y;
				if (vertex.position.z > maxz) maxz = vertex.position.z;
				if (vertex.position.z < minz) minz = vertex.position.z;

				if (index.normal_index >= 0) {
					vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
					vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
					vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
				}

				if (index.texcoord_index >= 0) {
					vertex.texCoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
					vertex.texCoord.y = attrib.texcoords[2 * index.texcoord_index + 1];
				}

				// check if the vertex appeared before to reduce redundant data
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
				//std::vector< int > curIndex = { index.vertex_index, index.texcoord_index, index.normal_index };
				//curFaceIndices.push_back(curIndex);

			}
			//_faceIndices.push_back(curFaceIndices);
		}
	}

	_vertices = vertices;
	_indices = indices;
	
	initGLResources();
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: _vertices(vertices), _indices(indices) {
	initGLResources();
}

Model::~Model() {
	if (_ebo != 0) {
		glDeleteBuffers(1, &_ebo);
		_ebo = 0;
	}

	if (_vbo != 0) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}

	if (_vao != 0) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

void Model::draw() const {
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

GLuint Model::getVertexArrayObject() const {
	return _vao;
}

size_t Model::getVertexCount() const {
	return _vertices.size();
}

size_t Model::getFaceCount() const {
	return _indices.size() / 3;
}

void Model::initGLResources() {
	// create a vertex array object
	glGenVertexArrays(1, &_vao);
	// create a vertex buffer object
	glGenBuffers(1, &_vbo);
	// create a element array buffer
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(), GL_STATIC_DRAW);

	// specify layout, size of a vertex, data type, normalize, sizeof vertex array, offset of the attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

bool Model::inside(glm::vec3 pos)
{
	float t1;
	glm::vec3 t2;
	t2 = ((pos - position) * getRight());
	t1 = t2.x + t2.y + t2.z;
	if (t1<minx * scale.x - 0.05f || t1>maxx * scale.x + 0.05f) return false;
	t2 = ((pos - position) * getUp());
	t1 = t2.x + t2.y + t2.z;
	if (t1<miny * scale.y - 0.05f || t1>maxy * scale.y + 0.05f) return false;
	t2 = -((pos - position) * getFront());
	t1 = t2.x + t2.y + t2.z;
	if (t1<minz * scale.z - 0.05f || t1>maxz * scale.z + 0.05f) return false;
	return true;
}