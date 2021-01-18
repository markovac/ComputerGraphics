#include "importer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <algorithm>

using namespace std;

GLfloat mesh::get_random_channel(){
    return static_cast<float>(rand() % 100) / 100 ;
}

void mesh::check_bounding_box(const aiVector3D& vertex){
    if(bounds[0] < vertex.x)
        bounds[0] = vertex.x;
    if(bounds[1] > vertex.x)
        bounds[1] = vertex.x;
    if(bounds[2] < vertex.y)
        bounds[2] = vertex.y;
    if(bounds[3] > vertex.y)
        bounds[3] = vertex.y;
    if(bounds[4] < vertex.z)
        bounds[4] = vertex.z;
    if(bounds[5] > vertex.z)
        bounds[5] = vertex.z;
}

mesh::mesh(aiMesh* mesh){
    unsigned size = mesh->mNumVertices;
    for(unsigned i = 0; i < size; i++){
        const auto vertex = mesh->mVertices[i];
        vertices.emplace_back(vertex.x, vertex.y, vertex.z);
        check_bounding_box(vertex);
        //colours.emplace_back(get_random_channel(), get_random_channel(), get_random_channel());
        colours.emplace_back(0.01, 1, 0.01);
    }

    size = mesh->mNumFaces;
    for(unsigned i = 0; i < size; i++){
        const auto face = mesh->mFaces[i].mIndices;
        
        faces.emplace_back(
            face[0], 
            face[1], 
            face[2]);
        auto vector1 = vertices[face[1]] - vertices[face[0]];
		auto vector2 = vertices[face[2]] - vertices[face[0]];
        normals.push_back(glm::cross(vector1, vector2));
    }
}

const vector<glm::vec3>& mesh::get_vertices() const{
    return vertices;
}

const vector<glm::vec3>& mesh::get_faces() const{
    return faces;
}

const vector<glm::vec3>& mesh::get_normals() const{
    return normals;
}

const vector<struct colour>& mesh::get_colours() const{
    return colours;
}

const std::vector<double>& mesh::get_bounding_box() const{
    return bounds;
}

/*
bool mesh::is_face_visible(const glm::vec3& face,
        const glm::vec3& normal,
        const glm::vec3& camera_position) const{
	// Calculate Face center cooridnates
    const auto X = vertices[face.x];
    const auto Y = vertices[face.y];
    const auto Z = vertices[face.z];
    glm::vec3 center{X + Y + Z};
    center /= 3;
    
    const auto view = camera_position - center;
	return dot(view, normal) < 0;
}
*/


model::model(const string& path){
    load_model(path);
}

void model::load_model(const string& path){
    Assimp::Importer imp;
    auto scene = imp.ReadFile(path, aiProcess_Triangulate 
        | aiProcess_GenBoundingBoxes);
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Invalid object file: " << imp.GetErrorString() << std::endl;
        return;
    }
    
    for(unsigned int i = 0; i < scene->mNumMeshes; i++){
        auto mesh = scene->mMeshes[i]; 
        meshes.emplace_back(mesh);			
    }
}


void model::print_model_stats() const{
    cout << "Number of loaded meshes: " << meshes.size() << endl;
    cout << "Listing mesh info :\n";
    const auto& ver = meshes[0].get_vertices();
    const auto& fac = meshes[0].get_faces();
    const auto& nor = meshes[0].get_normals();

    unsigned vernum=0;
    unsigned fanum=0;
    unsigned nornum=0;
    
    std::for_each(std::begin(ver), std::end(ver), 
    [&](const glm::vec3& element){
        cout << element.x << ";" << element.y << ";" << element.z << endl;
        vernum++;
    });

    cout << "\n------------------------------------\n\n";

    std::for_each(std::begin(fac), std::end(fac), 
    [&](const glm::vec3& element){
        cout << element.x << ";" << element.y << ";" << element.z << endl;
        fanum++;
    });

    cout << "\n------------------------------------\n\n";

    std::for_each(std::begin(nor), std::end(nor), 
    [&](const glm::vec3& element){
        cout << element.x << ";" << element.y << ";" << element.z << endl;
        nornum++;
    });

    cout << "\n\n\tTotal vertices: \t" << vernum;
    cout << "\n\tTotal faces: \t" << fanum;
    cout << "\n\tTotal normals: \t" << nornum << endl;
}