#include "../include/dcel.hpp"
#include <math.h>

void createEdge(Vertex_t& v1, Vertex_t& v2, HalfEdge_t& he1, HalfEdge_t& he2)
{
	he1.orig = &v1;
	he1.end = &v2;
	he2.orig = &v2;
	he2.end = &v1;
	he1.twin = &he2;
	he2.twin = &he1;
}

float area2(Vertex_t v1, Vertex_t v2, Vertex_t v3)
{
	float area =    v1.x*v2.y                     //x1y2
					- v2.x*v1.y                   //x2y1
					+ v2.x*v3.y                   //x2y3
					- v3.x*v2.y                   //x3y2
					+ v3.x*v1.y                   //x3y1
					- v1.x*v3.y;                  //x1y3
	return area;
}

int searchHalfEdge(int s, int e, std::vector<HalfEdge_t> &h) {
  int l = h.size();
  for (int i = 0; i < l; i++) {
	if (h[i].orig->key == s && h[i].end->key == e) {
	  return i;
	}
  }
  std::cout << "Error in serching half edge"
	   << "\n";
  return -1;
}


void fillVertexTable(std::vector<VertexTable_t> &ver_tab, std::vector<HalfEdge_t> &h, std::vector<Vertex_t> &v, std::map<int,std::vector<int>>& adj) {
  for (int i = 0; i < v.size(); i++) 
  {
	std::cout << "Pol created" << std::endl;
	int s = v[i].key;
	int e = adj[s][0]; // Assuming no stand alone vertex
	VertexTable_t vt_aux = {.v = &v[i], .he = &h[searchHalfEdge(s, e, h)]};
	ver_tab.push_back(vt_aux);
  }
}

float angle(Vertex_t v1, Vertex_t v2, Vertex_t v3) {
  float x1 = v1.x - v2.x;
  float y1 = v1.y - v2.y;
  float x3 = v3.x - v2.x;
  float y3 = v3.y - v2.y;
  float dot = x1 * x3 + y1 * y3;
  float det = x1 * y3 - y1 * x3;
  float result = atan2(det, dot);
  return ((result < 0) ? (result * 180 / 3.141592) + 360
					   : (result * 180 / 3.141592));
}

int nextHalfEdge(int current, std::map<int, std::vector<int>>& adj,
				   std::vector<Vertex_t>& vertex, std::vector<HalfEdge_t> &half_edge) {
	int s, e;
	float max_angle = 0, temp_angle;
	int next_vertex;
	s = half_edge[current].orig->key;
	e = half_edge[current].end->key;
	next_vertex = s;
	for (auto k : adj[e]) {
		if (k == e)
		continue;
		else {
		temp_angle = angle(vertex[s], vertex[e], vertex[k]);
		if (max_angle < temp_angle) {
			max_angle = temp_angle;
			next_vertex = k;
		}
		}
	}
	return searchHalfEdge(e, next_vertex, half_edge);
}

void fillHalfEdgeTable(std::vector<HalfEdgeTable_t> &half_edge_table,
							std::vector<HalfEdge_t> &half_edge,
							std::vector<bool> &unvisited_half_edge,
							std::vector<Vertex_t>& vertex, std::map<int, std::vector<int>>& adj,
							std::vector<Face_t> &face, std::vector<FaceTable_t> &face_table) {
	int l = unvisited_half_edge.size();
	int current, next, previous;
	int head, j = 0, head_index;
	for (int i = 0; i < l; i++) {
		if (unvisited_half_edge[i] == false) {
			unvisited_half_edge[i] = true;
			head = i;
			head_index = j;
			HalfEdgeTable_t het_aux;
			het_aux.he = &half_edge[i];
			next = nextHalfEdge(i, adj, vertex, half_edge);
			het_aux.next = &half_edge[next];
			half_edge_table.push_back(het_aux);
			j++;
			previous = i;
			current = next;
			while (1) {
				unvisited_half_edge[current] = true;
				het_aux.he = &half_edge[current];
				next = nextHalfEdge(current, adj, vertex, half_edge);
				het_aux.next = &half_edge[next];
				het_aux.prev = &half_edge[previous];
				half_edge_table.push_back(het_aux);
				j++;
				previous = current;
				current = next;
				if (next == head) {
					break;
				}
			}
			
			half_edge_table[head_index].prev = &half_edge[previous];
		}
	}
	fillHalfEdgeFace(half_edge_table, face, vertex, face_table);
}


void fillHalfEdgeFace(std::vector<HalfEdgeTable_t> &half_edge_table,
						 std::vector<Face_t> &face, std::vector<Vertex_t>& vertex,
						 std::vector<FaceTable_t> &face_table) {
	
	int l = half_edge_table.size();
	int face_key = 0;
	int head = 0;
	float x2, y2;
	float signedArea = 0;
	for (int i = 0; i < l; i++) {
		float x1 = vertex[half_edge_table[i].he->orig->key].x;
		float y1 = vertex[half_edge_table[i].he->orig->key].y;
		if (half_edge_table[i].next == half_edge_table[head].he) {
			x2 = vertex[half_edge_table[head].he->orig->key].x;
			y2 = vertex[half_edge_table[head].he->orig->key].y;
			signedArea += (x1 * y2 - x2 * y1);
			if (signedArea > 0) // Assuming no standalone edge
			{
				Face_t* f_aux = new Face_t;
				FaceTable_t ft_aux;
				ft_aux.area = abs(signedArea / 2);
				f_aux->key = face_key;
				ft_aux.face = f_aux;
				ft_aux.outer_component = half_edge_table[head].he;
				for (int j = head; j <= i; j++)
					half_edge_table[j].incident_face = f_aux;
				face_key++;
				face.push_back(*f_aux);
				face_table.push_back(ft_aux);
			}
			signedArea = 0;
			head = i + 1;
		} else {
			x2 = vertex[half_edge_table[i].he->end->key].x;
			y2 = vertex[half_edge_table[i].he->end->key].y;
			signedArea += (x1 * y2 - x2 * y1);
		}
	}
	Face_t* f_aux = new Face_t;
	FaceTable_t ft_aux;
	f_aux->key = -1;
	ft_aux.face = f_aux;
	face.push_back(*f_aux);
	face_table.push_back(ft_aux);
}

void fillFaceTableInnerComponents(std::vector<FaceTable_t> &face_table,
                                      std::vector<HalfEdge_t> &half_edge,
                                      std::vector<HalfEdgeTable_t> &half_edge_table,
                                      std::vector<Face_t> &face,
                                      std::vector<Vertex_t>& vertex)
{
	int face_index;
	int l = half_edge_table.size();
	HalfEdge_t *temp, *head;
	for (int i = 0; i < l; i++) {
		if (half_edge_table[i].incident_face != NULL)
			continue;
		temp = half_edge_table[i].he;
		head = temp;
		std::vector<int> key;
		while (1) {
			key.push_back(temp->orig->key);
			temp = half_edge_table[i].next;
			if (head == temp) {
				face_index = checkIfInside(face_table, vertex, key, face, half_edge_table);
				if (face_index != -1) {
					int index = searchHalfEdgeTable(temp, half_edge_table);
					face_table[face_index].inner_components.push_back(half_edge_table[index].he);
					while (1) {
						half_edge_table[index].incident_face = &face[face_index];
						temp = half_edge_table[index].next;
						if (temp == head)
						break;
						index++;
					}
				} else {
					face_index = searchOuterFace(face);
					int index = searchHalfEdgeTable(temp, half_edge_table);
					face_table[face_index].inner_components.push_back(half_edge_table[index].he);
					while (1) {
						half_edge_table[index].incident_face = &face[face_index];
						temp = half_edge_table[index].next;
						if (temp == head)
							break;
						index++;
					}
				}
				break;
			}
			i++;
		}
		key.clear();
  	}
}

int searchOuterFace(std::vector<Face_t>& face) {
  for (int i = 0; i < face.size(); i++) {
    if (face[i].key == -1)
      return i;
  }
  return 0;
}

int checkIfInside(std::vector<FaceTable_t> &face_table, std::vector<Vertex_t> &vertex,
                    std::vector<int> key, std::vector<Face_t>& face,
                    std::vector<HalfEdgeTable_t>& half_edge_table) {
  int face_index = -1;
  float self_area;
  int l = face_table.size();
  FaceTable_t temp;
  HalfEdge_t *half_edge, *temp2, *head;
  float min_area = 100021.1;
  self_area = areaPoly(key, vertex);
  for (int i = 0; i < l; i++) {
    if (face_table[i].face == NULL)
      break;
    temp = face_table[i];
    half_edge = temp.outer_component;
    int index = searchHalfEdgeTable(half_edge, half_edge_table);
    temp2 = half_edge_table[index].he;
    head = temp2;
    std::vector<int> key2;
    while (1) {
      key2.push_back(temp2->orig->key);
      temp2 = half_edge_table[index].next;
      if (head == temp2)
        break;
      index++;
    }
    bool flag = 1;
    for (int k = 0; k < key.size(); k++) {
      flag = flag & checkIfPointIsInside(key[k], key2, vertex);
      if (flag == 0)
        break;
    }
    if (flag) {
      float a = areaPoly(key2, vertex);
      if (min_area > a && self_area != a && self_area < a) {
        min_area = a;
        face_index = i;
      }
    }
  }
  return face_index;
}

bool checkIfPointIsInside(int ver, std::vector<int> key,
                              std::vector<Vertex_t> &vertex) {
  float x, y;
  int n = key.size();
  Point_t polygon1[n];
  for (int i = 0; i < n; i++) {
    polygon1[i] = {vertex[key[i]].x, vertex[key[i]].y};
  }
  Point_t p = {vertex[ver].x, vertex[ver].y};
  return isInside(polygon1, n, p);
}

int searchHalfEdgeTable(HalfEdge_t *half_edge,
                           std::vector<HalfEdgeTable_t>& half_edge_table) {
  int l = half_edge_table.size();
  for (int i = 0; i < l; i++) {
    if (half_edge_table[i].he == half_edge)
      return i;
  }
  return 0;
}

bool isInside(Point_t polygon[], int n, Point_t p) {
  // There must be at least 3 vertices in polygon[]
  if (n < 3)
    return false;

  // Create a point for line segment from p to infinite
  Point_t extreme = {INF, p.y};

  // Count intersections of the above line with sides of polygon
  int count = 0, i = 0;
  do {
    int next = (i + 1) % n;

    // Check if the line segment from 'p' to 'extreme' intersects
    // with the line segment from 'polygon[i]' to 'polygon[next]'
    if (doIntersect(polygon[i], polygon[next], p, extreme)) {
      // If the point 'p' is colinear with line segment 'i-next',
      // then check if it lies on segment. If it lies, return true,
      // otherwise false
      if (orientation(polygon[i], p, polygon[next]) == 0)
        return onSegment(polygon[i], p, polygon[next]);

      count++;
    }
    i = next;
  } while (i != 0);

  // Return true if count is odd, false otherwise
  return count & 1; // Same as (count%2 == 1)
}

bool doIntersect(Point_t p1, Point_t q1, Point_t p2, Point_t q2) {
  // Find the four orientations needed for general and
  // special cases
  float o1 = orientation(p1, q1, p2);
  float o2 = orientation(p1, q1, q2);
  float o3 = orientation(p2, q2, p1);
  float o4 = orientation(p2, q2, q1);

  // General case
  if (o1 != o2 && o3 != o4)
    return true;

  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && onSegment(p1, p2, q1))
    return true;

  // p1, q1 and p2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && onSegment(p1, q2, q1))
    return true;

  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && onSegment(p2, p1, q2))
    return true;

  // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && onSegment(p2, q1, q2))
    return true;

  return false; // Doesn't fall in any of the above cases
}

int orientation(Point_t p, Point_t q, Point_t r) {
  float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (val == 0)
    return 0;               // colinear
  return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool onSegment(Point_t p, Point_t q, Point_t r) {
  if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) &&
      q.y >= std::min(p.y, r.y))
    return true;
  return false;
}

float areaPoly(std::vector<int> key, std::vector<Vertex_t>& vertex) {
  float x2, y2;
  float signedArea = 0;
  int l = key.size();
  for (int i = 0; i < l; i++) {
    float x1 = vertex[key[i]].x;
    float y1 = vertex[key[i]].y;
    if (i == l - 1) {
      x2 = vertex[key[0]].x;
      y2 = vertex[key[0]].y;
      signedArea += (x1 * y2 - x2 * y1);
    } else {
      x2 = vertex[key[i + 1]].x;
      y2 = vertex[key[i + 1]].y;
      signedArea += (x1 * y2 - x2 * y1);
    }
  }
  return abs(signedArea / 2);
}