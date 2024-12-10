#include "FileManager.h"

GameObject FileManager::LoadFile(const char* path)
{
	// Load file
	std::string extension = getFileExtension(path);

	if (extension == "obj" || extension == "fbx" || extension == "dae" || extension == "FBX") {
		// Load Mesh
		const aiScene* fbx_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FlipUVs);
		auto meshes = meshImporter.ImportMesh(*fbx_scene);
		auto materials = meshImporter.createMaterialsFromFBX(*fbx_scene, path);
		GameObject go = meshImporter.gameObjectFromNode(*fbx_scene, *fbx_scene->mRootNode, meshes, materials);
		aiReleaseImport(fbx_scene);
		for (int i = 0; i < meshImporter.meshGameObjects.size(); i++)
		{
			auto gameObject = meshImporter.meshGameObjects[i];
			scene.emplaceChild(*gameObject);
			
		}
		std::string nameFile = getFileNameWithoutExtension(path);
		const std::string finalPath = "Library/Meshes/" + nameFile + ".mesh";
		meshImporter.SaveMeshToFile(meshes, finalPath.c_str());
		go.meshPath = path;

		// Set ID
		int newID = scene.children().back().id;
		go.id = newID + 1;
		return go;
		
	}
	else {
		std::cerr << "Unsupported file extension: " << extension << std::endl;
	}
}

void FileManager::ImportTexture(const char* path)
{
	// Load Texture
	auto imageTexture = std::make_shared<Image>();
	imageTexture = textureImporter.ImportTexture(path);
	std::string nameFile = getFileNameWithoutExtension(path);
	const std::string finalPath = "Library/Textures/" + nameFile + ".tex";
	textureImporter.SaveTextureToFile(imageTexture, finalPath.c_str());
}

void FileManager::LoadTexture(const char* path, GameObject& go)
{
	// Load Texture
	std::string extension = getFileExtension(path);
	auto imageTexture = std::make_shared<Image>();
	auto texture = std::make_shared<Texture>();
	if (extension == "tex")
	{
		imageTexture = textureImporter.LoadTextureFromFile(path);
	}
	else if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp")
	{
		imageTexture->LoadTexture(path);
	}
	
	go.texturePath = path;
	texture->setImage(imageTexture);
	go.GetComponent<MeshLoader>()->GetMesh()->deleteCheckerTexture();
	go.GetComponent<MeshLoader>()->SetImage(imageTexture);
	go.GetComponent<MeshLoader>()->SetTexture(texture);
}

void FileManager::LoadCustomFile(const char* path, GameObject& go)
{
	//// Load file
	//std::string extension = getFileExtension(path);

	//if (extension == "mesh") {
	//	// Load Mesh
	//	const aiScene* fbx_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FlipUVs);
	//	std::vector<std::shared_ptr<Mesh>> meshes;
	//	meshes = meshImporter.LoadMeshFromFile(path);
	//	go = meshImporter.gameObjectFromNode(*fbx_scene, *fbx_scene->mRootNode, meshes, {});
	//	go.meshPath = path;
	//	

	//	// Set ID
	//	int newID = scene.children().back().id;
	//	go.id = newID + 1;
	//}
	//else {
	//	std::cerr << "Unsupported file extension: " << extension << std::endl;
	//}
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

std::string FileManager::getFileNameWithoutExtension(const std::string& filePath) {
	// Find the last directory separator in the file path
	size_t sepPosition = filePath.find_last_of("/\\");
	// Find the last dot in the file path
	size_t dotPosition = filePath.rfind('.');

	// If no dot is found or the dot is before the last directory separator, return the whole file name
	if (dotPosition == std::string::npos || (sepPosition != std::string::npos && dotPosition < sepPosition)) {
		return filePath.substr(sepPosition + 1);
	}

	// Extract and return the file name without the extension
	return filePath.substr(sepPosition + 1, dotPosition - sepPosition - 1);
}


