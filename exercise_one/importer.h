#include<string>
#include<vector>
#include<glm/glm.hpp>
#include<assimp/scene.h>
#include <GL/glut.h>

struct colour{
    const GLfloat R, G, B;
    colour(float r, float g, float b) : R{r}, G{g}, B{b} { 
    };
};

class mesh{
    std::vector<glm::vec3> vertices;
    std::vector<struct colour> colours;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> faces;
    std::vector<double> bounds{-100, 100, -100, 100, -100, 100}; //{maxx, minx, maxy, miny, maxz, minz} 
    GLfloat get_random_channel();
    void check_bounding_box(const aiVector3D& vertex);
public:
    mesh(aiMesh* mesh);
    mesh(const mesh& other) = default;
    mesh& operator=(const mesh& mesh) = default;

    const std::vector<glm::vec3>& get_vertices() const;
    const std::vector<glm::vec3>& get_faces() const;
    //bool is_face_visible(const glm::vec3& face,
    //    const glm::vec3& normal,
    //    const glm::vec3& camera_position) const;
    const std::vector<glm::vec3>& get_normals() const;
    const std::vector<struct colour>& get_colours() const;
    const std::vector<double>& get_bounding_box() const;
};


class model {
public:
    model() = default;
    model(const std::string& path);

    std::vector<mesh> meshes;
    void print_model_stats() const;
    
private:
    void load_model(const std::string& path);
};