#pragma once
#include <vector>
#include <map>
#include <iostream>

#define INF 10000

typedef struct
{
    float x;
    float y;
} Point_t;

typedef struct 
{
    float x;
    float y;
    float r;
    float g;
    float b;
    int key;
} Vertex_t;

typedef struct HalfEdge
{
    Vertex_t *orig, *end;
    HalfEdge* twin;
} HalfEdge_t;

typedef struct 
{
    int key = -1;
} Face_t;

typedef struct
{
    int origin_v, end_v;
    Vertex_t* v;
    HalfEdge_t* he;
} VertexTable_t;

typedef struct
{
    Face_t* face;
    std::vector<HalfEdge_t*> inner_components;
    HalfEdge_t* outer_component = NULL;
    float area = 0;
} FaceTable_t;

typedef struct
{
    HalfEdge_t* he, *next, *prev;
    Face_t* incident_face = NULL;
} HalfEdgeTable_t;

HalfEdge_t* next(HalfEdge_t& he);
HalfEdge_t* prev(HalfEdge_t& he);
HalfEdge_t* twin(HalfEdge_t& he);
bool isPointInsideFace(Face_t& face, Point_t& point);
float area2(Vertex_t v1, Vertex_t v2, Vertex_t v3);
int searchHalfEdge(int s, int e, std::vector<HalfEdge_t> &h);
void createEdge(Vertex_t& v1, Vertex_t& v2, HalfEdge_t& he1, HalfEdge_t& he2);
void fillVertexTable(std::vector<VertexTable_t> &ver_tab, std::vector<HalfEdge_t> &h, std::vector<Vertex_t> &v, std::map<int,std::vector<int>>& adj);


float angle(Vertex_t v1, Vertex_t v2, Vertex_t v3);
int nextHalfEdge(int current, std::map<int, std::vector<int>>& adj,
                   std::vector<Vertex_t>& vertex, std::vector<HalfEdge_t> &half_edge);

void fillHalfEdgeTable(std::vector<HalfEdgeTable_t> &half_edge_table,
                          std::vector<HalfEdge_t> &half_edge,
                          std::vector<bool> &unvisited_half_edge,
                          std::vector<Vertex_t>& vertex, std::map<int, std::vector<int>>& adj,
                          std::vector<Face_t> &face, std::vector<FaceTable_t> &face_table);
                          
void fillHalfEdgeFace(std::vector<HalfEdgeTable_t> &half_edge_table,
                         std::vector<Face_t> &face, std::vector<Vertex_t>& vertex,
                         std::vector<FaceTable_t> &face_table);

void fillFaceTableInnerComponents(std::vector<FaceTable_t> &face_table,
                                      std::vector<HalfEdge_t> &half_edge,
                                      std::vector<HalfEdgeTable_t> &half_edge_table,
                                      std::vector<Face_t> &face,
                                      std::vector<Vertex_t>& vertex);

bool checkIfPointIsInside(int ver, std::vector<int> key,
                              std::vector<Vertex_t> &vertex);

int searchHalfEdgeTable(HalfEdge_t *half_edge,
                           std::vector<HalfEdgeTable_t>& half_edge_table);

bool isInside(Point_t polygon[], int n, Point_t p);

bool doIntersect(Point_t p1, Point_t q1, Point_t p2, Point_t q2);

int orientation(Point_t p, Point_t q, Point_t r);

bool onSegment(Point_t p, Point_t q, Point_t r);

int checkIfInside(std::vector<FaceTable_t> &face_table, std::vector<Vertex_t> &vertex,
                    std::vector<int> key, std::vector<Face_t>& face,
                    std::vector<HalfEdgeTable_t>& half_edge_table);

int searchOuterFace(std::vector<Face_t>& face);

float areaPoly(std::vector<int> key, std::vector<Vertex_t>& vertex);