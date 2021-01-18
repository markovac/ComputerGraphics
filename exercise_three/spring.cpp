#include "spring.h"
#include <iostream>

using namespace std;

spring::spring(unsigned _i, unsigned _j, double _k, double _l) : i{_i}, j{_j}, k{_k}, l{_l}{
}

void spring::update(std::vector<particle>& particles, const particle& sphere) const{
    const auto pos1 = particles[i].get_coordinates();
    const auto pos2 = particles[j].get_coordinates();

    // Update particle forces
    const auto spring_vector = glm::normalize(pos2 - pos1);
    const auto delta_x = glm::distance(pos2, pos1) - l;
    const auto hooks_coef = -k * delta_x;

    const auto one_mass = particles[j].get_mass();

    // F = hook * spring_vec / mass
    glm::vec3 new_force = multiply(1 / one_mass, multiply(hooks_coef, spring_vector));

    particles[i].add_force(-new_force);
    particles[j].add_force(new_force);

}

void spring::draw(std::vector<particle>& particles) const{
    glColor3f(0.2, 0.9, 0.2);
    const auto pos1 = particles[i].get_coordinates();
    const auto pos2 = particles[j].get_coordinates();
    glBegin(GL_LINES);
    glVertex3f(pos1.x, pos1.y, pos1.z);
    glVertex3f(pos2.x, pos2.y, pos2.z);
    glEnd();
}