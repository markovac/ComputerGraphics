#include <iostream>
#include "importer.h"
#include <algorithm>
#include <random>
#include <time.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <thread>
#include <chrono>
#include <glm/gtx/norm.hpp>

using namespace std;

model m;
glm::vec3 camera{1, 0.5, 1};
glm::vec3 view_up{0, 1, 0};
bool draw_object = false;
bool draw_curve = false;
bool start_animation = false;
bool draw_tangent = false;
std::vector<glm::vec3> spline;
int start = -1;
std::vector<glm::vec3> spline_dots{};
std::vector<glm::vec3> spline_tangents{};
std::vector<glm::vec3> spline_normals{};

struct my_pair{
    glm::vec3 dot;
    glm::vec3 tan;
    glm::vec3 nor;
};
struct my_pair tangent_to_draw;

void update_function(int val);

// The middle matrix for spline segment calculation
const glm::mat4x4 B1{ -1.0/6, 1.0/2, -1.0/2, 1.0/6,
                       1.0/2,  -1.0,  1.0/2,   0.0,
                      -1.0/2,   0.0,  1.0/2,   0.0,
                       1.0/6, 4.0/6,  1.0/6,   0.0};

// The middle matrix for Interpolation spline segment calculation
const glm::mat4x4 B2{ -1.0/2,  3.0/2, -3.0/2,  1.0/2,
                         1.0, -5.0/2,    2.0, -1.0/2,
                      -1.0/2,    0.0,  1.0/2,    0.0,
                         0.0,    1.0,    0.0,    0.0};

glm::mat4x4 B;

void move_into_center(const vector<double>& box){
    glPushMatrix();
    // Move the object into center in the beginning
    const double x_offset = (box[0] + box[1])/2;
    const double y_offset = (box[2] + box[3])/2;
    const double z_offset = (box[4] + box[5])/2;
    glTranslatef(-x_offset, -y_offset, -z_offset);
    glPopMatrix();
    return;
}

void display_object(){
    // Load mesh data
    const auto mesh = m.meshes[0];
    const auto vertices = mesh.get_vertices();
    const auto faces = mesh.get_faces();
    auto clrs = mesh.get_colours().begin();
    auto it = mesh.get_normals().begin();
    const auto box = mesh.get_bounding_box();

    // Move the object in the center
    move_into_center(box); 

    // Scale to fit the box
    double min_by_axis = 6.0/(box[0] - box[1]);
    const double by_y = 6.0/(box[2] - box[3]);
    const double by_z = 6.0/(box[4] - box[5]);
    min_by_axis = min_by_axis < by_y ? by_y : min_by_axis;
    min_by_axis = min_by_axis < by_z ? by_z : min_by_axis;

    // Scale to have visible objects
    glScalef(min_by_axis,
        min_by_axis,
        min_by_axis);

    // Draw object along with the polygon borders
    std::for_each(std::begin(faces), std::end(faces),
        [&](const glm::vec3& face){
            // Show triangle
            const auto clr = *clrs;
            glColor3f(clr.R, clr.G, clr.B);
            glBegin(GL_TRIANGLES);
                glVertex3f(vertices[face.x].x, vertices[face.x].y, vertices[face.x].z);
                glVertex3f(vertices[face.y].x, vertices[face.y].y, vertices[face.y].z);
                glVertex3f(vertices[face.z].x, vertices[face.z].y, vertices[face.z].z);
            glEnd();

            // Show triangle border
            glColor3f(0,0,0);
            glBegin(GL_LINE_LOOP);
                glVertex3f(vertices[face.x].x, vertices[face.x].y, vertices[face.x].z);
                glVertex3f(vertices[face.y].x, vertices[face.y].y, vertices[face.y].z);
                glVertex3f(vertices[face.z].x, vertices[face.z].y, vertices[face.z].z);
            glEnd();

            it++;
            clrs++;
        });
}

inline glm::vec3 calculate_tangent_vector(double t, const glm::mat4x3& R){
    return R * B * glm::vec4{3*pow(t, 2), 2*t, 1, 0};
}

glm::vec3 calculate_normal_vector(double t, const glm::mat4x3& R){
    return R * B * glm::vec4{6*t, 2, 0, 0};
}

// Delta for t increment
const double delta = 0.02;

vector<glm::vec3> get_segment(const vector<glm::vec3>& points){
    vector<glm::vec3> results;

    const glm::mat4x3 R{points[0], points[1], points[2], points[3]};
    
    for(double t = 0.0; t <= 1; t+=delta){
        // Calculate current curve value
        const glm::vec4 T{pow(t, 3), pow(t, 2), t, 1};

        // Reversed ordering
        results.emplace_back(R * B * T);

        // Tangents
        spline_tangents.push_back(calculate_tangent_vector(t, R));

        // Normals
        spline_normals.push_back(calculate_normal_vector(t, R));
    }

    return results;
}

vector<glm::vec3> calculate_spline_dots(){
    if(!spline_dots.empty())
        return spline_dots;

    vector<glm::vec3> results;
    unsigned num_of_segments = spline.size() - 3;
    auto first = begin(spline);
    
    while(num_of_segments > 0){
        // Choose 4 neighbouring spline points
        const vector<glm::vec3> points{first, first+4};

        // Draw the segments
        const auto segments = get_segment(points);
        results.insert(end(results), begin(segments), end(segments));
        first++;
        num_of_segments--;
    }

    return results;
}

void display_curve(){
    // Display only the curve points
    glColor3f(0, 0, 0);
    for_each(begin(spline), end(spline), [&](const glm::vec3& point){
        glPushMatrix();
        glTranslatef(point.x, point.y, point.z);
        glutSolidSphere(0.2, 10, 10);
        glPopMatrix();
    });

    // Draw the curve
    const auto spline_segments = calculate_spline_dots();
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_STRIP);
    for_each(begin(spline_segments), end(spline_segments), 
        [](const glm::vec3& segment){
            glVertex3f(segment.x, segment.y, segment.z);
        });
    glEnd();
}

void displayMe(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    // Look at the center
    gluLookAt(camera.x, camera.y, camera.z, 
        0.0, 0.0, 0.0,
        view_up.x, view_up.y, view_up.z);
    
    // Draw curve if enabled
    if(draw_curve)
        display_curve();

    // Draw tangent if enabled
    if(draw_tangent){
        const auto dot = tangent_to_draw.dot;
        const auto tan = tangent_to_draw.tan;
        glColor3f(1, 0, 0);
        glBegin(GL_LINE_STRIP);
            glVertex3f(dot.x, dot.y, dot.z);
            glVertex3f(dot.x + tan.x, dot.y + tan.y, dot.z + tan.z);
        glEnd();
    }

    glPushMatrix();
    
    // Animate object motion
    if(start_animation){
        // Move the object along the curve
        const auto goal = tangent_to_draw.dot;
        glTranslatef(goal.x, goal.y, goal.z);

        // Rotate the object depending on current tangent value
        const auto tangent = tangent_to_draw.tan;

        const glm::vec3 s{0, 0, 1};
        const auto axis = glm::cross(s, tangent);
        const auto fi = glm::acos(glm::dot(tangent, s) / (glm::l2Norm(tangent) * glm::l2Norm(s)));

        glRotatef(glm::degrees(fi), axis.x, axis.y, axis.z);
    }

    // Draw object if enabled - dcm matrix is used on every vertex separately
    if(draw_object)
        display_object();

    glPopMatrix();
    glutSwapBuffers();
}

void reshape(int width, int height) {
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-50, 50, -50, 50, 50, -50);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		camera.y -= 0.1;
		break;
	case 's':
		camera.y += 0.1;
		break;
	case 'a':
		camera.x += 0.1;
		break;
	case 'd':
		camera.x -= 0.1;
		break;
	case 'r':
		camera.z -= 0.1;
		break;
	case 'f':
		camera.z += 0.1;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void menu_callback(int entry){
    switch (entry){
    case 100:
        cout << "Drawing loaded object";
        draw_object = !draw_object;
        break;
    case 200:
        cout << "Drawing b_spline";
        draw_curve = !draw_curve;
        break;
    case 300:
        if(!start_animation && !draw_tangent){
            cout << "Starting animation with cross product rotation";
            draw_curve = true;
            draw_object = true;
            start_animation = true;
            glutTimerFunc(0, update_function, 205);
        }
        break;
    case 350:
        if(!start_animation && !draw_tangent){
            cout << "Starting animation with dcm rotation";
            draw_curve = true;
            draw_object = true;
            start_animation = true;
            glutTimerFunc(0, update_function, 205);
        }
        break;
    case 400:
        if(!draw_tangent && !start_animation){
            cout << "Drawing tangents";
            draw_curve = true;
            glutTimerFunc(0, update_function, 105);
        }
        break;
    default:
        cout << "An invalid click occured : " << entry;
        break;
    }
    cout << endl;
    glutPostRedisplay();
}

void load_spline(const string& path){
    // The input format of the spline file is following
    // x1 y1 z1
    // x2 y2 z2
    // ...
    ifstream input;
    input.open(path);
    string line;
    double x, y, z;
    while(getline(input, line)){
        stringstream ss;
        ss.str(line);
        ss >> x >> y >> z;
        spline.emplace_back(x, y, z);
    }
    input.close();
}

void update_function(int val){
    // Tangent and object animation
    if(val == 105 || val == 205){
        const auto spline_dots_copy = calculate_spline_dots();
        const auto num_of_tangents = spline_dots_copy.size();
        start++;
        
        if(start >= num_of_tangents){
            start = -1;
            if(val == 105)
                draw_tangent = false;
            else
                start_animation = false;
        }
        else{
            tangent_to_draw.dot = spline_dots_copy[start];
            tangent_to_draw.tan = spline_tangents[start];
            tangent_to_draw.nor = spline_normals[start];
            draw_tangent = true;
            glutPostRedisplay();
            if(val == 105)
                glutTimerFunc(0, update_function, 105);
            else
                glutTimerFunc(0, update_function, 205);
        }
    }
}

int main(int argc, char** argv){
    if(argc < 4){
        cout << "Usage:\n\tmain <object_file> <spline_file> {interpolation | approximation} [print_object_info]" << endl;
        return 1;
    }
    m = model(argv[1]);
    
    load_spline(argv[2]);

    if(strcmp("interpolation", argv[3]) == 0)
        B = B2;
    else if(strcmp("approximation", argv[3]) == 0)
        B = B1;
    else{
        cout << "Invalid spline mode. Choose \"approximation\" or \"interpolation\"" << endl;
        return 1;
    }
        
    srandom(time(NULL));

    if(argc == 5){
        if(strcmp("1", argv[4]) == 0)
            m.print_model_stats();
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    auto win = glutCreateWindow("Animation curves");
    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    glutFullScreen();

    int menu = glutCreateMenu(menu_callback);
    glutSetMenu(menu);
    glutAddMenuEntry("Draw Object", 100);
    glutAddMenuEntry("Draw Curve", 200);
    glutAddMenuEntry("Start Animation (Cross-product rotation)", 300);
    glutAddMenuEntry("Draw tangents", 400);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

	glScalef(1, -1, 1);
    glEnable(GL_DEPTH_TEST);
    
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(displayMe);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}