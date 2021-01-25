#include "Model.h"

//https://realitymultiplied.wordpress.com/2016/07/24/assimp-skeletal-animation-tutorial-3-something-about-skeletons/

aiMatrix4x4 GLMMat4ToAi(glm::mat4 mat)
{
	return aiMatrix4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

glm::mat4 AiToGLMMat4(aiMatrix4x4& in_mat)
{
	glm::mat4 tmp;
	tmp[0][0] = in_mat.a1;
	tmp[1][0] = in_mat.b1;
	tmp[2][0] = in_mat.c1;
	tmp[3][0] = in_mat.d1;

	tmp[0][1] = in_mat.a2;
	tmp[1][1] = in_mat.b2;
	tmp[2][1] = in_mat.c2;
	tmp[3][1] = in_mat.d2;

	tmp[0][2] = in_mat.a3;
	tmp[1][2] = in_mat.b3;
	tmp[2][2] = in_mat.c3;
	tmp[3][2] = in_mat.d3;

	tmp[0][3] = in_mat.a4;
	tmp[1][3] = in_mat.b4;
	tmp[2][3] = in_mat.c4;
	tmp[3][3] = in_mat.d4;
	return tmp;
}

Model::Model()
{
}

Model::~Model()
{
}

void Model::Load(const char* filepath) {

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, 
		aiProcess_Triangulate | 
		aiProcess_FlipUVs | 
		aiProcess_CalcTangentSpace | 
		aiProcess_GenSmoothNormals | 
		aiProcess_JoinIdenticalVertices);

	if (!scene) {
		printf("[MODEL LOADER]: %s\n", importer.GetErrorString());
		return;
	}
	else {
		//printf("[MODEL LOADER]: Opened - %s \n", filepath);

		LoadNode(scene->mRootNode, scene);
	}

}

//void Model::Move(const glm::vec3 position)
//{
//	for (auto& m : m_meshes)
//		m->Move(position);
//}
//
//void Model::Rotate(const glm::vec3 rotation)
//{
//	for (auto& m : m_meshes)
//		m->SetRotation(rotation);
//}
//
//void Model::Scale(const glm::vec3 scale)
//{
//	for (auto& m : m_meshes)
//		m->Scale(scale);
//}
//
//void Model::SetPosition(const glm::vec3 position)
//{
//	for (auto& m : m_meshes)
//		m->SetPosition(position);
//}

void Model::SetRotation(const glm::vec3 rotation)
{

}

void Model::SetScale(const glm::vec3 scale)
{
}

void Model::LoadNode(aiNode* node, const aiScene* scene) {

	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}

}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene) {


	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
	   // std::cout << mesh->mVertices[i].x << " " << mesh->mVertices[i].y << " " << mesh->mVertices[i].z << "\n";
		m_vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
	}
	//std::cout << "Vert count: " << mesh->mNumVertices << "\n";

	for (size_t i = 0; i < mesh->mNumFaces; i++) {

		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			//std::cout << face.mIndices[j] << "\n";
			m_indices.push_back(face.mIndices[j]);
		}
	}
}

