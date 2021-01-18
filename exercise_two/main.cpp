#include "SOIL.h"
#include <glm/glm.hpp>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <random>
#include <time.h>
#include <algorithm>
#include <chrono>

using namespace std;

glm::vec3 camera{0, 1, -2};
glm::vec3 view_up{0, 1, 0};
unsigned initial_emission = 50;

chrono::steady_clock::time_point last_time{chrono::steady_clock::now()};

enum class direction{
    none = 0,
    left,
    right
};
direction wind = direction::none;
bool triangles = false;

unsigned load_texture(const string& name){
    auto texture = SOIL_load_OGL_texture(
        name.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y
    );

    if (texture == 0){
        cerr << "Unable to load texture" << endl;
        return 1;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return texture;
}


class particle{
    // Final destination of a particle
    glm::vec3 direction{0, -1, 0};
    double speed = 0.005; // Point per second

    glm::vec3 current_coordinates;

    // used for random motion
    default_random_engine gen;
    normal_distribution<double> n{0, 0.02};

    unsigned age{0};
    bool dirty = false;
public:
    particle(double x, double y, double z){
        current_coordinates = glm::vec3{x, y, z};
        gen = default_random_engine(time(NULL) + x);
    }

    string print() const{
        return "[ " + to_string(current_coordinates.x) + " , " + to_string(current_coordinates.y) + " , " + to_string(current_coordinates.z) + " ]" ;
    }

    void move() {
        auto delta_y = speed * age / 1000;
        // Fall downwards
        current_coordinates.y -= abs(delta_y);
        // And move randomly
        current_coordinates.x += n(gen);
        current_coordinates.z += n(gen);

        switch (wind){
        case direction::left:
            current_coordinates.x -= 0.05;
            break;
        case direction::right:
            current_coordinates.x += 0.05;
            break;
        default:
            break;
        }

        // Dont go beyond the wall
        if(current_coordinates.x > 4.5)
            current_coordinates.x = 4.5;
        if(current_coordinates.x < -4.5)
            current_coordinates.x = -4.5;

    }   

    void draw(){
        float modelview[16];
        glPushMatrix();
        glTranslatef(current_coordinates.x, current_coordinates.y, current_coordinates.z);
        
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

        glScalef(0.2, 0.2, 0.2);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
        glEnd();
        
        glPopMatrix();
    }

    // Vector orientation bilboarding
    void draw_2(){
        glPushMatrix();
        glTranslatef(current_coordinates.x, current_coordinates.y, current_coordinates.z);
        glScalef(0.2, 0.2, 0.2);
        
        const auto object_camera = camera ;
        const auto projection = glm::normalize(glm::vec3{object_camera.x, 0, object_camera.z});
        const auto object_camera_n = glm::normalize(object_camera);

        const glm::vec3 obj_ori{0, 0, -1};
        const auto alpha = glm::acos(glm::dot(obj_ori, projection));

        if(camera.x > 0)
            glRotatef( glm::degrees(alpha), view_up.x, -view_up.y, view_up.z);
        else   
            glRotatef( glm::degrees(alpha), view_up.x, view_up.y, view_up.z);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
        glEnd();

        glPopMatrix();
    }

    // Dead is when it falls to the ground
    bool is_dead(){
        return current_coordinates.y <= -1;
    }

    // Simulates the rebirth of a particle
    void reset(double x, double y, double z){
        current_coordinates = glm::vec3{x, y, z};
        age = 0;
    }

    void set_age(unsigned num){
        age = num;
    }

    void set_dirty(){
        dirty = true;
    }

    bool is_dirty() const{
        return dirty;
    }
};

class particle_generator{
    double _x, _y, _z, _p;
    double height{5};

    vector<particle> particles;
    default_random_engine gen;
    uniform_real_distribution<double> x_dice;
    uniform_real_distribution<double> z_dice;

    chrono::microseconds delta_t;
public:
    particle_generator(){};

    particle_generator(double x, double y, double z, double p) : _x{x}, _y{y}, _z{z}, _p{p}{
        gen = default_random_engine(time(NULL));
        x_dice = uniform_real_distribution<double>(x, z);
        z_dice = uniform_real_distribution<double>(y, p);
    }

    // Invoked in each step by gui
    void draw_next_step(){
        auto texture = load_texture("snow.bmp");
        
        // This ensures that there is a maximum number of particles in the system
        if (particles.size() < initial_emission)
            if (x_dice(gen) > x_dice(gen) + x_dice(gen))
                particles.emplace_back(x_dice(gen), height, z_dice(gen));

        unsigned curr_dirty = 0;

        for_each(begin(particles), end(particles), [&](particle& p) {
            p.set_age(delta_t.count());
            p.move();
            p.draw_2();
            if(p.is_dead())
                if(particles.size() > initial_emission - curr_dirty){
                    p.set_dirty();
                    curr_dirty++;
                }else
                    p.reset(x_dice(gen), height, z_dice(gen));
        });
        auto it = remove_if(begin(particles), end(particles), [](const particle& p){
            return p.is_dirty();
        });
        if(it != end(particles))
            particles.erase(it);
    }

    // Clouds(generators are on height 5)
    void draw_triangles() const{
        glPushMatrix();

        glColor3f(0.6, 0.6, 0.6);
        glTranslatef(0, height, 0);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(_x, 0.0f, _y); 
        glVertex3f(_z, 0.0f, _y); 
        glVertex3f(_x, 0.0f, _p); 
        glVertex3f(_z, 0.0f, _p);
        glEnd();

        glPopMatrix();
    }

    void set_time(const chrono::milliseconds& point){
        delta_t = point;
    }
};


void draw_floor(){
    glPushMatrix();

    glColor3f(1, 1, 1);
    glTranslatef(0, -1, 0);

    auto texture = load_texture("floor.jpg");

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5, 0,-5);
    glTexCoord2f(2.0f, 0.0f); glVertex3f(-5, 0, 5);
    glTexCoord2f(2.0f, 2.0f); glVertex3f( 5, 0, 5);
    glTexCoord2f(0.0f, 2.0f); glVertex3f( 5, 0,-5);
    glEnd();

    glPopMatrix();
}

void update_function(int val){
    glutTimerFunc(0, update_function, 105);
    glutPostRedisplay();
}

// Main particle generator, initialized in main
particle_generator generator;

void draw_function(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-5, 5, -5, 5, 10, -10);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    // Look at the center
    gluLookAt(camera.x, camera.y, camera.z, 
        0.0, 0.0, 0.0,
        view_up.x, view_up.y, view_up.z);

	draw_floor();

    if(triangles)
        generator.draw_triangles();
    auto now = chrono::steady_clock::now();
    generator.set_time(chrono::duration_cast<chrono::milliseconds>(now - last_time));
    last_time = now;
    generator.draw_next_step();

    glutSwapBuffers();
}

const double delta = 0.03;

void key_pressed(unsigned char key, int x, int y) {
    switch (key) {
	case 'w':
		camera.y += delta;
        if(camera.y > 5)
            camera.y = 5;
		break;
	case 's':
		camera.y -= delta;
        if(camera.y < 0.2)
            camera.y = 0.2;
		break;
	case 'a':
		camera.x += delta;
        if(camera.x > 3)
            camera.x = 3;
		break;
	case 'd':
		camera.x -= delta;
        if(camera.x < -3)
            camera.x = -3;
		break;
	case 'f':
        glutTimerFunc(0, update_function, 105);
		break;
    case 'k':
        if(initial_emission < 11)
            initial_emission -= 10;
        break;
    case 'l':
        initial_emission += 10;
        break;
    case 'p':
        cout << camera.x << " " << camera.y << " " << camera.z << endl;
        break;
	default:
		break;
	}
	glutPostRedisplay();
}

void menu_callback(int entry){
    switch (entry){
    case 100:
        wind = direction::left;
        break;
    case 200:
        wind = direction::right;
        break;
    case 300:
        wind = direction::none;
        break;
    case 400:
        triangles = !triangles;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char **argv){
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInit(&argc, argv);
    glutCreateWindow("Particle model");
    glutFullScreen();
    glutDisplayFunc(draw_function);
    glutKeyboardFunc(key_pressed);

    int menu = glutCreateMenu(menu_callback);
    glutSetMenu(menu);
    glutAddMenuEntry("Toggle wind - left", 100);
    glutAddMenuEntry("Toggle wind - right", 200);
    glutAddMenuEntry("No wind", 300);
    glutAddMenuEntry("Show generator", 400);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

	glScalef(1, -1, 1);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    
    // Smooth texture edges
    glEnable(GL_BLEND);
    glBlendEquation(GL_MAX);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    generator = particle_generator(-4, -4, 4, 4);

    glutMainLoop();

    return 0;
}

