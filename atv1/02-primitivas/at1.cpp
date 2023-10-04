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

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.5f,  0.5f, 0.0f
};


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
    	glDrawArrays(type_primitive, 0, 3);

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
    std::cout << "X: " << cx << " Y: " << cy << std::endl;

    // Shift vertices
    for(uint8_t i = 0; i < 6; i++)
    {
        vertices[i] = vertices[i+3];
    }
    vertices[6] = cx;
    vertices[7] = cy;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

void findAreaAndOrientation()
{
    float area =    vertices[0]*vertices[4]         //x1y2
                    - vertices[3]*vertices[1]       //x2y1
                    + vertices[3]*vertices[7]       //x2y3
                    - vertices[6]*vertices[4]       //x3y2
                    + vertices[6]*vertices[1]       //x3y1
                    - vertices[0]*vertices[7];      //x1y3
    
    int8_t orientation = (area > 0.0f)?1:-1;
    std::cout << "Area: " << area*orientation << " Orientation: " << std::to_string(orientation) << std::endl;
}


void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        createPoint(x,y);
        findAreaAndOrientation();
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
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
    type_primitive = GL_TRIANGLES;
	glutMainLoop();
}
