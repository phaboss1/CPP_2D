#pragma once

#include <iostream>
#include <vector>
#include <cassert>
#include <assimp/cimport.h>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags



class ModelDescription {
public:
	std::string modelName;
	std::string textureName;

	// Indices
	unsigned int mNumFaces;
	int* mIndices = nullptr;

	// Vertices
	unsigned int mNumVertices;
	float* mVertices = nullptr;

	// Textures
	unsigned int mNumTextures;
	float* mTextures = nullptr;

	ModelDescription(std::string modelName, std::string textureName)
	{
		// Get model name and texture name
		this->modelName = modelName;
		this->textureName = textureName;
	}

	~ModelDescription()
	{
		delete mIndices;
		delete mVertices;
		delete mTextures;
	}
};

class AssimpUtils {
public:
	static ModelDescription* LoadColladaModel(std::string path)
	{
		// Load file
		const aiScene* pScene = aiImportFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenBoundingBoxes);

		// File not found
		assert(pScene != nullptr);

		// Model must have only a single mesh
		assert(pScene->mNumMeshes == 1);

		// Model must have only a single material
		assert(pScene->mNumMaterials == 1);

		// Model must have only a single texture
		aiString aiTextureName;
		assert(pScene->mMaterials[0]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), aiTextureName) == aiReturn::aiReturn_SUCCESS);

		// Mesh must have only a single UV component
		assert(pScene->mMeshes[0]->mNumUVComponents[1] == 0);

		// The UV component must be only 2 dimentional
		assert(pScene->mMeshes[0]->mNumUVComponents[0] == 2);

		//// Create TexturedModel
		std::string textureName = path.substr(path.find_last_of("/") + 1);
		ModelDescription* modelDescription = new ModelDescription(textureName.substr(0, path.find_first_of(".")), std::string(aiTextureName.C_Str()));

		// Build indices array and get numFaces
		BuildIndices(modelDescription, pScene);

		//// Build vertices array and get numVertices
		BuildVertices(modelDescription, pScene);

		//// Build textured array and get numTextures
		BuildTexturesArray(modelDescription, pScene);

		aiReleaseImport(pScene);

		return modelDescription;
	}

private:
	static void BuildIndices(ModelDescription* modelDescription, const aiScene* scene)
	{
		modelDescription->mNumFaces = scene->mMeshes[0]->mNumFaces;
		modelDescription->mIndices = new int[modelDescription->mNumFaces * 3];
		for (unsigned int i = 0; i < modelDescription->mNumFaces; i++)
		{
			modelDescription->mIndices[i * 3 + 0] = scene->mMeshes[0]->mFaces[i].mIndices[0];
			modelDescription->mIndices[i * 3 + 1] = scene->mMeshes[0]->mFaces[i].mIndices[1];
			modelDescription->mIndices[i * 3 + 2] = scene->mMeshes[0]->mFaces[i].mIndices[2];
		}
	}

	static void BuildVertices(ModelDescription* modelDescription, const aiScene* scene)
	{
		modelDescription->mNumVertices = scene->mMeshes[0]->mNumVertices;
		modelDescription->mVertices = new float[modelDescription->mNumVertices * 3];
		for (unsigned int i = 0; i < modelDescription->mNumVertices; i++)
		{
			modelDescription->mVertices[i * 3 + 0] = scene->mMeshes[0]->mVertices[i].x;
			modelDescription->mVertices[i * 3 + 1] = scene->mMeshes[0]->mVertices[i].y;
			modelDescription->mVertices[i * 3 + 2] = scene->mMeshes[0]->mVertices[i].z;
		}
	}

	static void BuildTexturesArray(ModelDescription* modelDescription, const aiScene* scene)
	{
		modelDescription->mNumTextures = modelDescription->mNumVertices;
		modelDescription->mTextures = new float[modelDescription->mNumTextures * 2];
		for (unsigned int i = 0; i < modelDescription->mNumTextures; i++)
		{
			modelDescription->mTextures[i * 2 + 0] = scene->mMeshes[0]->mTextureCoords[0][i].x;
			modelDescription->mTextures[i * 2 + 1] = scene->mMeshes[0]->mTextureCoords[0][i].y;
		}
	}
};
