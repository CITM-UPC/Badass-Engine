#include "FileManager.h"

void FileManager::LoadFile(const char* path, GameObject& go)
{
	// Load file
	std::string extension = getFileExtension(path);

	if (extension == "obj" || extension == "fbx" || extension == "dae") {
		// Load Mesh
		auto mesh = std::make_shared<Mesh>();
		mesh = meshImporter.ImportMesh(path);
		meshImporter.SaveMeshToFile(mesh, "Library/Meshes/mesh.mesh");
		//mesh->LoadFile(path);
		go.meshPath = path;
		go.AddComponent<MeshLoader>()->SetMesh(mesh);
		go.setMesh(mesh);

		// Set ID
		int newID = scene.children().back().id;
		go.id = newID + 1;

		
	}
	else {
		std::cerr << "Unsupported file extension: " << extension << std::endl;
	}
}

void FileManager::LoadTexture(const char* path, GameObject& go)
{
	// Load Texture
	auto imageTexture = std::make_shared<Image>();
	auto texture = std::make_shared<Texture>();
	imageTexture->LoadTexture(path);
	go.texturePath = path;
	texture->setImage(imageTexture);
	go.GetComponent<MeshLoader>()->GetMesh()->deleteCheckerTexture();
	go.GetComponent<MeshLoader>()->SetImage(imageTexture);
	go.GetComponent<MeshLoader>()->SetTexture(texture);
}

void FileManager::LoadCustomFile(const char* path, GameObject& go)
{
	// Load file
	std::string extension = getFileExtension(path);

	if (extension == "mesh") {
		// Load Mesh
		auto mesh = std::make_shared<Mesh>();
		mesh = meshImporter.LoadMeshFromFile(path);
		go.meshPath = path;
		go.AddComponent<MeshLoader>()->SetMesh(mesh);
		go.setMesh(mesh);

		// Set ID
		int newID = scene.children().back().id;
		go.id = newID + 1;
	}
	else {
		std::cerr << "Unsupported file extension: " << extension << std::endl;
	}
}

std::string FileManager::getFileExtension(const std::string& filePath)
{
	// Find the last dot in the file path
	size_t dotPosition = filePath.rfind('.');

	// If no dot is found, return an empty string
	if (dotPosition == std::string::npos) {
		return "";
	}

	// Extract and return the file extension
	return filePath.substr(dotPosition + 1);
}


