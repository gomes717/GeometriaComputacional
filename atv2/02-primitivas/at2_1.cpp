/**
 * @file primitives.cpp
 * Draws different primitives.
 *
 * Draws different primitives of OpenGL according to a pressed key:
 * 1 for points;
 * 2 for lines;
 * 3 for line strip;
 * 4 for line loop;
 * 5 for triangles;
 * 6 for triangle strip;
 * 7 for triangle fan.
 * 
 * @author Ricardo Dutra da Silva
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "../lib/utils.h"
#include <vector>
#include <array>


/* Globals */
/** Window width. */
int win_width  = 800;
/** Window height. */
int win_height = 600;

/** Program variable. */
int program;
/** Vertex array object. */
unsigned int VAO;
/** Vertex buffer object. */
unsigned int VBO;

/** Primitive type. */
int type_primitive = GL_POINTS;


std::vector<std::array<float,3>> vertex = {};

int8_t orientation = 0;


/** Vertex shader. */
const char *vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\0";

/* Functions. */
void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void initData(void);
void initShaders(void);

/** 
 * Drawing function.
 *
 * Draws primitive.
 */
void display()
{
    	glClearColor(0.2, 0.1, 0.3, 1.0);
    	glClear(GL_COLOR_BUFFER_BIT);

    	glUseProgram(program);
    	glBindVertexArray(VAO);
    	glDrawArrays(type_primitive, 0, vertex.size());

    	glutSwapBuffers();
}

/**
 * Reshape function.
 *
 * Called when window is resized.
 *
 * @param width New window width.
 * @param height New window height.
 */
void reshape(int width, int height)
{
    win_width = width;
    win_height = height;
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}

void createPoint(uint16_t x, uint16_t y)
{
    
    const float cx = (float)(x - win_width/2)/(win_width/2);
    const float cy = -(float)(y - win_height/2)/(win_height/2);
    //std::cout << "X: " << cx << " Y: " << cy << std::endl;

    std::array<float, 3> aux_vertex = {cx, cy, 0.0f};
    vertex.push_back(aux_vertex);
    std::cout << "size "  << sizeof(*(vertex.data()))*vertex.size() << std::endl;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(*(vertex.data()))*vertex.size()*3, vertex.data(), GL_DYNAMIC_DRAW);
}

float area2(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    float area =    v1[0]*v2[1]                      //x1y2
                    - v2[0]*v1[1]                    //x2y1
                    + v2[0]*v3[1]               //x2y3
                    - v3[0]*v2[1]               //x3y2
                    + v3[0]*v1[1]                  //x3y1
                    - v1[0]*v3[1];                 //x1y3
    return area;
}

bool left(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    return(area2(v1,v2,v3) > 0);
}

bool leftOn(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    return(area2(v1,v2,v3) >= 0);
}

bool collinear(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    return (area2(v1,v2,v3) == 0);
}

bool Xor(bool x, bool y)
{
    return !x ^ !y;
}

bool intersectionProp(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3, std::array<float,3> v4)
{
    if(collinear(v1,v2,v3) ||
            collinear(v1,v2,v4) ||
            collinear(v3, v4, v1) ||
            collinear(v3, v4, v2))
    {
         return false;
    }

    return (Xor(left(v1,v2,v3), left(v1,v2,v4)) && Xor(left(v3, v4, v1), left(v3, v4, v2)));
}

bool between(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    if(!collinear(v1, v2, v3))
    {
        return false;
    }
    if(v1[0] != v2[0])
    {
        return ((v1[0] <= v3[0]) && (v3[0] <= v2[0]) || (v1[0] >= v3[0]) && (v3[0] >= v2[0]));
    }
    else
    {
        return ((v1[1] <= v3[1]) && (v3[1] <= v2[1]) || (v1[1] >= v3[1]) && (v3[1] >= v2[1]));
    }
}

bool intersect(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3, std::array<float,3> v4)
{
    if(intersectionProp(v1, v2, v3, v4))
    {
        return true;
    }
    else if(between(v1,v2,v3) ||
            between(v1,v2,v4) ||
            between(v3, v4, v1) ||
            between(v3, v4, v2))
    {
        return true;
    }
    return false;

}

bool diagonalie(std::array<float,3> v1, std::array<float,3> v2)
{
    for(int i = 0; i < vertex.size() - 1; i++)
    {
        if((vertex[i] != v1 && vertex[i] != v2) && (vertex[i + 1] != v1 && vertex[i + 1] != v2)
            && intersect(v1, v2, vertex[i], vertex [i+1]))
        {
            return false;
        }
    }
    return true;
}

bool inCone(std::array<float,3> v0,std::array<float,3> v,std::array<float,3> v1,std::array<float,3> b0,std::array<float,3> b, std::array<float,3> b1)
{
    if(leftOn(v, v1, v0))
    {
        return (left(v,b,v0) && left(b,v,v1));
    }
    return !(leftOn(v,b,v1) & leftOn(b,v,v0));
}

bool diagonal(std::array<float,3> v0,std::array<float,3> v,std::array<float,3> v1,std::array<float,3> b0,std::array<float,3> b, std::array<float,3> b1)
{
    return (inCone(v0, v, v1, b0, b, b1) && inCone(b0,b,b1,v0,v,v1) && diagonalie(v, b));
}

void findAreaAndOrientation()
{
    if(vertex.size() < 3)
        return;
    static uint16_t time = 0;
    float tot_area = 0.0f;
    const std::array<float,3> anchor = {1.1f, 1.1f, 0.0f};
    for(int i = 0; i < vertex.size() - 1; i++)
    {
        float area = area2(anchor, vertex[i], vertex[i+1]);
        orientation = (area > 0.0f)?1:-1;
        //std::cout << "Area: " << area*orientation << " Orientation: " << std::to_string(orientation) << std::endl;
        tot_area += area;
    }
    tot_area += area2(anchor, vertex[vertex.size() -1], vertex[0]);
    orientation = (tot_area > 0.0f)?1:-1;
    std::cout << "Area: " << tot_area*orientation << " Orientation: " << ((orientation > 0.0f)?"clockwise":"counterclockwise") << std::endl;
}

bool isConvex(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    float tri_area = area2(v1, v2, v3);
    return tri_area*orientation > 0.0f;
}

void verifyAngle()
{
    if(vertex.size() < 3)
        return;

    bool is_convex = isConvex(vertex[vertex.size() - 1], vertex[0], vertex[1]);
    std::cout << "Vertex " << vertex.size() - 1 <<" is " << ((is_convex)?"convex":"reflex") << std::endl;

    for(int i = 1; i < vertex.size() - 1; i++)
    {
        is_convex =   isConvex(vertex[i - 1], vertex[i], vertex[i+1]);
        std::cout << "Vertex " << i <<" is " << ((is_convex)?"convex":"reflex") << std::endl;
    }
    

    is_convex = isConvex(vertex[vertex.size() - 2], vertex[vertex.size() - 1], vertex[0]);
    std::cout << "Vertex " << vertex.size() - 1<<" is " << ((is_convex)?"convex":"reflex") << std::endl;
}

bool ear(std::array<float,3> v0,std::array<float,3> v,std::array<float,3> v1,std::array<float,3> v00,std::array<float,3> v11)
{
    if(!isConvex(v0, v, v1))
    {
        return false;
    }
    return Xor(diagonal(v00, v0, v, v, v1, v11), orientation < 0);
}

void earVerify()
{
    if(vertex.size() < 4)
        return;
    std::cout << "Vertex " << 0 <<" is " << ((ear(vertex[vertex.size() - 1], vertex[0], vertex[1], vertex[vertex.size() - 2], vertex[2]))?"ear":"not ear") << std::endl;
    std::cout << "Vertex " << 1 <<" is " << ((ear(vertex[0], vertex[1], vertex[2], vertex[vertex.size() - 1], vertex[3]))?"ear":"not ear") << std::endl;
    for(int i = 2; i < vertex.size() - 2; i++)
    {
        std::cout << "Vertex " << i<<" is " << ((ear(vertex[i-1], vertex[i], vertex[i+1], vertex[i-2], vertex[i+2]))?"ear":"not ear") << std::endl;
    }
    std::cout << "Vertex " << vertex.size() - 2 <<" is " << ((ear(vertex[vertex.size() - 3], vertex[vertex.size() - 2], vertex[vertex.size() - 1], vertex[vertex.size() - 4], vertex[0]))?"ear":"not ear") << std::endl;
    std::cout << "Vertex " << vertex.size() - 1 <<" is " << ((ear(vertex[vertex.size() - 2], vertex[vertex.size() - 1], vertex[0], vertex[vertex.size() - 3], vertex[1]))?"ear":"not ear") << std::endl;
}


void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        createPoint(x,y);
        findAreaAndOrientation();
        verifyAngle();
        earVerify();
    }
    glutPostRedisplay();
}

/** 
 * Keyboard function.
 *
 * Called to treat pressed keys.
 *
 * @param key Pressed key.
 * @param x Mouse x coordinate when key pressed.
 * @param y Mouse y coordinate when key pressed.
 */
void keyboard(unsigned char key, int x, int y)
{
        switch (key)
        {
                case 27:
                        glutLeaveMainLoop();
                case 'd':
                    vertex.clear();
                    break;
                case 'q':
                case 'Q':
                        glutLeaveMainLoop();
        }
    
	glutPostRedisplay();
}

/**
 * Init vertex data.
 *
 * Defines the coordinates for vertices, creates the arrays for OpenGL.
 */
void initData()
{
    // Set triangle vertices.

    
    // Vertex array.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind Vertex Array Object.
    glBindVertexArray(0);
}

/** Create program (shaders).
 * 
 * Compile shaders and create the program.
 */
void initShaders()
{
    // Request a program and shader slots from GPU
    program = createShaderProgram(vertex_code, fragment_code);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(win_width,win_height);
	glutCreateWindow(argv[0]);
	glewInit();

    	// Init vertex data for the triangle.
    	initData();
    
    	// Create shaders.
    	initShaders();
	
    	glutReshapeFunc(reshape);
    	glutDisplayFunc(display);
    	glutKeyboardFunc(keyboard);
        glutMouseFunc(mouse);
    type_primitive = GL_LINE_LOOP;
	glutMainLoop();
}
