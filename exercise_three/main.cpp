#include <iostream>
#include <random>
#include <time.h>
#include <algorithm>
#include <chrono>
#include "spring.h"

using namespace std;

glm::vec3 camera{0, 30, -20};
glm::vec3 view_up{0, 1, 0};

chrono::steady_clock::time_point last_time{chrono::steady_clock::now()};
bool e_gravity = false;
bool wired_object = true;
bool full_sphere = true;
vector<particle> particles;
vector<spring> springs;
particle sphere{-100, -100, -100, 0};

enum class type{
    none = 0,
    flag,
    square,
    tetrahedon
};
type current_type;

// Invoked in each step by gui
void draw_next_step(){
    const auto k = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - last_time);

    // Draw springs
    for_each(begin(springs), end(springs), [&](spring& s){
        s.update(particles, sphere);
        if(wired_object)
            s.draw(particles);
    });

    // Draw particles
    for_each(begin(particles), end(particles), [&](particle& p){
        p.set_age(k.count());
        p.move(sphere);
        if(wired_object)
            p.draw();
    });

    // Draw sphere
    if(sphere.get_mass() > 0)
        sphere.draw();

    if(!wired_object){
        switch (current_type){
        case type::flag:
            {
                bool meta = false;
                for(unsigned i = 21; i < 200; i++){
                    if(i % 20 != 0){
                        const auto p1 = particles[i].get_coordinates();
                        const auto p2 = particles[i-1].get_coordinates();
                        const auto p3 = particles[i-20].get_coordinates();
                        const auto p4 = particles[i-21].get_coordinates();
                        if(meta)
                            glColor3f(0.9, 0.4, 0.4);
                        else
                            glColor3f(1, 0.5, 0.5);
                        meta = !meta;
                        glBegin(GL_TRIANGLE_FAN);
                        glVertex3f(p1.x, p1.y, p1.z);
                        glVertex3f(p2.x, p2.y, p2.z);
                        glVertex3f(p4.x, p4.y, p4.z);
                        glVertex3f(p3.x, p3.y, p3.z);
                        glEnd();
                    }
                }
            }
            break;
        case type::tetrahedon:
            {
                const auto p1 = particles[0].get_coordinates();
                const auto p2 = particles[1].get_coordinates();
                const auto p3 = particles[2].get_coordinates();
                const auto p4 = particles[3].get_coordinates();
                
                glColor3f(0.9, 0.4, 0.4);
                glBegin(GL_TRIANGLES);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
                glVertex3f(p3.x, p3.y, p3.z);
                glColor3f(1, 0.5, 0.5);
                glEnd();
                glBegin(GL_TRIANGLES);
                glVertex3f(p4.x, p4.y, p4.z);
                glVertex3f(p2.x, p2.y, p2.z);
                glVertex3f(p3.x, p3.y, p3.z);
                glEnd();
                glColor3f(0.8, 0.3, 0.3);
                glBegin(GL_TRIANGLES);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
                glVertex3f(p4.x, p4.y, p4.z);
                glEnd();
                glColor3f(1, 0.4, 0.3);
                glBegin(GL_TRIANGLES);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p4.x, p4.y, p4.z);
                glVertex3f(p3.x, p3.y, p3.z);
                glEnd();
            }
            break;
        case type::square:
            {
                vector<vector<int>> sets;
                sets.push_back({0,1,2,3});
                sets.push_back({0,6,5,1});
                sets.push_back({0,3,8,5});
                sets.push_back({6,7,2,1});
                sets.push_back({3,2,7,8});
                sets.push_back({5,8,7,6});
                bool meta = false;
                for(auto& s : sets){
                    const auto p1 = particles[s[0]].get_coordinates();
                    const auto p2 = particles[s[1]].get_coordinates();
                    const auto p3 = particles[s[2]].get_coordinates();
                    const auto p4 = particles[s[3]].get_coordinates();
                    if(meta)
                        glColor3f(0.9, 0.4, 0.4);
                    else
                        glColor3f(1, 0.5, 0.5);
                    meta = !meta;
                    glBegin(GL_QUADS);
                    glVertex3f(p1.x, p1.y, p1.z);
                    glVertex3f(p2.x, p2.y, p2.z);
                    glVertex3f(p3.x, p3.y, p3.z);
                    glVertex3f(p4.x, p4.y, p4.z);
                    glEnd();
                }
            }
            break;
        default:
            break;
        }
    }

    last_time = chrono::steady_clock::now();
}

void draw_floor(){
    glPushMatrix();

    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(0, -1, 0);

    //Floor
    glBegin(GL_QUADS);
    glVertex3f(-50, 0,-50);
    glVertex3f(-50, 0, 50);
    glVertex3f( 50, 0, 50);
    glVertex3f( 50, 0,-50);
    glEnd();

    glPopMatrix();
}

void update_function(int val){
    glutTimerFunc(0, update_function, 105);
    glutPostRedisplay();
}

void reshape(int width, int height){
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100, width / height, 10, 100);

    glMatrixMode(GL_MODELVIEW);

    glutPostRedisplay();
}

void draw_function(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    // Look at the center
    gluLookAt(camera.x, camera.y, camera.z, 
        0.0, 10, 10,
        view_up.x, view_up.y, view_up.z);

	draw_floor();
    draw_next_step();

    glutSwapBuffers();
    glutTimerFunc(0, update_function, 105);
}

const double delta = 0.5;

void key_pressed(unsigned char key, int x, int y) {
    switch (key) {
	case 'w':
		camera.y += delta;
        if(camera.y > 50)
            camera.y = 50;
		break;
	case 's':
		camera.y -= delta;
        if(camera.y < 2)
            camera.y = 2;
		break;
	case 'a':
		camera.x += delta;
        if(camera.x > 50)
            camera.x = 50;
		break;
	case 'd':
		camera.x -= delta;
        if(camera.x < -50)
            camera.x = -50;
		break;
    case 'k':
        sphere = particle{20, 15, 0, 5};
        sphere.set_sphere();
        break;
    case 'p':
        cout << camera.x << " " << camera.y << " " << camera.z << endl;
        break;
	default:
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y){
    switch (key){
    case GLUT_KEY_UP:
        sphere.move_up(0.5);
        break;
    case GLUT_KEY_DOWN:
        sphere.move_down(0.5);
        break;
    case GLUT_KEY_LEFT:
        sphere.move_left(0.5);
        break;
    case GLUT_KEY_RIGHT:
        sphere.move_right(0.5);
        break;
    
    default:
        break;
    }
    glutPostRedisplay();
}

void menu_callback(int entry){
    switch (entry){
    case 100:
        e_gravity = !e_gravity;
        for_each(begin(particles), end(particles), [=](particle& p){
            p.set_gravity(e_gravity);
        });
        break;
    case 200:
        wired_object = !wired_object;
        break;
    case 300:
        full_sphere = !full_sphere;
        sphere.set_full_sphere(full_sphere);
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void tetrahedon(){
    current_type = type::tetrahedon;

    particles.emplace_back(  5, 25, 5, 1);
    particles.emplace_back(  5, 20, 0, 1);
    particles.emplace_back(  0, 25, 0, 1);
    particles.emplace_back(0.5, 20, 5, 1);
    springs.emplace_back(0, 1, 40, 5);
    springs.emplace_back(0, 2, 40, 5);
    springs.emplace_back(0, 3, 40, 5);
    springs.emplace_back(1, 2, 40, 5);
    springs.emplace_back(1, 3, 40, 5);
    springs.emplace_back(2, 3, 40, 5);

    //particles[0].set_debug();
}

void square(){
    current_type = type::square;

    particles.emplace_back(-2, 24, -2, 1);
    particles.emplace_back(-2, 24,  2, 1);
    particles.emplace_back( 2, 24,  2, 1);
    particles.emplace_back( 2, 24, -2, 1);
    particles.emplace_back( 0, 22,  0, 1);
    particles.emplace_back(-2, 20, -2, 1);
    particles.emplace_back(-2, 20,  2, 1);
    particles.emplace_back( 2, 20, -2, 1);
    particles.emplace_back( 2, 20,  2, 1);

    // jello - 200
    // steel - to 3000
    // 0 - sand
    const auto k = 200;

    springs.emplace_back(0, 1, k, 4);
    springs.emplace_back(0, 3, k, 4);
    springs.emplace_back(0, 5, k, 4);
    springs.emplace_back(1, 2, k, 4);
    springs.emplace_back(1, 6, k, 4);
    springs.emplace_back(2, 3, k, 4);
    springs.emplace_back(2, 7, k, 4);
    springs.emplace_back(3, 8, k, 4);
    springs.emplace_back(5, 6, k, 4);
    springs.emplace_back(6, 7, k, 4);
    springs.emplace_back(7, 8, k, 4);
    springs.emplace_back(8, 5, k, 4);
    springs.emplace_back(0, 4, k, 3.4641);
    springs.emplace_back(1, 4, k, 3.4641);
    springs.emplace_back(2, 4, k, 3.4641);
    springs.emplace_back(3, 4, k, 3.4641);
    springs.emplace_back(5, 4, k, 3.4641);
    springs.emplace_back(6, 4, k, 3.4641);
    springs.emplace_back(7, 4, k, 3.4641);
    springs.emplace_back(8, 4, k, 3.4641);
}

void flag(){
    current_type = type::flag;

    for(double row = 25; row > 20; row-=0.5){
        for(double column = -5; column < 5; column+=0.5){
            particles.emplace_back(1, row, column, 1);
            const auto index = particles.size()-1;
            if(row > 24.59)
                particles.back().make_fixed();
            else
                springs.emplace_back(index, index - 20, 50, 0.5);
            if (column > -5)
                springs.emplace_back(index, index - 1, 50, 0.5);

        }
    }

    for (unsigned i = 20; i < particles.size(); i++){
        if(i % 20 != 0)
            springs.emplace_back(i, i - 21, 30, 0.70711);
        if(i % 20 != 19)
            springs.emplace_back(i, i - 19, 30, 0.70711);
    }

}

int main(int argc, char **argv){
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glDepthMask(GL_TRUE);
    glutInit(&argc, argv);
    glutCreateWindow("Material modelling");
    glutFullScreen();
    glutDisplayFunc(draw_function);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_pressed);
    glutSpecialFunc(special);

    int menu = glutCreateMenu(menu_callback);
    glutSetMenu(menu);
    glutAddMenuEntry("Toggle gravity", 100);
    glutAddMenuEntry("Show wire", 200);
    glutAddMenuEntry("Full sphere", 300);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

	glScalef(1, -1, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

    flag();
    //tetrahedon();
    //square();
    
    for_each(begin(particles), end(particles), [&](particle& p){
        p.set_gravity(e_gravity);
    });

    glutMainLoop();

    return 0;
}

