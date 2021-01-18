#include "particle.h"
#include <iostream>

using namespace std;



glm::vec3 inline multiply(double K, const glm::vec3& vec){
    return {vec.x * K, vec.y * K, vec.z * K};
}




particle::particle(double x, double y, double z, double m) : mass{m}{
    current_coordinates = glm::vec3{x, y, z};
}

string particle::print() const{
    return "[ " + to_string(current_coordinates.x) + " , " + to_string(current_coordinates.y) + " , " + to_string(current_coordinates.z) + " ]" ;
}

void particle::move(const particle& sp) {
    if(fixed)
        return;

    glm::vec3 direction_vector = sum_force;
    // Damping = factor * speed / mass
    glm::vec3 damping = multiply(1 / mass, multiply(-constants::damping_factor, speed_vector));

    direction_vector += damping;

    if(e_gravity)
        direction_vector += multiply(1 / mass, constants::gravity);

    speed_vector += multiply(age, direction_vector);
    current_coordinates += multiply(age, speed_vector);

    // Detect floor colision
    if(current_coordinates.y < -1){
        current_coordinates.y = -1;
        speed_vector.y = -speed_vector.y / 2;
        if(glm::abs(speed_vector.y - 1) < 0.001)
            dead = true;
    }
    
/*
    // Detect ball collision - with displacement
    const auto d = glm::distance(current_coordinates, sp.current_coordinates);
    if ( d < constants::ball_radius){
        const auto vec = glm::normalize(current_coordinates - sp.current_coordinates);
        current_coordinates.x += vec.x * (constants::ball_radius - d);
        current_coordinates.y += vec.y * (constants::ball_radius - d);
        current_coordinates.z += vec.z * (constants::ball_radius - d);
    }
*/

    // Reset particle forces
    sum_force = {0, 0, 0};

    // Detect ball collision - with forces
    if ( glm::distance(current_coordinates, sp.current_coordinates) < constants::ball_radius){
        set_speed(multiply(0.5, speed_vector + glm::normalize(current_coordinates - sp.current_coordinates)));
        move(sp);
    }
        
    if(debug){
        cout << "damping vector:\t\t\t[ " + to_string(damping.x) + " , " + to_string(damping.y) + " , " + to_string(damping.z) + " ]" << endl;
        cout << "accumulated direction vector:\t[ " + to_string(direction_vector.x) + " , " + to_string(direction_vector.y) + " , " + to_string(direction_vector.z) + " ]" << endl;
        cout << "accumulated speed vector:\t[ " + to_string(speed_vector.x) + " , " + to_string(speed_vector.y) + " , " + to_string(speed_vector.z) + " ]" << endl;
        cout << "calculeted coordinates:\t\t" << print() << endl << endl;
    }
}   

void particle::draw(){
    float modelview[16];
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(current_coordinates.x, current_coordinates.y, current_coordinates.z);


    if(sphere && full_sphere){
        glColor3f(0.4, 0.3, 0.9);
        glutSolidSphere(constants::ball_radius, 10, 10);
    }else{
        glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
        // undo all rotations and scaling
        for(unsigned i = 0; i < 3; i++ )
            for(unsigned j = 0; j < 3; j++ ) {
                if( i == j )
                    modelview[i*4+j] = 1.0;
                else
                    modelview[i*4+j] = 0.0;
            }   
        glLoadMatrixf(modelview);

        glColor3f(0.8, 0.1, 0.1);
        glScalef(0.2, 0.2, 0.2);
        glBegin(GL_QUADS);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f( 1.0f, -1.0f, 0.0f);
        glVertex3f( 1.0f,  1.0f, 0.0f);
        glVertex3f(-1.0f,  1.0f, 0.0f);
        glEnd();
    }
        
    glPopMatrix();
}

void particle::set_age(unsigned num){
    age = num / 1000.0 ;
}

bool particle::is_dead() const{
    return dead;
}

void particle::set_gravity(bool g){
    e_gravity = g;
}

glm::vec3 particle::get_coordinates() const{
    return current_coordinates;
}

void particle::add_force(const glm::vec3& f){
    sum_force += f;
}

void particle::reset_forces(){
    sum_force = glm::vec3{0, 0, 0};
}

void particle::set_debug(){
    debug = !debug;
}

void particle::make_fixed(){
    fixed = !fixed;
}

double particle::get_mass() const{
    return mass;
}

bool particle::is_sphere() const{
    return sphere;
}

void particle::set_sphere(){
    sphere = true;
}

void particle::set_full_sphere(bool s){
    full_sphere = s;
}

void particle::set_speed(const glm::vec3& speed){
    speed_vector = speed;
}

void particle::move_up(const double w){
    current_coordinates.y += w;
}

void particle::move_down(const double w){
    current_coordinates.y -= w;
}

void particle::move_left(const double w){
    current_coordinates.x += w;
}

void particle::move_right(const double w){
    current_coordinates.x -= w;
}

void particle::set_coordinates(const glm::vec3& co){
    current_coordinates = co;
}