#include "MeshImporter.h"
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

static mat4 aiMat4ToMat4(const aiMatrix4x4& aiMat) {
	mat4 mat;
	for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) mat[c][r] = aiMat[r][c];
	return mat;
}

static void decomposeMatrix(const mat4& matrix, vec3& scale, glm::quat& rotation, vec3& translation) {
	// Extraer la traslación
	translation = vec3(matrix[3]);

	// Extraer la escala
	scale = vec3(
		length(vec3(matrix[0])),
		length(vec3(matrix[1])),
		length(vec3(matrix[2]))
	);

	// Remover la escala de la matriz
	mat4 rotationMatrix = matrix;
	rotationMatrix[0] /= scale.x;
	rotationMatrix[1] /= scale.y;
	rotationMatrix[2] /= scale.z;

	// Extraer la rotación
	rotation = glm::quat(vec3(0.0f, 0.0f, 0.0f));
}

std::string removeLastPartOfPath(const std::string& path) {
	std::filesystem::path fsPath(path);
	fsPath.remove_filename();
	return fsPath.string();
}

std::vector<std::shared_ptr<Mesh>> MeshImporter::ImportMesh(const aiScene& scene)
{
	vector<shared_ptr<Mesh>> meshes;
	for (unsigned int i = 0; i < scene.mNumMeshes; ++i) {
		const aiMesh* fbx_mesh = scene.mMeshes[i];
		auto mesh_ptr = make_shared<Mesh>();

		vector<unsigned int> indices(fbx_mesh->mNumFaces * 3);
		for (unsigned int j = 0; j < fbx_mesh->mNumFaces; ++j) {
			indices[j * 3 + 0] = fbx_mesh->mFaces[j].mIndices[0];
			indices[j * 3 + 1] = fbx_mesh->mFaces[j].mIndices[1];
			indices[j * 3 + 2] = fbx_mesh->mFaces[j].mIndices[2];
		}

		mesh_ptr->load(reinterpret_cast<const glm::vec3*>(fbx_mesh->mVertices), fbx_mesh->mNumVertices, indices.data(), indices.size());
		if (fbx_mesh->HasTextureCoords(0)) {
			vector<glm::vec2> texCoords(fbx_mesh->mNumVertices);
			for (unsigned int j = 0; j < fbx_mesh->mNumVertices; ++j) texCoords[j] = glm::vec2(fbx_mesh->mTextureCoords[0][j].x, fbx_mesh->mTextureCoords[0][j].y);
			mesh_ptr->loadTexCoords(texCoords.data(), texCoords.size());
		}
		if (fbx_mesh->HasNormals()) mesh_ptr->loadNormals(reinterpret_cast<glm::vec3*>(fbx_mesh->mNormals), fbx_mesh->mNumVertices);
		if (fbx_mesh->HasVertexColors(0)) {
			vector<glm::u8vec3> colors(fbx_mesh->mNumVertices);
			for (unsigned int j = 0; j < fbx_mesh->mNumVertices; ++j) colors[j] = glm::u8vec3(fbx_mesh->mColors[0][j].r * 255, fbx_mesh->mColors[0][j].g * 255, fbx_mesh->mColors[0][j].b * 255);
			mesh_ptr->loadColors(colors.data(), colors.size());
		}
		meshes.push_back(mesh_ptr);
	}
	return meshes;
}

std::vector<std::shared_ptr<Material>> MeshImporter::createMaterialsFromFBX(const aiScene& scene, const fs::path& basePath) {

	std::vector<std::shared_ptr<Material>> materials;
	map<string, std::shared_ptr<Image>> images;

	for (unsigned int i = 0; i < scene.mNumMaterials; ++i) {
		const auto* fbx_material = scene.mMaterials[i];
		auto material = make_shared<Material>();

		if (fbx_material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString texturePath;
			fbx_material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
			const string textureFileName = fs::path(texturePath.C_Str()).filename().string();
			const auto image_itr = images.find(textureFileName);
			if (image_itr != images.end()) material->texture.setImage(image_itr->second);
			else {
				std::string imagePath = removeLastPartOfPath(basePath.string()) + textureFileName;
				auto image = std::make_shared<Image>();
				image = textureImporter.ImportTexture(imagePath);
				//images.insert({ textureFileName, image });
				material->texture.setImage(image);
			}

			auto uWrapMode = aiTextureMapMode_Wrap;
			auto vWrapMode = aiTextureMapMode_Wrap;
			fbx_material->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), uWrapMode);
			fbx_material->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), vWrapMode);
			assert(uWrapMode == aiTextureMapMode_Wrap);
			assert(vWrapMode == aiTextureMapMode_Wrap);

			unsigned int flags = 0;
			fbx_material->Get(AI_MATKEY_TEXFLAGS_DIFFUSE(0), flags);
			assert(flags == 0);
		}

		aiColor4D color;
		fbx_material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->color = color4(color.r * 255, color.g * 255, color.b * 255, color.a * 255);

		materials.push_back(material);
	}
	return materials;
}

bool containsSubstring(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
}

GameObject MeshImporter::gameObjectFromNode(const aiScene& scene, const aiNode& node, const vector<shared_ptr<Mesh>>& meshes, const vector<shared_ptr<Material>>& materials) {
	GameObject go;

	mat4 localMatrix = aiMat4ToMat4(node.mTransformation);

	// Decompose the transformation matrix
	vec3 scale, translation;
	glm::quat rotation;
	decomposeMatrix(localMatrix, scale, rotation, translation);

	// Apply the transformation matrix directly
	go.GetComponent<TransformComponent>()->transform().SetLocalMatrix(localMatrix);

	go.name = node.mName.C_Str();

	if (containsSubstring(go.name, "$AssimpFbx$_Translation"))
	{
		_translation = translation;
	}
	else if (!containsSubstring(go.name, "$AssimpFbx$"))
	{
		go.GetComponent<TransformComponent>()->transform().SetPosition(_translation);
		go.GetComponent<TransformComponent>()->transform().SetRotation(vec3(-90, 0, 0));
		_translation = vec3(0.0f);
	}

	go.meshPath = "";
	go.texturePath = "";

	for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
		const auto* fbx_mesh = scene.mMeshes[node.mMeshes[i]];
		auto mesh = meshes[node.mMeshes[i]];
		auto material = materials[fbx_mesh->mMaterialIndex];
		go.AddComponent<MeshLoader>();
		go.GetComponent<MeshLoader>()->SetMesh(mesh);
		go.GetComponent<MeshLoader>()->SetMaterial(material);
		go.GetComponent<MeshLoader>()->SetColor(material->color);
		go.GetComponent<MeshLoader>()->GetMaterial()->texture = material->texture;
		go.GetComponent<MeshLoader>()->SetImage(material->texture.image());
		auto texture = make_shared<Texture>();
		*texture = go.GetComponent<MeshLoader>()->GetMaterial()->texture;
		go.GetComponent<MeshLoader>()->SetTexture(texture);
		meshGameObjects.push_back(std::make_shared<GameObject>(go));

	}

	for (unsigned int i = 0; i < node.mNumChildren; ++i) {
		gameObjectFromNode(scene, *node.mChildren[i], meshes, materials);
	}

	return go;
}

// SaveMeshToFile function
void MeshImporter::SaveMeshToFile(const std::vector<std::shared_ptr<Mesh>>& meshes, const std::string& filePath)
{
	// Check if the directory exists
	std::filesystem::path path(filePath);
	if (!std::filesystem::exists(path.parent_path())) {
		std::filesystem::create_directories(path.parent_path());
	}

	std::ofstream outFile(filePath, std::ios::binary);
	if (!outFile.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}

	// Serialize the FBX path
	size_t fbxPathSize = filePath.size();
	outFile.write(reinterpret_cast<const char*>(&fbxPathSize), sizeof(fbxPathSize));
	outFile.write(filePath.c_str(), fbxPathSize);

	// Serialize the number of meshes
	size_t meshCount = meshes.size();
	outFile.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

	for (const auto& mesh : meshes) {
		bool isNull = (mesh == nullptr);
		outFile.write(reinterpret_cast<const char*>(&isNull), sizeof(isNull));
		if (isNull) continue;

		// Serialize vertices
		const auto& vertices = mesh->vertices();
		size_t verticesSize = vertices.size();
		outFile.write(reinterpret_cast<const char*>(&verticesSize), sizeof(verticesSize));
		outFile.write(reinterpret_cast<const char*>(vertices.data()), verticesSize * sizeof(glm::vec3));

		// Serialize indices
		const auto& indices = mesh->indices();
		size_t indicesSize = indices.size();
		outFile.write(reinterpret_cast<const char*>(&indicesSize), sizeof(indicesSize));
		outFile.write(reinterpret_cast<const char*>(indices.data()), indicesSize * sizeof(unsigned int));

		// Serialize texture coordinates
		const auto& texCoords = mesh->texCoords();
		size_t texCoordsSize = texCoords.size();
		outFile.write(reinterpret_cast<const char*>(&texCoordsSize), sizeof(texCoordsSize));
		outFile.write(reinterpret_cast<const char*>(texCoords.data()), texCoordsSize * sizeof(glm::vec2));

		// Serialize colors
		const auto& colors = mesh->colors();
		size_t colorsSize = colors.size();
		outFile.write(reinterpret_cast<const char*>(&colorsSize), sizeof(colorsSize));
		outFile.write(reinterpret_cast<const char*>(colors.data()), colorsSize * sizeof(glm::u8vec3));

		// Serialize bounding box
		const auto& boundingBox = mesh->boundingBox();
		outFile.write(reinterpret_cast<const char*>(&boundingBox.min), sizeof(boundingBox.min));
		outFile.write(reinterpret_cast<const char*>(&boundingBox.max), sizeof(boundingBox.max));
	}

	outFile.close();
}

// LoadMeshFromFile function
std::vector<std::shared_ptr<Mesh>> MeshImporter::LoadMeshFromFile(const std::string& filePath, std::string& fbxPath)
{
	std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + filePath);
	}

	// Deserialize the FBX path
	size_t fbxPathSize;
	inFile.read(reinterpret_cast<char*>(&fbxPathSize), sizeof(fbxPathSize));
	fbxPath.resize(fbxPathSize);
	inFile.read(fbxPath.data(), fbxPathSize);

	// Deserialize the meshes using the operator>>
	std::vector<std::shared_ptr<Mesh>> meshes;
	inFile >> meshes;

	inFile.close();
	return meshes;
}

// Function to return the FBX path
std::string MeshImporter::GetFBXPath(const std::string& filePath)
{
	std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + filePath);
	}

	// Deserialize the FBX path
	size_t fbxPathSize;
	inFile.read(reinterpret_cast<char*>(&fbxPathSize), sizeof(fbxPathSize));
	std::string fbxPath(fbxPathSize, '\0');
	inFile.read(fbxPath.data(), fbxPathSize);

	inFile.close();
	return fbxPath;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::shared_ptr<Mesh>>& meshes)
{

	size_t meshCount = meshes.size();
	os.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
	for (const auto& mesh : meshes) {
		bool isNull = (mesh == nullptr);
		os.write(reinterpret_cast<const char*>(&isNull), sizeof(isNull));
		if (isNull) continue;

		// Serialize vertices
		const auto& vertices = mesh->vertices();
		size_t verticesSize = vertices.size();
		os.write(reinterpret_cast<const char*>(&verticesSize), sizeof(verticesSize));
		os.write(reinterpret_cast<const char*>(vertices.data()), verticesSize * sizeof(glm::vec3));

		// Serialize indices
		const auto& indices = mesh->indices();
		size_t indicesSize = indices.size();
		os.write(reinterpret_cast<const char*>(&indicesSize), sizeof(indicesSize));
		os.write(reinterpret_cast<const char*>(indices.data()), indicesSize * sizeof(unsigned int));

		// Serialize texture coordinates
		const auto& texCoords = mesh->texCoords();
		size_t texCoordsSize = texCoords.size();
		os.write(reinterpret_cast<const char*>(&texCoordsSize), sizeof(texCoordsSize));
		os.write(reinterpret_cast<const char*>(texCoords.data()), texCoordsSize * sizeof(glm::vec2));

		// Serialize colors
		const auto& colors = mesh->colors();
		size_t colorsSize = colors.size();
		os.write(reinterpret_cast<const char*>(&colorsSize), sizeof(colorsSize));
		os.write(reinterpret_cast<const char*>(colors.data()), colorsSize * sizeof(glm::u8vec3));

		// Serialize bounding box
		const auto& boundingBox = mesh->boundingBox();
		os.write(reinterpret_cast<const char*>(&boundingBox.min), sizeof(boundingBox.min));
		os.write(reinterpret_cast<const char*>(&boundingBox.max), sizeof(boundingBox.max));
	}
	return os;
}

std::istream& operator>>(std::istream& is, std::vector<std::shared_ptr<Mesh>>& meshes)
{
	size_t meshCount;
	is.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
	meshes.resize(meshCount);

	for (size_t i = 0; i < meshCount; ++i) {
		bool isNull;
		is.read(reinterpret_cast<char*>(&isNull), sizeof(isNull));
		if (isNull) {
			meshes[i] = nullptr;
			continue;
		}

		auto mesh = std::make_shared<Mesh>();

		// Deserialize vertices
		size_t verticesSize;
		is.read(reinterpret_cast<char*>(&verticesSize), sizeof(verticesSize));
		std::vector<glm::vec3> vertices(verticesSize);
		is.read(reinterpret_cast<char*>(vertices.data()), verticesSize * sizeof(glm::vec3));

		// Deserialize indices
		size_t indicesSize;
		is.read(reinterpret_cast<char*>(&indicesSize), sizeof(indicesSize));
		std::vector<unsigned int> indices(indicesSize);
		is.read(reinterpret_cast<char*>(indices.data()), indicesSize * sizeof(unsigned int));

		// Deserialize texture coordinates
		size_t texCoordsSize;
		is.read(reinterpret_cast<char*>(&texCoordsSize), sizeof(texCoordsSize));
		std::vector<glm::vec2> texCoords(texCoordsSize);
		is.read(reinterpret_cast<char*>(texCoords.data()), texCoordsSize * sizeof(glm::vec2));

		// Deserialize colors
		size_t colorsSize;
		is.read(reinterpret_cast<char*>(&colorsSize), sizeof(colorsSize));
		std::vector<glm::u8vec3> colors(colorsSize);
		is.read(reinterpret_cast<char*>(colors.data()), colorsSize * sizeof(glm::u8vec3));

		// Load the mesh data
		mesh->load(vertices.data(), vertices.size(), indices.data(), indices.size());
		if (!texCoords.empty()) {
			mesh->loadTexCoords(texCoords.data(), texCoords.size());
		}
		if (!colors.empty()) {
			mesh->loadColors(colors.data(), colors.size());
		}

		// Deserialize bounding box
		glm::vec3 min, max;
		is.read(reinterpret_cast<char*>(&min), sizeof(min));
		is.read(reinterpret_cast<char*>(&max), sizeof(max));
		BoundingBox boundingBox{ min, max };
		// Assuming Mesh has a method to set bounding box
		// mesh->setBoundingBox(boundingBox);

		meshes[i] = mesh;
	}
	return is;
}