#include "Mesh.h"

#include <GL/glew.h>
using namespace std;

#define CHECKERS_HEIGHT 32
#define CHECKERS_WIDTH 32

Mesh::Mesh()
{
	
}

Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec2> tex_coords, std::vector<glm::vec3> normals, std::vector<glm::u8vec3> colors, std::vector<unsigned int> indices)
{
	load(vertices.data(), vertices.size(), indices.data(), indices.size());

	if (!tex_coords.empty()) {
		loadTexCoords(tex_coords.data(), tex_coords.size());
	}

	if (!normals.empty()) {
		loadNormals(normals.data(), normals.size());
	}

	if (!colors.empty()) {
		loadColors(colors.data(), colors.size());
	}
}

void Mesh::load(const glm::vec3* vertices, size_t num_verts, unsigned int* indices, size_t num_indexs)
{
	_vertices.assign(vertices, vertices + num_verts);
	_indices.assign(indices, indices + num_indexs);
	_vertices_buffer.loadData(vertices, num_verts * sizeof(glm::vec3));
	_indices_buffer.loadIndices(indices, num_indexs);
	_texCoords_buffer.unload();
	_normals_buffer.unload();
	_colors_buffer.unload();

	_boundingBox.min = _vertices.front();
	_boundingBox.max = _vertices.front();

	for (const auto& v : _vertices) {
		_boundingBox.min = glm::min(_boundingBox.min, glm::dvec3(v));
		_boundingBox.max = glm::max(_boundingBox.max, glm::dvec3(v));
	}
}

void Mesh::loadTexCoords(const glm::vec2* tex_coords, size_t num_tex_coords)
{
	_texCoords_buffer.loadData(tex_coords, num_tex_coords * sizeof(glm::vec2));
}

void Mesh::loadNormals(const glm::vec3* normals, size_t num_normals)
{
	_normals_buffer.loadData(normals, num_normals * sizeof(glm::vec3));
}

void Mesh::loadColors(const glm::u8vec3* colors, size_t num_colors)
{
	_colors_buffer.loadData(colors, num_colors * sizeof(glm::u8vec3));
}

void Mesh::draw() const
{

	if (texture_id)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}

	if (_texCoords_buffer.id())
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		_texCoords_buffer.bind();
		glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
	}

	if (_normals_buffer.id())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		_normals_buffer.bind();
		glNormalPointer(GL_FLOAT, 0, nullptr);
	}

	if (_colors_buffer.id())
	{
		glEnableClientState(GL_COLOR_ARRAY);
		_colors_buffer.bind();
		glColorPointer(3, GL_UNSIGNED_BYTE, 0, nullptr);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	_vertices_buffer.bind();
	glVertexPointer(3, GL_FLOAT, 0, nullptr);

	_indices_buffer.bind();
	glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (_colors_buffer.id()) glDisableClientState(GL_COLOR_ARRAY);
	if (_normals_buffer.id()) glDisableClientState(GL_NORMAL_ARRAY);
	if (_texCoords_buffer.id()) glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (texture_id)
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}



void Mesh::CheckerTexture()
{
	GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];
	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkerImage[i][j][0] = (GLubyte)c;
			checkerImage[i][j][1] = (GLubyte)c;
			checkerImage[i][j][2] = (GLubyte)c;
			checkerImage[i][j][3] = (GLubyte)255;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
	

}

void Mesh::deleteCheckerTexture() {
	if (texture_id) {
		glDeleteTextures(1, &texture_id);
		texture_id = 0;
	}
}

void Mesh::drawNormals(float length) const {
	glBegin(GL_LINES);
	glColor3ub(255, 0, 0);
	for (size_t i = 0; i < _indices.size(); i += 3) {
		glm::vec3 v0 = _vertices[_indices[i]];
		glm::vec3 v1 = _vertices[_indices[i + 1]];
		glm::vec3 v2 = _vertices[_indices[i + 2]];

		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		glm::vec3 center = (v0 + v1 + v2) / 3.0f;

		glVertex3fv(glm::value_ptr(center));
		glVertex3fv(glm::value_ptr(center + normal * length));
	}
	glClearColor(0, 0, 0, 0);
	glEnd();
}

void Mesh::LoadFile(const char* file_path)
{
	const aiScene* scene = aiImportFile(file_path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr && scene->HasMeshes()) {
		std::vector<glm::vec3> all_vertices;
		std::vector<unsigned int> all_indices;
		std::vector<glm::vec2> all_texCoords;
		std::vector<glm::vec3> all_normals;
		std::vector<glm::u8vec3> all_colors;

		unsigned int vertex_offset = 0;

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];

			// Copy vertices
			//gui->logMessage("Loading mesh vertices");
			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				all_vertices.push_back(glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z));
			}

			// Copy indices
			//gui->logMessage("Loading mesh indices");
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				aiFace& face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; k++) {
					all_indices.push_back(face.mIndices[k] + vertex_offset);
				}
			}

			// Copy texture coordinates
			//gui->logMessage("Loading mesh texture coordinates");
			if (mesh->HasTextureCoords(0)) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_texCoords.push_back(glm::vec2(mesh->mTextureCoords[0][j].x, -mesh->mTextureCoords[0][j].y));
				}
			}

			// Copy normals
			//gui->logMessage("Loading mesh normals");
			if (mesh->HasNormals()) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_normals.push_back(glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z));
				}
			}

			// Copy colors
			//gui->logMessage("Loading mesh colors");
			if (mesh->HasVertexColors(0)) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_colors.push_back(glm::u8vec3(mesh->mColors[0][j].r * 255, mesh->mColors[0][j].g * 255, mesh->mColors[0][j].b * 255));
				}
			}

			vertex_offset += mesh->mNumVertices;
		}

		// Load the combined mesh data
		load(all_vertices.data(), all_vertices.size(), all_indices.data(), all_indices.size());

		if (!all_texCoords.empty()) {
			loadTexCoords(all_texCoords.data(), all_texCoords.size());
		}

		if (!all_normals.empty()) {
			loadNormals(all_normals.data(), all_normals.size());
		}

		if (!all_colors.empty()) {
			loadColors(all_colors.data(), all_colors.size());
		}

		aiReleaseImport(scene);
	}
	else {
		// Handle error
		//cout << "Error loading mesh: " << file_path << endl;
	}
}










