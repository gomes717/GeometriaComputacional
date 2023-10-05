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
#include "../include/dcel.hpp"
#include <vector>
#include <array>
#include <map>


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


std::vector<std::array<float,3>> points = {};

std::vector<Vertex_t> vertex;
std::vector<HalfEdge_t> h;
std::vector<VertexTable_t> ver_tab;
std::vector<HalfEdgeTable_t> half_edge_table;
std::map<int,std::vector<int>> adj;
std::vector<bool> unvisited_half_edge;
std::vector<Face_t> face;
std::vector<FaceTable_t> face_table;

bool creating_initial_pol = true;

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

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if(!creating_initial_pol)
    {
        std::cout << "Num faces: " << face.size() << std::endl;
        std::cout << "Num aresta: " << half_edge_table.size() << std::endl;
        std::cout << "Num aresta face 0: " << face_table[0].inner_components.size() << std::endl;
        glBegin(GL_LINES);
        for (HalfEdgeTable_t het : half_edge_table)
        {
            std::cout << het.incident_face->key << std::endl;
            glColor3f(het.he->orig->r, het.he->orig->g, het.he->orig->b); 
            glVertex2f(het.he->orig->x,het.he->orig->y);
            glColor3f(het.he->twin->orig->r, het.he->twin->orig->g, het.he->twin->orig->b); 
            glVertex2f(het.he->twin->orig->x, het.he->twin->orig->y);
        }
        glEnd();
    }
    glFlush();
}

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

    std::array<float, 3> aux_point = {cx, cy, 0.0f};
    points.push_back(aux_point);
}


float area2(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    float area =    v1[0]*v2[1]                     //x1y2
                    - v2[0]*v1[1]                   //x2y1
                    + v2[0]*v3[1]                   //x2y3
                    - v3[0]*v2[1]                   //x3y2
                    + v3[0]*v1[1]                   //x3y1
                    - v1[0]*v3[1];                  //x1y3
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
    for(int i = 0; i < points.size() - 1; i++)
    {
        if((points[i] != v1 && points[i] != v2) && (points[i + 1] != v1 && points[i + 1] != v2)
            && intersect(v1, v2, points[i], points [i+1]))
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

bool polygonOrientation()
{
    if(vertex.size() < 3)
        return false;
    static uint16_t time = 0;
    float tot_area = 0.0f;
    const Vertex_t anchor = {.x = 1.1f, .y = 1.1f};
    for(int i = 0; i < vertex.size() - 1; i++)
    {
        tot_area += area2(anchor, vertex[i], vertex[i+1]);
    }
    tot_area += area2(anchor, vertex[points.size() -1], vertex[0]);
    int8_t orientation = (tot_area > 0.0f)?1:-1;
    std::cout << "Area: " << tot_area*orientation << " Orientation: " << ((orientation > 0.0f)?"counterclockwise":"clockwise") << std::endl;
    return (orientation > 0.0f);
}

bool isConvex(std::array<float,3> v1, std::array<float,3> v2, std::array<float,3> v3)
{
    float tri_area = area2(v1, v2, v3);
    return tri_area > 0.0f;
}

void verifyAngle()
{
    if(points.size() < 3)
        return;

    bool is_convex = isConvex(points[points.size() - 1], points[0], points[1]);
    std::cout << "points " << points.size() - 1 <<" is " << ((is_convex)?"convex":"reflex") << std::endl;

    for(int i = 1; i < points.size() - 1; i++)
    {
        is_convex =   isConvex(points[i - 1], points[i], points[i+1]);
        std::cout << "Vertex " << i <<" is " << ((is_convex)?"convex":"reflex") << std::endl;
    }
    

    is_convex = isConvex(points[points.size() - 2], points[points.size() - 1], points[0]);
    std::cout << "Vertex " << points.size() - 1 <<" is " << ((is_convex)?"convex":"reflex") << std::endl;
}

bool ear(std::array<float,3> v0,std::array<float,3> v,std::array<float,3> v1,std::array<float,3> v00,std::array<float,3> v11)
{
    if(!isConvex(v0, v, v1))
    {
        return false;
    }
    return diagonal(v00, v0, v, v, v1, v11);
}

void earVerify()
{
    if(points.size() < 4)
        return;
    std::cout << "Vertex " << 0 <<" is " << ((ear(points[points.size() - 1], points[0], points[1], points[points.size() - 2], points[2]))?"ear":"not ear") << std::endl;
    std::cout << "Vertex " << 1 <<" is " << ((ear(points[0], points[1], points[2], points[points.size() - 1], points[3]))?"ear":"not ear") << std::endl;
    for(int i = 2; i < points.size() - 2; i++)
    {
        std::cout << "Vertex " << i<<" is " << ((ear(points[i-1], points[i], points[i+1], points[i-2], points[i+2]))?"ear":"not ear") << std::endl;
    }
    std::cout << "Vertex " << points.size() - 2 <<" is " << ((ear(points[points.size() - 3], points[points.size() - 2], points[points.size() - 1], points[points.size() - 4], points[0]))?"ear":"not ear") << std::endl;
    std::cout << "Vertex " << points.size() - 1 <<" is " << ((ear(points[points.size() - 2], points[points.size() - 1], points[0], points[points.size() - 3], points[1]))?"ear":"not ear") << std::endl;
}

void createInitialPolygon()
{
    int i = 0;
    for (std::array<float,3> v : points)
    {
        std::cout << "Coordinates of " << i << " vertex : " << std::endl;
        Vertex_t aux_v = {.x = v[0], .y = v[1], .r = 1.0f, .g = 1.0f, .b = 1.0f, .key = i++};
        vertex.push_back(aux_v);
    }

    bool counterclockwise = polygonOrientation();
    if(counterclockwise)
    {
        for(int j = 0; j < vertex.size() - 1; j++)
        {
            HalfEdge_t* he = new HalfEdge_t;
            HalfEdge_t* twin = new HalfEdge_t;
            adj[j].push_back(j+1);
            adj[j+1].push_back(j);
            createEdge(vertex[j], vertex[j+1], *he, *twin);
            h.push_back(*he);
            h.push_back(*twin);
            unvisited_half_edge.push_back(false);
            unvisited_half_edge.push_back(false);
        }

        HalfEdge_t* he = new HalfEdge_t;
        HalfEdge_t* twin = new HalfEdge_t;
        adj[vertex.size()-1].push_back(0);
        adj[0].push_back(vertex.size()-1);
        createEdge(vertex[vertex.size()-1], vertex[0], *he, *twin);
        h.push_back(*he);
        h.push_back(*twin);
        unvisited_half_edge.push_back(false);
        unvisited_half_edge.push_back(false);
    }
    else
    {
        for(int j = vertex.size() - 1; j > 0; j--)
        {
            
            HalfEdge_t* he = new HalfEdge_t;
            HalfEdge_t* twin = new HalfEdge_t;
            adj[j].push_back(j-1);
            adj[j-1].push_back(j);
            createEdge(vertex[j], vertex[j-1], *he, *twin);
            h.push_back(*he);
            h.push_back(*twin);
            unvisited_half_edge.push_back(false);
            unvisited_half_edge.push_back(false);
        }

        HalfEdge_t* he = new HalfEdge_t;
        HalfEdge_t* twin = new HalfEdge_t;
        adj[vertex.size()-1].push_back(0);
        adj[0].push_back(vertex.size()-1);
        createEdge(vertex[0], vertex[vertex.size()-1], *he, *twin);
        h.push_back(*he);
        h.push_back(*twin);
        unvisited_half_edge.push_back(false);
        unvisited_half_edge.push_back(false);
    }
    creating_initial_pol = false;
}

void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        createPoint(x,y);
        //findAreaAndOrientation();
        //verifyAngle();
        //earVerify();
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
                    points.clear();
                    break;
                case 'c':
                {
                    createInitialPolygon();
                    fillVertexTable(ver_tab, h, vertex, adj);
                    fillHalfEdgeTable(half_edge_table, h, unvisited_half_edge, vertex, adj, face, face_table);
                    fillFaceTableInnerComponents(face_table, h, half_edge_table, face, vertex);
                    break;
                }
                case 'q':
                case 'Q':
                    glutLeaveMainLoop();
        }
    
	glutPostRedisplay();
}

void initGL() {
   // Set "clearing" or background color
   glClearColor(0.2, 0.1, 0.3, 1.0); // Black and opaque
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);          // Initialize GLUT
   glutInitWindowSize(win_width, win_height);   // Set the window's initial width & height
   glutCreateWindow("Vertex, Primitive & Color");  // Create window with the given title
   glutDisplayFunc(display);       // Register callback handler for window re-paint event
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMouseFunc(mouse);
   initGL();                       // Our own OpenGL initialization
   glutMainLoop();                 // Enter the event-processing loop
   return 0;
}

