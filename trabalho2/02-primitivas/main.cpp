#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "../lib/utils.h"
#include <vector>
#include <array>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <tgmath.h> 
#include <set>

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

typedef enum
{
    VERTEX,
    INT,
    INT_ENTER,
    INT_EXIT
} PointInfo;

std::vector<std::array<float,6>> polygon1 = {};
std::vector<std::array<float,3>> p1_color = {};

std::vector<std::array<float,6>> polygon2 = {};

std::vector<std::array<float,3>> poly_color = {{(float)std::rand()/RAND_MAX,(float)std::rand()/RAND_MAX,(float)std::rand()/RAND_MAX}};
std::vector<std::vector<std::array<float,6>>> polygons = {{}};

std::vector<std::array<float,6>> intersection_points = {};

std::vector<std::vector<std::array<float,6>>> int_polygons = {};

unsigned int actual_poly = 0;

int8_t orientation = 0;

/** Vertex shader. */
const char *vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"\n"
"out vec3 vColor;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position, 1.0);\n"
"    vColor = color;\n"
"}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
"#version 330 core\n"
"\n"
"in vec3 vColor;\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vec4(vColor, 1.0f);\n"
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
        glPointSize(1);
        for(int i = 0; i < polygons.size(); i++)
        {   
            glBufferData(GL_ARRAY_BUFFER, sizeof(*(polygons[i].data()))*polygons[i].size()*6, polygons[i].data(), GL_DYNAMIC_DRAW);
    	    glDrawArrays(type_primitive, 0, polygons[i].size());
        }

        for(std::vector<std::array<float,6>> int_poly : int_polygons)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(*(int_poly.data()))*int_poly.size()*6, int_poly.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(type_primitive, 0, int_poly.size());
        }
        glPointSize(10);  
        glBufferData(GL_ARRAY_BUFFER, sizeof(*(intersection_points.data()))*intersection_points.size()*6, intersection_points.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, intersection_points.size());


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

void createPoint(uint16_t x, uint16_t y, std::vector<std::array<float,6>>& polygon, std::array<float, 3> color)
{
    
    const float cx = (float)(x - win_width/2)/(win_width/2);
    const float cy = -(float)(y - win_height/2)/(win_height/2);
    //std::cout << "X: " << cx << " Y: " << cy << std::endl;

    std::array<float, 6> aux_vertex = {cx, cy, 0.0f};
    aux_vertex[3] = color[0];
    aux_vertex[4] = color[1];
    aux_vertex[5] = color[2];
    polygon.push_back(aux_vertex);
    std::cout << "size "  << sizeof(*(polygon.data()))*polygon.size() << std::endl;
}

float area2(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
{
    float area =    v1[0]*v2[1]                      //x1y2
                    - v2[0]*v1[1]                    //x2y1
                    + v2[0]*v3[1]               //x2y3
                    - v3[0]*v2[1]               //x3y2
                    + v3[0]*v1[1]                  //x3y1
                    - v1[0]*v3[1];                 //x1y3
    return area;
}

bool left(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
{
    return(area2(v1,v2,v3) > 0);
}

bool leftOn(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
{
    return(area2(v1,v2,v3) >= 0);
}

bool collinear(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
{
    return (area2(v1,v2,v3) == 0);
}

bool Xor(bool x, bool y)
{
    return !x ^ !y;
}

bool intersectionProp(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3, std::array<float,6> v4)
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

bool between(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
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

bool intersect(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3, std::array<float,6> v4)
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

// bool diagonalie(std::array<float,3> v1, std::array<float,3> v2)
// {
//     for(int i = 0; i < polygon1.size() - 1; i++)
//     {
//         if((polygon1[i] != v1 && polygon1[i] != v2) && (polygon1[i + 1] != v1 && polygon1[i + 1] != v2)
//             && intersect(v1, v2, polygon1[i], polygon1[i+1]))
//         {
//             return false;
//         }
//     }
//     return true;
// }

bool inCone(std::array<float,6> v0,std::array<float,6> v,std::array<float,6> v1,std::array<float,6> b0,std::array<float,6> b, std::array<float,6> b1)
{
    if(leftOn(v, v1, v0))
    {
        return (left(v,b,v0) && left(b,v,v1));
    }
    return !(leftOn(v,b,v1) & leftOn(b,v,v0));
}

// bool diagonal(std::array<float,6> v0,std::array<float,6> v,std::array<float,6> v1,std::array<float,6> b0,std::array<float,6> b, std::array<float,6> b1)
// {
//     return (inCone(v0, v, v1, b0, b, b1) && inCone(b0,b,b1,v0,v,v1) && diagonalie(v, b));
// }

// void findAreaAndOrientation()
// {
//     if(polygon1.size() < 3)
//         return;
//     static uint16_t time = 0;
//     float tot_area = 0.0f;
//     const std::array<float,3> anchor = {1.1f, 1.1f, 0.0f};
//     for(int i = 0; i < polygon1.size() - 1; i++)
//     {
//         float area = area2(anchor, polygon1[i], polygon1[i+1]);
//         orientation = (area > 0.0f)?1:-1;
//         //std::cout << "Area: " << area*orientation << " Orientation: " << std::to_string(orientation) << std::endl;
//         tot_area += area;
//     }
//     tot_area += area2(anchor, polygon1[polygon1.size() -1], polygon1[0]);
//     orientation = (tot_area > 0.0f)?1:-1;
//     std::cout << "Area: " << tot_area*orientation << " Orientation: " << ((orientation > 0.0f)?"clockwise":"counterclockwise") << std::endl;
// }

bool isConvex(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3)
{
    float tri_area = area2(v1, v2, v3);
    return tri_area*orientation > 0.0f;
}

// void verifyAngle()
// {
//     if(polygon1.size() < 3)
//         return;

//     bool is_convex = isConvex(polygon1[polygon1.size() - 1], polygon1[0], polygon1[1]);
//     std::cout << "Vertex " << polygon1.size() - 1 <<" is " << ((is_convex)?"convex":"reflex") << std::endl;

//     for(int i = 1; i < polygon1.size() - 1; i++)
//     {
//         is_convex =   isConvex(polygon1[i - 1], polygon1[i], polygon1[i+1]);
//         std::cout << "Vertex " << i <<" is " << ((is_convex)?"convex":"reflex") << std::endl;
//     }
    

//     is_convex = isConvex(polygon1[polygon1.size() - 2], polygon1[polygon1.size() - 1], polygon1[0]);
//     std::cout << "Vertex " << polygon1.size() - 1<<" is " << ((is_convex)?"convex":"reflex") << std::endl;
// }

// bool ear(std::array<float,3> v0,std::array<float,3> v,std::array<float,3> v1,std::array<float,3> v00,std::array<float,3> v11)
// {
//     if(!isConvex(v0, v, v1))
//     {
//         return false;
//     }
//     return Xor(diagonal(v00, v0, v, v, v1, v11), orientation < 0);
// }

// void earVerify()
// {
//     if(polygon1.size() < 4)
//         return;
//     std::cout << "Vertex " << 0 <<" is " << ((ear(polygon1[polygon1.size() - 1], polygon1[0], polygon1[1], polygon1[polygon1.size() - 2], polygon1[2]))?"ear":"not ear") << std::endl;
//     std::cout << "Vertex " << 1 <<" is " << ((ear(polygon1[0], polygon1[1], polygon1[2], polygon1[polygon1.size() - 1], polygon1[3]))?"ear":"not ear") << std::endl;
//     for(int i = 2; i < polygon1.size() - 2; i++)
//     {
//         std::cout << "Vertex " << i<<" is " << ((ear(polygon1[i-1], polygon1[i], polygon1[i+1], polygon1[i-2], polygon1[i+2]))?"ear":"not ear") << std::endl;
//     }
//     std::cout << "Vertex " << polygon1.size() - 2 <<" is " << ((ear(polygon1[polygon1.size() - 3], polygon1[polygon1.size() - 2], polygon1[polygon1.size() - 1], polygon1[polygon1.size() - 4], polygon1[0]))?"ear":"not ear") << std::endl;
//     std::cout << "Vertex " << polygon1.size() - 1 <<" is " << ((ear(polygon1[polygon1.size() - 2], polygon1[polygon1.size() - 1], polygon1[0], polygon1[polygon1.size() - 3], polygon1[1]))?"ear":"not ear") << std::endl;
// }


std::array<float,6> intersectionPoint(std::array<float,6> v1, std::array<float,6> v2, std::array<float,6> v3, std::array<float,6> v4)
{
    float px = (v1[0]*v2[1] - v1[1]*v2[0])*(v3[0] - v4[0]) - (v3[0]*v4[1] - v3[1]*v4[0])*(v1[0] - v2[0]);
    px /= ((v1[0] - v2[0])*(v3[1] - v4[1]) - (v3[0] - v4[0])*(v1[1] - v2[1]));
    float py = (v1[0]*v2[1] - v1[1]*v2[0])*(v3[1] - v4[1]) - (v3[0]*v4[1] - v3[1]*v4[0])*(v1[1] - v2[1]);
    py /= ((v1[0] - v2[0])*(v3[1] - v4[1]) - (v3[0] - v4[0])*(v1[1] - v2[1]));
    //TODO: wrong color
    return std::array<float,6>{px, py, 0, (v1[3] - v3[3])/2, (v1[4] - v3[4])/2, (v1[5] - v3[5])/2};
}

bool pointInside(std::array<float,6> p, const std::vector<std::array<float,6>>& p1)
{
    uint8_t hits = 0;
    for(int i = 0; i < p1.size() - 1; i++)
    {
        if(intersect(p, {p[0], 2, 0, 0, 0, 0}, p1[i], p1[i+1]))
            hits ++;
    }

    if(intersect(p, {p[0], 2, 0, 0, 0, 0}, p1[p1.size() - 1], p1[0]))
        hits ++;

    return hits%2 == 1;
}

std::pair<std::array<float,6>, PointInfo> classifyPoint(std::array<float,6>& intersection, bool* inside)
{
    PointInfo info;
    if(*inside)
    {
        std::cout << "Intersection exit" << std::endl;
        intersection[3] = 0.0f;
        intersection[4] = 0.0f;
        intersection[5] = 0.0f;
        info = INT_EXIT;
        *inside = false;
    }
    else
    {
        std::cout << "Intersection enter" << std::endl;
        intersection[3] = 1.0f;
        intersection[4] = 1.0f;
        intersection[5] = 1.0f;
        info = INT_ENTER;
        *inside = true;
    }
    std::pair<std::array<float,6>, PointInfo> point = {intersection, info};
    return point;
}

float pointDst(std::array<float,6> p1, std::array<float,6> p2)
{
    float out = sqrt(pow((p1[0] - p2[0]),2) + pow((p1[1] - p2[1]),2));
    std::cout << "dist " << out << std::endl;
    return out;
}

bool compareDists(std::pair<int, float> p1, std::pair<int, float> p2)
{
    return p1.second < p2.second;
}

std::array<std::vector<std::pair<int, PointInfo>>, 2> intersectionBetweenPolys(const std::vector<std::array<float,6>>& p1, const std::vector<std::array<float,6>>& p2)
{
    std::array<std::vector<std::pair<int, PointInfo>>, 2> intersections = {};
    if(p1.size() < 3 || p2.size() < 3)
        return intersections;
    
    std::map<int, std::vector<int>> inter_map1;
    std::map<int, std::vector<int>> inter_map2;

    for(int i = 0; i < p1.size(); i++)
    {
        std::pair<int, std::vector<int>> aux = {i, {}};
        inter_map1.insert(aux);
    }

    for(int i = 0; i < p2.size(); i++)
    {
        std::pair<int, std::vector<int>> aux = {i, {}};
        inter_map2.insert(aux);
    }

    std::cout << p1.size() << " " << inter_map1.size() << std::endl;
    std::cout << p2.size() << " " << inter_map2.size() << std::endl;
    
    int pos_intersection = -1;

    for(int i = 0; i < p1.size() - 1; i++)
    {
        for(int j = 0; j < p2.size() - 1; j++)
        {
            if(intersect(p1[i], p1[i+1], p2[j], p2[j+1]))
            {
                std::cout << "Intersection case 1 between " << i << " " << i + 1 << " " << j << " " << j+1 << " " << std::endl;
                std::array<float,6> intersection = intersectionPoint(p1[i], p1[i+1], p2[j], p2[j+1]);
                std::cout << "Intersection case 1 in x=" << intersection[0] << " y= " << intersection[1] << std::endl;

                intersection_points.push_back(intersection);
                pos_intersection++;
                inter_map1[i].push_back(pos_intersection);
                inter_map2[j].push_back(pos_intersection);
            }
        }

        if(intersect(p1[i], p1[i+1], p2[p2.size() - 1], p2[0]))
        {
            std::cout << "Intersection case 2 between " << i << " " << i + 1 << " " << p2.size() - 1 << " " << 0 << " " << std::endl;
            std::array<float,6> intersection = intersectionPoint(p1[i], p1[i+1], p2[p2.size() - 1], p2[0]);
            std::cout << "Intersection case 2 in x=" << intersection[0] << " y= " << intersection[1] << std::endl;
            
            intersection_points.push_back(intersection);
            pos_intersection++;
            inter_map1[i].push_back(pos_intersection);
            inter_map2[p2.size() - 1].push_back(pos_intersection);
        }
    }

    //intersections[0].push_back(p1[p1.size() - 1]);
    for(int j = 0; j < p2.size() - 1; j++)
    {
        if(intersect(p1[p1.size() - 1], p1[0], p2[j], p2[j+1]))
        {
            std::cout << "Intersection case 3 between " << p1.size() - 1 << " " << 0 << " " << j << " " << j+1 << " " << std::endl;
            std::array<float,6> intersection = intersectionPoint(p1[p1.size() - 1], p1[0], p2[j], p2[j+1]);
            std::cout << "Intersection case 3 in x=" << intersection[0] << " y= " << intersection[1] << std::endl;
            
            intersection_points.push_back(intersection);
            pos_intersection++;
            inter_map1[p1.size() - 1].push_back(pos_intersection);
            inter_map2[j].push_back(pos_intersection);
        }
    }

    if(intersect(p1[p1.size() - 1], p1[0], p2[p2.size() - 1], p2[0]))
    {
        std::cout << "Intersection case 4 between " << p1.size() - 1 << " " << 0 << " " << p2.size() - 1 << " " << 0 << " " << std::endl;
        std::array<float,6> intersection = intersectionPoint(p1[p1.size() - 1], p1[0], p2[p2.size() - 1], p2[0]);
        std::cout << "Intersection case 4 in x=" << intersection[0] << " y= " << intersection[1] << std::endl;
        
        intersection_points.push_back(intersection);
        pos_intersection++;
        inter_map1[p1.size() - 1].push_back(pos_intersection);
        inter_map2[p2.size() - 1].push_back(pos_intersection);
    }

    std::cout << "listing"  << std::endl;

    for(int i = 0; i < inter_map1.size(); i++)
    {
        std::pair<int, PointInfo> point = {i, VERTEX};
        intersections[0].push_back(point);
        std::vector<std::pair<int, float>> dists = {};
        for(int j = 0; j < inter_map1[i].size(); j++)
        {
            std::pair<int, float> dist = {j, pointDst(p1[i], intersection_points[inter_map1[i][j]])};
            dists.push_back(dist);
        }
        std::sort(dists.begin(), dists.end(), compareDists);
        for(int j = 0; j < dists.size(); j++)
        {
            point = {inter_map1[i][dists[j].first], INT};
            std::cout << " " << inter_map1[i][dists[j].first] << " " << dists[j].second<< std::endl;
            intersections[0].push_back(point);
        }
    }

    for(int i = 0; i < inter_map2.size(); i++)
    {
        std::pair<int, PointInfo> point = {i, VERTEX};
        intersections[1].push_back(point);
        std::vector<std::pair<int, float>> dists = {};
        for(int j = 0; j < inter_map2[i].size(); j++)
        {
            std::pair<int, float> dist = {j, pointDst(p2[i], intersection_points[inter_map2[i][j]])};
            dists.push_back(dist);
        }
        std::sort(dists.begin(), dists.end(), compareDists);
        for(int j = 0; j < dists.size(); j++)
        {
            point = {inter_map2[i][dists[j].first], INT};
            std::cout << " " << inter_map2[i][dists[j].first] << " " << dists[j].second<< std::endl;
            intersections[1].push_back(point);
        }
    }

    std::cout << "Intersections found"  << std::endl;
    return intersections;
}

void negate(std::vector<std::array<float,6>>& poly)
{
    std::reverse(poly.begin(),poly.end());
    for(std::array<float,6>& v : poly)
    {
        v[3] = 1 - v[3];
        v[4] = 1 - v[4];
        v[5] = 1 - v[5];
    }
}

void waClipping(std::array<std::vector<std::pair<int, PointInfo>>, 2> ints, bool inside, const std::vector<std::array<float,6>>& p1, const std::vector<std::array<float,6>>& p2)
{
    //int_polygons.clear();
    int i = 0, j = 0;
    int ints_visited = 0;
    std::vector<std::array<float,6>> int_polygon = {};
    int enter_point = -1;
    std::set<int> visited_ints = {};
    if(intersection_points.size() == 0 && inside)
    {
        for(std::array<float,6> v : p2)
        {
           int_polygon.push_back(v);
           int_polygon[int_polygon.size() - 1][3] = 0.0f; 
           int_polygon[int_polygon.size() - 1][4] = 0.5f; 
           int_polygon[int_polygon.size() - 1][5] = 0.5f; 
        }
        int_polygons.push_back(int_polygon);
        return;
    }

    while(visited_ints.size() < intersection_points.size())
    {
        if(ints[1][j].second == INT && inside)
        {
            inside = false;
            std::cout << "EXIT"  << std::endl;
        }
        else if(ints[1][j].second == INT && !inside)
        {
            std::cout << "ENTER"  << std::endl;
            inside = true;
            std::cout << "Starting intersection polygon"  << std::endl;

            if(enter_point == -1)
                enter_point = ints[1][j].first;
            visited_ints.insert(ints[1][j].first);
            while((ints[0][i].first != enter_point && ints[0][i].second != INT) || !(ints[0][i].first == enter_point && ints[0][i].second == INT))
            {
                intersection_points[ints[1][j].first][3] = 1;
                intersection_points[ints[1][j].first][4] = 1;
                intersection_points[ints[1][j].first][5] = 1;
                int_polygon.push_back(intersection_points[ints[1][j].first]);
                j = (j + 1)%ints[1].size();
                while(ints[1][j].second != INT)
                {
                    int_polygon.push_back(p2[ints[1][j].first]);
                    j = (j + 1)%ints[1].size();
                }

                while((ints[1][j].first != ints[0][i].first && ints[0][i].second != INT) || !(ints[1][j].first == ints[0][i].first && ints[0][i].second == INT)) i = (i + 1)%ints[0].size();
                intersection_points[ints[1][j].first][3] = 0;
                intersection_points[ints[1][j].first][4] = 0;
                intersection_points[ints[1][j].first][5] = 0;
                int_polygon.push_back(intersection_points[ints[0][i].first]);
                visited_ints.insert(ints[0][i].first);
                i = (i + 1)%ints[0].size();
                
                while(ints[0][i].second != INT)
                {
                    int_polygon.push_back(p1[ints[0][i].first]);
                    i = (i + 1)%ints[0].size();
                }
                intersection_points[ints[0][i].first][3] = 0.5;
                intersection_points[ints[0][i].first][4] = 0.5;
                intersection_points[ints[0][i].first][5] = 0.5;
                int_polygon.push_back(intersection_points[ints[0][i].first]);
                while((ints[1][j].first != ints[0][i].first && ints[1][j].second != INT) || !(ints[1][j].first == ints[0][i].first && ints[1][j].second == INT)) j = (j + 1)%ints[1].size();
                visited_ints.insert(ints[0][i].first);
            }
            int_polygons.push_back(int_polygon);
            int_polygon.clear();
            enter_point = -1;
        }
        i=0;
        j = (j + 1)%ints[1].size();
    }
    std::set<int>::iterator it;
    std::cout << "polygons created"  << std::endl;
    for (it=visited_ints.begin(); it!=visited_ints.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << '\n';
}

void waUnion(std::array<std::vector<std::pair<int, PointInfo>>, 2> ints, bool inside, const std::vector<std::array<float,6>>& p1, const std::vector<std::array<float,6>>& p2)
{
    //int_polygons.clear();
    int i = 0, j = 0;
    int ints_visited = 0;
    std::vector<std::array<float,6>> int_polygon = {};
    int enter_point = -1;
    std::set<int> visited_ints = {};
    if(intersection_points.size() == 0 && !inside)
    {
        for(std::array<float,6> v : p2)
        {
           int_polygon.push_back(v);
           int_polygon[int_polygon.size() - 1][3] = 0.0f; 
           int_polygon[int_polygon.size() - 1][4] = 0.5f; 
           int_polygon[int_polygon.size() - 1][5] = 0.5f; 
        }
        int_polygons.push_back(int_polygon);
        return;
    }

    while(visited_ints.size() < intersection_points.size())
    {
        if(ints[1][j].second == INT && !inside)
        {
            inside = true;
            std::cout << "EXIT"  << std::endl;
        }
        else if(ints[1][j].second == INT && inside)
        {
            std::cout << "ENTER"  << std::endl;
            inside = false;
            std::cout << "Starting intersection polygon"  << std::endl;

            if(enter_point == -1)
                enter_point = ints[1][j].first;
            visited_ints.insert(ints[1][j].first);
            while((ints[0][i].first != enter_point && ints[0][i].second != INT) || !(ints[0][i].first == enter_point && ints[0][i].second == INT))
            {
                intersection_points[ints[1][j].first][3] = 1;
                intersection_points[ints[1][j].first][4] = 1;
                intersection_points[ints[1][j].first][5] = 1;
                int_polygon.push_back(intersection_points[ints[1][j].first]);
                j = (j + 1)%ints[1].size();
                while(ints[1][j].second != INT)
                {
                    int_polygon.push_back(p2[ints[1][j].first]);
                    j = (j + 1)%ints[1].size();
                }

                while((ints[1][j].first != ints[0][i].first && ints[0][i].second != INT) || !(ints[1][j].first == ints[0][i].first && ints[0][i].second == INT)) i = (i + 1)%ints[0].size();
                intersection_points[ints[1][j].first][3] = 0;
                intersection_points[ints[1][j].first][4] = 0;
                intersection_points[ints[1][j].first][5] = 0;
                int_polygon.push_back(intersection_points[ints[0][i].first]);
                visited_ints.insert(ints[0][i].first);
                i = (i + 1)%ints[0].size();
                
                while(ints[0][i].second != INT)
                {
                    int_polygon.push_back(p1[ints[0][i].first]);
                    i = (i + 1)%ints[0].size();
                }
                intersection_points[ints[0][i].first][3] = 0.5;
                intersection_points[ints[0][i].first][4] = 0.5;
                intersection_points[ints[0][i].first][5] = 0.5;
                int_polygon.push_back(intersection_points[ints[0][i].first]);
                while((ints[1][j].first != ints[0][i].first && ints[1][j].second != INT) || !(ints[1][j].first == ints[0][i].first && ints[1][j].second == INT)) j = (j + 1)%ints[1].size();
                visited_ints.insert(ints[0][i].first);
            }
            int_polygons.push_back(int_polygon);
            int_polygon.clear();
            enter_point = -1;
        }
        i=0;
        j = (j + 1)%ints[1].size();
    }
    std::set<int>::iterator it;
    std::cout << "polygons created"  << std::endl;
    for (it=visited_ints.begin(); it!=visited_ints.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << '\n';
}

void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        createPoint(x,y, polygons[actual_poly], poly_color[actual_poly]);
        //findAreaAndOrientation();
        //verifyAngle();
        //earVerify();
    }
    else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        createPoint(x,y, polygon2, {0, 0, 1.0});
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
                    break;
                case 'd':
                    for(int i = 0; i < polygons.size(); i++)
                        polygons[i].clear();
                    polygons.clear();
                    polygons.push_back({});
                    actual_poly = 0;
                    break;
                case 'D':
                    polygons[actual_poly].clear();
                    break;
                case 'i':
                {
                    if(polygons.size() < 2)
                        break;
                    std::array<std::vector<std::pair<int, PointInfo>>, 2> ints = intersectionBetweenPolys(polygons[0], polygons[1]);
                    bool start_inside = pointInside(polygons[1][0], polygons[0]);
                    waClipping(ints, start_inside, polygons[0], polygons[1]);
                    intersection_points.clear();
                    if(polygons.size() < 3)
                        break;
                    

                    for(int i = 2; i < polygons.size(); i++)
                    {
                        int num_ints = int_polygons.size();
                        for(int j = 0; j < num_ints; j++)
                        {
                            if(polygons[i].size() == 0)
                                continue;
                            std::cout << "Entering polygon " << i << " with intersection " << j << std::endl;
                            ints = intersectionBetweenPolys(polygons[i], int_polygons[j]);
                            start_inside = pointInside(int_polygons[j][0], polygons[i]);
                            waClipping(ints, start_inside, polygons[i], int_polygons[j]);
                            intersection_points.clear();
                        }
                        int_polygons.erase(int_polygons.begin(), int_polygons.begin() + num_ints);
                    }
                    
                    break;
                }
                case 'n':
                    negate(polygons[actual_poly]);
                    break;
                case 'c':
                    int_polygons.clear();
                    intersection_points.clear();
                    break;
                case 'e':
                {
                    if(polygons.size() < 2 || polygons.size() >= 3)
                        break;
                    negate(polygons[0]);
                    std::array<std::vector<std::pair<int, PointInfo>>, 2> ints = intersectionBetweenPolys(polygons[0], polygons[1]);
                    bool start_inside = pointInside(polygons[1][0], polygons[0]);
                    waClipping(ints, start_inside, polygons[0], polygons[1]);
                    break;
                }
                case 'E':
                {
                    if(polygons.size() < 2 || polygons.size() >= 3)
                        break;
                    negate(polygons[1]);
                    std::array<std::vector<std::pair<int, PointInfo>>, 2> ints = intersectionBetweenPolys(polygons[1], polygons[0]);
                    bool start_inside = pointInside(polygons[0][0], polygons[1]);
                    waClipping(ints, start_inside, polygons[1], polygons[0]);
                    break;
                }
                case 'u':
                {
                    if(polygons.size() < 2)
                        break;
                    std::array<std::vector<std::pair<int, PointInfo>>, 2> ints = intersectionBetweenPolys(polygons[1], polygons[0]);
                    bool start_inside = pointInside(polygons[0][0], polygons[1]);
                    waUnion(ints, start_inside, polygons[1], polygons[0]);
                    intersection_points.clear();
                    if(polygons.size() < 3)
                        break;
                    int num_ints = int_polygons.size();

                    for(int i = 2; i < polygons.size(); i++)
                    {
                        if(polygons[i].size() == 0)
                            continue;
                        std::cout << "Entering polygon for union" << i << std::endl;
                        ints = intersectionBetweenPolys(polygons[i], int_polygons[0]);
                        start_inside = pointInside(int_polygons[0][0], polygons[i]);
                        waUnion(ints, start_inside, polygons[i], int_polygons[0]);
                        intersection_points.clear();
                        int_polygons.erase(int_polygons.begin(), int_polygons.begin() + 1);
                    }
                    break;
                }
                case 'a':
                    if(actual_poly == 0)
                        actual_poly = 0;
                    else
                        actual_poly--;
                    break;
                case 'b':
                    actual_poly ++;
                    if(actual_poly == polygons.size()){
                        polygons.push_back({});
                        poly_color.push_back({(float)std::rand()/RAND_MAX,(float)std::rand()/RAND_MAX,(float)std::rand()/RAND_MAX});
                    }
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

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
