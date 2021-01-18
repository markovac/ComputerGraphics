#include <string>
#include <glm/glm.hpp>
#include <GL/glut.h>

namespace constants{
    const glm::vec3 gravity{0, -9.81, 0};
    const double damping_factor{0.8};
    const double ball_radius{5};
};

glm::vec3 inline multiply(double K, const glm::vec3& vec);

class particle{
    glm::vec3 sum_force{0, 0, 0};
    glm::vec3 speed_vector{0, 0, 0};
    glm::vec3 current_coordinates{0, 0, 0};

    double age{0};
    bool dead{false};
    bool e_gravity{false};
    double mass{0};
    bool debug{false};
    bool fixed{false};
    bool sphere{false};
    bool full_sphere{true};
public:
    particle(double x, double y, double z, double m);
    particle& operator=(const particle& p) = default;

    std::string print() const;
    void move(const particle& sp);  
    void draw();
    void set_age(unsigned num);
    bool is_dead() const;
    void set_gravity(bool g);
    glm::vec3 get_coordinates() const;
    void add_force(const glm::vec3& f);
    void reset_forces();
    void set_debug();
    void make_fixed();
    double get_mass() const;
    bool is_sphere() const;
    void set_sphere();
    void set_full_sphere(bool s);
    void set_speed(const glm::vec3& speed);

    void move_up(const double w);
    void move_down(const double w);
    void move_left(const double w);
    void move_right(const double w);

    void set_coordinates(const glm::vec3& co);
};