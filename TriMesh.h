//
// Created by leoyolo on 9/9/16.
// Basic data structure that holds a triangular mesh using half-edge data structure.
//

#ifndef OPENGLPLAYGROUND_TRIMESH_H
#define OPENGLPLAYGROUND_TRIMESH_H

#include <vector>
#include <stdio.h>
#include <map>
#include <atomic>
#include <memory>
#include <forward_list>
#include <assert.h>
#include <array>
#include <set>
#include <algorithm>
#include <queue>
#include <stdexcept>
#include <math.h>

// forward declaration
struct HE_edge;
struct HE_vert;
struct HE_face;

struct HE_edge {
	HE_vert *vert;  // vertex at the end of the half-edge
	HE_edge *pair;  // oppositely oriented half-edge
	HE_face *face;  // the incident face
	HE_edge *prev;  // previous half-edge around the face
	HE_edge *next;  // next half-edge around the face
	int id;         // id of an edge.  Opposite edges have different ids.

	HE_edge() : vert(nullptr), pair(nullptr), face(nullptr), prev(nullptr), next(nullptr), id(-1) {}
};

struct HE_vert {
	float x, y, z;  // vertex coordinates
    float nx, ny, nz; // vertex normals
	HE_edge *edge; // one of the half-edges emanating from the vertex
	std::forward_list<HE_edge*> out_edge;	// for the ease of edge look-up
	int id;         // id of an vertex

    HE_vert() : x(0.f), y(0.f), z(0.f), nx(0.f), ny(0.f), nz(0.f), edge(nullptr), out_edge(), id(-1) {}
    void ComputeNormal();       // forward declaration
};

struct HE_face {
    HE_edge *edge;  // one of the half-edges bordering the face
    int id;         // id of a face
    int vertid[3];     // id of the vertices around the face.  This field is useful for m and obj files.
    float nx, ny, nz;   // normal vector which has the same orientation as the mesh.

    HE_face(): edge(nullptr), id(-1), vertid{-1, -1, -1} {}
    void ComputeNormal();
};

void HE_vert::ComputeNormal() {
    assert(edge && !out_edge.empty() && "HE_vert.ComputeNormal: Edges are not initialized.");
    for (auto it = out_edge.begin(); it != out_edge.end(); ++it) {
        if (!(*it)->face) continue; // edge is on boundary
        nx += (*it)->face->nx;
        ny += (*it)->face->ny;
        nz += (*it)->face->nz;
    }
    float norm = sqrt(nx*nx + ny*ny + nz*nz);
    if (norm < 1e-5) {
        return; // TODO: handle degraded triangle
    }
    nx /= norm;
    ny /= norm;
    nz /= norm;
}

void HE_face::ComputeNormal() {
    assert(edge && "HE_face.ComputeNormal: Edges around the face are not initialized.");
    assert(edge->vert == edge->next->pair->vert &&
           "HE_face.ComputeNormal: Incorrect face orientation.");
    float vec1[3];
    float vec2[3];
    vec1[0] = edge->pair->vert->x - edge->vert->x;
    vec1[1] = edge->pair->vert->y - edge->vert->y;
    vec1[2] = edge->pair->vert->z - edge->vert->z;
    vec2[0] = edge->next->vert->x - edge->vert->x;
    vec2[1] = edge->next->vert->y - edge->vert->y;
    vec2[2] = edge->next->vert->z - edge->vert->z;
    // should be vec2->vec1 to maintain orientation
    nx = vec2[1]*vec1[2] - vec2[2]*vec1[1];
    ny = vec1[0]*vec2[2] - vec1[2]*vec2[0];
    nz = vec2[0]*vec1[1] - vec2[1]*vec1[0];
    float norm = sqrt(nx*nx + ny*ny + nz*nz);
    if (norm < 1e-5) {
        return; // TODO: handle degraded triangle
    }
    nx /= norm;
    ny /= norm;
    nz /= norm;
}

class TriMesh {

protected:

// AdjacencyInfo contains necessary adjacency information for constructing TriMesh class.
// Upon construction of TriMesh the AdjacencyInfo is no longer needed.
struct AdjacencyInfo {
	std::map<HE_face*, std::array<HE_vert*, 3>> fvert;
	std::map<HE_vert*, std::forward_list<HE_face*>> vface;
	std::map<HE_face*, std::set<HE_face*>> fface;	// TODO: Try to replace std::set to other data structure to reduce complexity.
	std::map<int, HE_edge*> edgemap;
	std::map<int, HE_face*> facemap;
	std::map<int, HE_vert*> vertmap;
	TriMesh *mesh;	// Since AdjacencyInfo is owned by TriMesh, no need to free.
	bool isUpdated;

	// constructor
	AdjacencyInfo(TriMesh *m)
		: fvert{}, vface{}, edgemap{}, facemap{}, vertmap{}, mesh{m}, isUpdated{false}
	{}

	// Do all the work together.
	void UpdateAdjacencyInfo() {
		CreateEdgeMap();
		CreateFaceMap();
		CreateVertexMap();
		ConstructFaceVertices();
		ConstructVertexFaces();
		ConstructFaceFaces();
		isUpdated = true;
	}

	// return a `reversed` map that maps edges/vertices/faces/ id into their corresponding pointer
	const std::map<int, HE_edge*> &CreateEdgeMap() {
		assert(mesh != nullptr && "AdjacencyInfo.CreateEdgeMap: mesh not initialized.");
		edgemap.clear();
		for (const auto edge_ptr : mesh->m_edges) {
			edgemap[edge_ptr->id] = edge_ptr;
		}
		return edgemap;
	}

	const std::map<int, HE_face*> &CreateFaceMap() {
		assert(mesh != nullptr && "AdjacencyInfo.CreateFaceMap: mesh not initialized.");
		facemap.clear();
		for (const auto face_ptr : mesh->m_faces) {
			facemap[face_ptr->id] = face_ptr;
		}
		return facemap;
	}

	const std::map<int, HE_vert*> &CreateVertexMap() {
		assert(mesh != nullptr && "AdjacencyInfo.CreateVertexMap: mesh not initialized.");
		vertmap.clear();
		for (const auto vert_ptr : mesh->m_vertices) {
			vertmap[vert_ptr->id] = vert_ptr;
		}
		return vertmap;
	}

	const std::map<HE_face*, std::array<HE_vert*, 3>> &ConstructFaceVertices () {
		assert(mesh != nullptr && "AdjacencyInfo.ConstructFaceVertices: mesh not initialized.");
		assert(!vertmap.empty() && "AdjacencyInfo.ConstructFaceVertices: vertmap not initialized.");
		fvert.clear();
		for (const auto face_ptr : mesh->m_faces) {
			std::array<HE_vert*, 3> fv{ nullptr, nullptr, nullptr };
			int counter_debug = 0;
			for (int i = 0; i < 3; ++i) {
				auto vertiter = vertmap.find(face_ptr->vertid[i]);
				if (vertiter != vertmap.end()) {
					fv[i] = vertiter->second;
					counter_debug++;
				} else {
					printf("AdjacencyInfo.ConstructFaceVertices: Cannot find vertex of id %d.\n", face_ptr->vertid[i]);
					continue;
				}
			}
			assert(counter_debug == 3 && "AdjacencyInfo.CounstructFaceVertices: Cannot find all vertice for face");
			fvert[face_ptr] = fv;
		}
		return fvert;
	}

	const std::map<HE_vert*, std::forward_list<HE_face*>> &ConstructVertexFaces() {
		assert(!fvert.empty() && "AdjacencyInfo.ConstructVertexFaces: fvert not initialized.");
		vface.clear();
		for (const auto fv_iter : fvert) {
			for (const auto vert_ptr : fv_iter.second) {
				vface[vert_ptr].push_front(fv_iter.first);
			}
		}
		return vface;
	}

	// Create a map that maps a face to its adjacent faces.
	// Complexity O(V)
	const std::map<HE_face*, std::set<HE_face*>> &ConstructFaceFaces() {
		assert(!vface.empty() && "AdjacencyInfo.ConstructFaceFaces: vface not initialized.");
		fface.clear();
		for (const auto vf_iter : vface) {
			const auto &vflist = vf_iter.second;	// a forward_list of all faces around a vertex
			for (const auto f1 : vflist) {
				for (const auto f2 : vflist) {
					if (f1->id == f2->id) continue;
					if (IsFaceAdjacent(f1, f2)) {	// add f2 into f1 forward_list
						auto fiter = fface.find(f1);
						if (fiter == fface.end()) {	// if there is no key `f1` inside the map
							fface[f1] = std::set<HE_face*>{ f2 };
						} else {
							fface[f1].insert(f2);
						}
					}
				}
			}
		}
		assert(std::all_of(fface.begin(), fface.end(), [](const std::pair<HE_face*, std::set<HE_face*>> &p) {return p.second.size() <= 3;}) &&
			"AdjacencyInfo.ConstructFaceFaces: Some faces contains more than 3 adjacent faces.");
		return fface;
	}

	// Determine whether two faces are adjacent.
	// Conplexity O(1).
	static bool IsFaceAdjacent(HE_face *f1, HE_face *f2) {
		int count = 0;
		for (auto vid : f1->vertid) {
			if (vid == f2->vertid[0] || vid == f2->vertid[1] || vid == f2->vertid[2])
				count++;
		}
		return count == 2;
	}

};


public:
	TriMesh()
			: m_edges(), m_vertices(), m_faces(),
			m_adjacency_info(std::unique_ptr<AdjacencyInfo>(new AdjacencyInfo(this)))
	{}

	~TriMesh() {
		RemoveAllEdges();
		RemoveAllVertices();
		RemoveAllFaces();
	}

	// Insert method.  Note that insertion does not involve any torpology modification.
	HE_vert *InsertVertex(float x, float y, float z, int id) {
		try {
			HE_vert *vert = new HE_vert();
			vert->x = x;
			vert->y = y;
			vert->z = z;
			vert->id = id;
			m_vertices.push_back(vert);
			return vert;
		} catch (const std::exception &e) {
			printf("TriMesh.InsertVertex: %s\n", e.what());
			return nullptr;
		}
	}

	HE_face *InsertFace(int id, int vertid1, int vertid2, int vertid3) {
		try {
			HE_face *face = new HE_face();
			face->id = id;
			face->vertid[0] = vertid1;
			face->vertid[1] = vertid2;
			face->vertid[2] = vertid3;
			m_faces.push_back(face);
			return face;
		} catch (const std::exception &e) {
			printf("TriMesh.InsertFace: %s\n", e.what());
			return nullptr;
		}
	}

	HE_edge* InsertEdge(int id) {
		try {
			HE_edge *edge = new HE_edge();
			edge->id = id;
			m_edges.push_back(edge);
			return edge;
		} catch (const std::exception &e) {
			printf("TriMesh.InsertEdge: %s\n", e.what());
			return nullptr;
		}
	}


	// The algorithm of creating edges are as follows:
	// - For the initial face, six half-edges are added.
	// - Then in a BFS manner, adjacent faces are traversed and edges are added.
	// https://en.wikipedia.org/wiki/Breadth-first_search
	// This process guarantees the orientations for manifold mesh.
	// Note that it is not robust to non-manifold case.  The user is responsible to check that the underlining
	// mesh torpology does not have any non-manifold-ness.
	// This involves connecting more than two faces to a single edges, or cannot traverse all adjacent 
	// faces for a vertex by half-edge data structure (namely the vertex only points to ONE adjacent half-edge).
	// The complexity is O(Flog(F) + FE) (or O(Flog(F) + Flog(E)) with different add edge method).
	// Udpate: The complexity is O(Flog(F)).
	void AddEdgesGlobal() {
		if (m_faces.empty()) {
			printf("TriMesh.CreateEdgesGlobal: Cannot find any faces. Nothing is done.\n");
			return;
		}
		if (!m_adjacency_info->isUpdated) {
			m_adjacency_info->UpdateAdjacencyInfo();
		}
		std::queue<HE_face*> faces_queue{};
		faces_queue.push(m_faces.front());
		std::set<HE_face*> faces_set(m_faces.begin() + 1, m_faces.end());
		while(!faces_queue.empty()) {
			assert(!faces_queue.empty() && "TriMesh.AddEdgesGlobal: The mesh is not connected.");
			HE_face *f = faces_queue.front();
			faces_queue.pop();
			AddFaceEdgesGlobal(f);
			auto fface_iter = m_adjacency_info->fface.find(f);
			assert(fface_iter != m_adjacency_info->fface.end() &&
				"TriMesh.AddEdgesGlobal: Cannot find the face from face-face adjacent map");
			for (const auto fadj : fface_iter->second) {	// adjacent faces
				auto fset_it = faces_set.find(fadj);
				if (fset_it != faces_set.end()) {
					faces_set.erase(fset_it);
					faces_queue.push(fadj);
				}
			}

		}
	}

    void ComputeNormal() {
        for (auto f : m_faces) f->ComputeNormal();
        for (auto v : m_vertices) v->ComputeNormal();
    }

    // Call this function after adding all vertices and faces.
    void Update() {
        this->UpdateAdjacencyGlobal();
        this->AddEdgesGlobal();
        this->ComputeNormal();
    }

protected:

	// The following global method assumes that the graph is not complete and the edges are not added.
	void UpdateAdjacencyGlobal() {
        if (m_adjacency_info->isUpdated)
			return;
		m_adjacency_info->UpdateAdjacencyInfo();
	}

	// Given a face, add all edges around if not exists.
	// The time complexity is O(E) since `LookUpHalfEdgeGlobal` is used.  The insertion is O(1).
	// Here we discuss the various implementation that may reduce O(E) to O(log(E)).
	// In fact using adjacent info such as a map this is possible, but the essence of `AdjacencyInfo`
	// is that it is constructed once and for all, namely that we should not lookup and modify simultaneously.
	// The second workaround is to store edges directly in a set/map, then the complexity of insertion and 
	// lookup are both O(log(E)).
	void AddFaceEdgesGlobal(HE_face *face) {
		assert(face != nullptr && "TriMesh.AddFaceEdgesGlobal: Input `face` is nullptr.");
		assert(m_adjacency_info != nullptr && m_adjacency_info->isUpdated == true &&
			"TriMesh.CreateEdgesGlobal: Adjacency map is not initialized.");
		if (face->edge != nullptr) {
			printf("TriMesh.AddFaceEdgesGlobal: The edges are already inserted.\n");
			return;
		}
		std::array<HE_vert*, 3> &fvertices = m_adjacency_info->fvert.at(face);
		HE_edge *e0 = LookUpHalfEdgeGlobal(fvertices[0], fvertices[1]);
		HE_edge *e1 = LookUpHalfEdgeGlobal(fvertices[1], fvertices[2]);
		HE_edge *e2 = LookUpHalfEdgeGlobal(fvertices[2], fvertices[0]);
		if (e0 == nullptr) {
			if (e1 == nullptr) {
				if (e2 == nullptr) { // all null.
					InsertTopology(fvertices[0], fvertices[1], fvertices[2], face);
				} else {	// e0, e1 null, e2 is not.
					InsertTopology(e2, fvertices[1], face);
				}
			} else if (e2 == nullptr) { // e0, e2 null, e1 is not.
				InsertTopology(e1, fvertices[0], face);
			} else { // e0 null, e1, e2 is not.
				InsertTopology(e1, e2, face);
			}
		} else if (e1 == nullptr) {
			if (e2 == nullptr) { // e1, e2 is null, e0 is not.
				InsertTopology(e0, fvertices[2], face);
			} else { // e1 is null, e0, e2 is not.
				InsertTopology(e0, e2, face);
			}
		} else if (e2 == nullptr) { // e2 is null, e0, e1 is not.
			InsertTopology(e0, e1, face);
		} else { // e0, e1, e2 is not null.
			InsertTopology(e0, e1, e2, face);
		}
	}

	// This function assumes that the edges around `face` are all nullptr.
	// always v0->e0->v1->e1->v2->e2->v0
	void InsertTopology(HE_vert *v0, HE_vert *v1, HE_vert *v2, HE_face *face) {
		assert(v0 && v1 && v2 && face &&
			"TriMesh.InsertTopology: Input nullptr");
		HE_edge *e0 = InsertEdge(GetUniqueId<int>());
		HE_edge *e1 = InsertEdge(GetUniqueId<int>());
		HE_edge *e2 = InsertEdge(GetUniqueId<int>());
		e0->pair = InsertEdge(GetUniqueId<int>());
		e1->pair = InsertEdge(GetUniqueId<int>());
		e2->pair = InsertEdge(GetUniqueId<int>());
		e0->vert = v1;
		e1->vert = v2;
		e2->vert = v0;
		e0->pair->vert = v0;
		e1->pair->vert = v1;
		e2->pair->vert = v2;
		e0->pair->pair = e0;
		e1->pair->pair = e1;
		e2->pair->pair = e2;
		e0->next = e1;
		e1->next = e2;
		e2->next = e0;
		e0->prev = e2;
		e1->prev = e0;
		e2->prev = e1;
		e0->face = face;
		e1->face = face;
		e2->face = face;
		face->edge = e0;
		v0->edge = e0;
		v1->edge = e1;
		v2->edge = e2;
		v0->out_edge.push_front(e0);
		v1->out_edge.push_front(e1);
		v2->out_edge.push_front(e2);
        v0->out_edge.push_front(e2->pair);
		v1->out_edge.push_front(e0->pair);
		v2->out_edge.push_front(e1->pair);
	}

	// This function assumes the opposite edge of `v2` is not empty, and the other edges are all empty.
	void InsertTopology(HE_edge *e0, HE_vert *v2, HE_face *face) {
		assert(e0 != nullptr && v2 != nullptr && face != nullptr &&
			"TriMesh.InsertTopology: Input nullptr");
		assert(e0->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert((e0->face == nullptr) ^ (e0->pair->face == nullptr) &&	// exactly one of the faces should be empty
			"TriMesh.InsertTopology: Non-manifold case occur!");
		if (e0->face != nullptr) e0 = e0->pair; // Find the right orientation.
		HE_vert *v0 = e0->pair->vert;
		HE_vert *v1 = e0->vert;
		assert(v0 != nullptr && v1 != nullptr && "TriMesh.InsertTopology: Vertices are not properly initialized.");
		assert(std::find(v0->out_edge.begin(), v0->out_edge.end(), e0) != v0->out_edge.end() &&
			"TriMesh.InsertTopology: Out edges are not containes at the list.");
		assert(std::find(v1->out_edge.begin(), v1->out_edge.end(), e0->pair) != v1->out_edge.end() &&
			"TriMesh.InsertTopology: Out edges are not containes at the list.");
		HE_edge *e1 = InsertEdge(GetUniqueId<int>());
		HE_edge *e2 = InsertEdge(GetUniqueId<int>());
		e1->pair = InsertEdge(GetUniqueId<int>());
		e2->pair = InsertEdge(GetUniqueId<int>());
		e1->vert = v2;
		e2->vert = v0;
		e1->pair->vert = v1;
		e2->pair->vert = v2;
		e1->pair->pair = e1;
		e2->pair->pair = e2;
		e0->next = e1;
		e1->next = e2;
		e2->next = e0;
		e0->prev = e2;
		e1->prev = e0;
		e2->prev = e1;
		e0->face = face;
		e1->face = face;
		e2->face = face;
		face->edge = e0;
		v0->edge = e0;
		v1->edge = e1;
		v2->edge = e2;
        v0->out_edge.push_front(e2->pair);
		v1->out_edge.push_front(e1);
        v2->out_edge.push_front(e1->pair);
		v2->out_edge.push_front(e2);
	}

	// This function assumes that `e0` and `e1` are not empty, and the remaining edge is empty.
	// We want e0->v1->e1
	void InsertTopology(HE_edge *e0, HE_edge *e1, HE_face *face) {
		assert(e0 != nullptr && e1 != nullptr && face != nullptr &&
			"TriMesh.InsertTopology: Input nullptr");
		assert(e0->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert(e1->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert((e0->face == nullptr) ^ (e0->pair->face == nullptr) &&		// exactly one of the faces should be empty
			"TriMesh.InsertTopology: Non-manifold case occur!");		
		assert((e1->face == nullptr) ^ (e1->pair->face == nullptr) &&
			"TriMesh.InsertTopology: Non-manifold case occur!");
		if (e0->face) e0 = e0->pair;
		if (e1->face) e1 = e1->pair;
		assert(e0->vert);
		if (e0->vert != e1->pair->vert) { // = v1
			assert(e0->pair->vert == e1->vert);
			HE_edge *t = e0; e0 = e1; e1 = t; // swap for ease.
		}
		HE_vert *v0 = e0->pair->vert;
		HE_vert *v1 = e0->vert;
		HE_vert *v2 = e1->vert;
		assert(v0 && v2 && "TriMesh.InsertTopology: Invalid topology.");
		assert(std::find(v0->out_edge.begin(), v0->out_edge.end(), e0) != v0->out_edge.end());
		assert(std::find(v1->out_edge.begin(), v1->out_edge.end(), e1) != v1->out_edge.end());
		assert(std::find(v2->out_edge.begin(), v2->out_edge.end(), e1->pair) != v2->out_edge.end());
		assert(std::find(v1->out_edge.begin(), v1->out_edge.end(), e0->pair) != v1->out_edge.end());
		HE_edge *e2 = InsertEdge(GetUniqueId<int>());
		e2->pair = InsertEdge(GetUniqueId<int>());
		e2->vert = v0;
		e2->pair->vert = v2;
		e2->pair->pair = e2;
		e0->next = e1;
		e1->next = e2;
		e2->next = e0;
		e0->prev = e2;
		e1->prev = e0;
		e2->prev = e1;
		e0->face = face;
		e1->face = face;
		e2->face = face;
		face->edge = e0;
		v0->edge = e0;
		v1->edge = e1;
		v2->edge = e2;
		v2->out_edge.push_front(e2);
		v0->out_edge.push_front(e2->pair);
	}

	// This function assumes that `e0` and `e1` and `e2` are not empty.
	void InsertTopology(HE_edge *e0, HE_edge *e1, HE_edge *e2, HE_face *face) {
		assert(e0 != nullptr && e1 != nullptr && e2 != nullptr && face != nullptr &&
			"TriMesh.InsertTopology: Input nullptr");
		assert(e0->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert(e1->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert(e2->pair && "TriMesh.InsertTopology: Edges are not properly initialized.");
		assert((e0->face == nullptr) ^ (e0->pair->face == nullptr) &&	// exactly one of the faces should be empty
			"TriMesh.InsertTopology: Non-manifold case occur!");
		assert((e1->face == nullptr) ^ (e1->pair->face == nullptr) &&
			"TriMesh.InsertTopology: Non-manifold case occur!");
		assert((e2->face == nullptr) ^ (e2->pair->face == nullptr) &&
			"TriMesh.InsertTopology: Non-manifold case occur!");
		if (e0->face) e0 = e0->pair;
		if (e1->face) e1 = e1->pair;
		if (e2->face) e2 = e2->pair;
		assert(e0->vert && e1->vert && e2->vert);
		// still we want e0->v1->e1->v2->e2->v0->e0
		if (e0->vert != e1->pair->vert) { // =v1
			assert(e0->pair->vert == e1->vert);
			assert(e1->pair->vert == e2->vert);
			assert(e2->pair->vert == e0->vert);
			HE_edge *t = e1; e1 = e2; e2 = t;
		} else {
			assert(e1->vert == e2->pair->vert);
			assert(e2->vert == e0->pair->vert);
		}
		HE_vert *v0 = e2->vert;
		HE_vert *v1 = e0->vert;
		HE_vert *v2 = e1->vert;
		assert(std::find(v0->out_edge.begin(), v0->out_edge.end(), e0) != v0->out_edge.end());
		assert(std::find(v1->out_edge.begin(), v1->out_edge.end(), e1) != v1->out_edge.end());
		assert(std::find(v2->out_edge.begin(), v2->out_edge.end(), e2) != v2->out_edge.end());
		assert(std::find(v0->out_edge.begin(), v0->out_edge.end(), e2->pair) != v0->out_edge.end());
		assert(std::find(v1->out_edge.begin(), v1->out_edge.end(), e0->pair) != v1->out_edge.end());
		assert(std::find(v2->out_edge.begin(), v2->out_edge.end(), e1->pair) != v2->out_edge.end());
		e0->next = e1;
		e1->next = e2;
		e2->next = e0;
		e0->prev = e2;
		e1->prev = e0;
		e2->prev = e1;
		e0->face = face;
		e1->face = face;
		e2->face = face;
		face->edge = e0;
		v0->edge = e0;
		v1->edge = e1;
		v2->edge = e2;
	}

	// Look for the specific half-edge vfrom -> vto.  Return nullptr if not exists.
	// Since the lookup is among all edges stored, the time complexity is O(E).
    // Update: The complexity has been reduced to O(1).
	HE_edge *LookUpHalfEdgeGlobal(HE_vert *vfrom, HE_vert *vto) const {
		assert(vfrom != nullptr && "TriMesh.LookUpHalfEdgeGlobal: Input `from` vertex is nullptr.");
		assert(vto   != nullptr && "TriMesh.LookUpHalfEdgeGlobal: Input `to` vertex is nullptr.");
		for (auto it = vfrom->out_edge.begin(); it != vfrom->out_edge.end(); ++it) {
			if ((*it)->vert == vto) return *it;
		}
		return nullptr;
	}

public:

	using VertIter = std::vector<HE_vert*>::iterator;
	using EdgeIter = std::vector<HE_edge*>::iterator;
	using FaceIter = std::vector<HE_face*>::iterator;

    VertIter GetVerticesBegin()  {
        return std::begin(m_vertices);
	}
    VertIter GetVerticesEnd() {
		return std::end(m_vertices);
	}
    EdgeIter GetEdgesBegin() {
		return std::begin(m_edges);
	}
    EdgeIter GetEdgesEnd()  {
		return std::end(m_edges);
	}
    FaceIter GetFacesBegin() {
		return std::begin(m_faces);
	}
    FaceIter GetFacesEnd() {
		return std::end(m_faces);
	}

	// get all the adjacent vertices to vertex `v`.
	std::vector<HE_vert*> GetVertexVertices(HE_vert *v) {
		assert(v && "TriMesh.GetVertexVertices: Input nullptr.");
		std::vector<HE_edge*> out_edges = GetVertexOutEdges(v);
		auto out_vertices = std::vector<HE_vert*>(out_edges.size());
		std::transform(out_edges.begin(), out_edges.end(), out_vertices.begin(),
			[](HE_edge *e) {return e->vert;});
		assert(std::all_of(out_vertices.begin(), out_vertices.end(), [](HE_vert *v){return v != nullptr;}) &&
			"TriMesh.GetVertexVertices: Output nullptr.");
		return out_vertices;
	}

	std::vector<HE_edge*> GetVertexInEdges(HE_vert *v) {
		assert(v && "TriMesh.GetVertexInEdges: Input nullptr.");
		std::vector<HE_edge*> out_edges = GetVertexOutEdges(v);
		auto in_edges = std::vector<HE_edge*>(out_edges.size());
		std::transform(out_edges.begin(), out_edges.end(), in_edges.begin(),
			[](HE_edge *e) {return e->pair;});
		assert(std::all_of(in_edges.begin(), in_edges.end(), [](HE_edge *e){return e != nullptr;}) &&
			   "TriMesh.GetVertexInEdges: Output nullptr.");
		return in_edges;
	}

	std::vector<HE_edge*> GetVertexOutEdges(HE_vert *v) {
		assert(v && "TriMesh.GetVertexOutEdges: Input nullptr.");
		auto out_edges = std::vector<HE_edge*>{};
		if (v->edge == nullptr) {
			printf("TriMesh.GetVertexOutEdges: Vertex pointing to nothing.");
			return out_edges;
		}
		// v->edge should be guaranteed that it is an out edge as well.
		assert(v->edge->pair && v->edge->pair->vert == v &&
			"TriMesh.GetVertexOutEdges: Vertex pointing to an edge that is not an out edge.");
		HE_edge *e = v->edge;
		do {
			assert(e->pair && "TriMesh.GetVertexOutEdges: Edges are not in pairs.");
			assert(e->pair->vert && e->pair->vert->id == v->id && "TriMesh.GetVertexOutEdges: Vertex not pointing to an out edge.");
			out_edges.push_back(e);
			e = e->pair;
			if (e->next)
				e = e->next;
			else
				break;
		} while (e->id != v->edge->id);
		if (e && e->id == v->edge->id)
			return out_edges;
		else
			assert(e->next == nullptr && e->prev == nullptr && EdgeOnBoundary(e) &&
						   "TriMesh.GetVertexOutEdges: Boundary edge pointing to some face.");
		// Otherwise, some edges should be on boundary.
		out_edges.clear();
		assert(e->vert->id == v->id && "TriMesh.GetVertexOutEdges: e should be an in-edge");
		while (e) {
			assert(e->pair && "TriMesh.GetVertexOutEdges: Edges are not in pairs.");
			assert(e->vert && e->vert->id == v->id && "TriMesh.GetVertexOutEdges: e is not an in-edge.");
			out_edges.push_back(e->pair);
			e = e->pair->prev;
		}
		return out_edges;

	}

	std::vector<HE_face *> GetVertexFaces(HE_vert *v) {
		assert(v && "TriMesh.GetVertexFaces: Input nullptr.");
		auto out_edges = GetVertexOutEdges(v);
		auto vfaces = std::vector<HE_face*>(out_edges.size());
		std::transform(out_edges.begin(), out_edges.end(), vfaces.begin(),
					   [](HE_edge *e) {return e->face;});
		vfaces.erase(std::remove_if(vfaces.begin(), vfaces.end(), [](HE_face *f){return f == nullptr;}), vfaces.end());
		assert(std::all_of(vfaces.begin(), vfaces.end(), [](HE_face *f) {return f != nullptr;}) &&
			"TriMesh.GetVertexFaces: Output nullptr.");
		return vfaces;
	}

	std::vector<HE_vert *> GetFaceVertices(HE_face *f) {
		assert(f && f->edge && "TriMesh.GetFaceVertices: Input nullptr.");
		auto fedges = GetFaceEdges(f);
		auto fverts = std::vector<HE_vert *>(fedges.size());
		std::transform(fedges.begin(), fedges.end(), fverts.begin(),
						[](HE_edge *e) {return e->vert;});
		assert(std::all_of(fverts.begin(), fverts.end(), [](HE_vert *v) {return v != nullptr;}) &&
			"TriMesh.GetFaceVertices: Output nullptr.");
		return fverts;
	}

	std::vector<HE_edge *> GetFaceEdges(HE_face *f) {
		assert(f && f->edge && "TriMesh.GetFaceEdges: Input nullptr.");
		auto fedges = std::vector<HE_edge *>{};
		HE_edge *e = f->edge;
		do {
			fedges.push_back(e);
			assert(e->next && "TriMesh.GetFaceEdges: Edge inside a face has null next pointer.");
			e = e->next;
		} while (e != f->edge);
		assert(fedges.size() == 3 && "TriMesh.GetFaceEdges: Face have edge number not equal to three.");	// trimesh
		return fedges;
	}

	std::vector<HE_face *> GetFaceFaces(HE_face *f) {
		assert(f && f->edge && "TriMesh.GetFaceFaces: Input nullptr.");
		auto fedges = GetFaceEdges(f);
		auto ffaces = std::vector<HE_face *>(fedges.size());
		assert(std::all_of(fedges.begin(), fedges.end(), [](HE_edge *e) {return e && e->pair;}) &&
			"TriMesh.GetFaceFaces: Edges have null pairs.");
		std::transform(fedges.begin(), fedges.end(), ffaces.begin(),
						[](HE_edge *e) {return e->pair->face;});
		// remove all nullptrs
		ffaces.erase(std::remove_if(ffaces.begin(), ffaces.end(),
						[](HE_face *f) {return f == nullptr;}),
					 ffaces.end());
		return ffaces;
	}


	// helpers

	bool HalfEdgeOnBoundary(HE_edge *e) {
		assert(e && "TriMesh.HalfEdgeOnBoundary: Input nullptr");
		return e->face == nullptr;
	}

	bool EdgeOnBoundary(HE_edge *e) {
		return HalfEdgeOnBoundary(e) || HalfEdgeOnBoundary(e->pair);
	}

	std::size_t NumVertices() const {
		return m_vertices.size();
	}
	std::size_t NumEdges() const {
		return m_edges.size();
	}
	std::size_t NumFaces() const {
		return m_faces.size();
	}
private:

	// Danger! Calling the following functions would make other elements pointing to them dangling! Use with care!
	void RemoveAllEdges() {
		for (auto p : m_edges) {
			if (p) {
				delete p;
				p = nullptr;
			}
		}
		m_edges.clear();
	}
	void RemoveAllVertices() {
		for (auto p : m_vertices) {
			if (p) {
				delete p;
				p = nullptr;
			}
		}
		m_vertices.clear();
	}
	void RemoveAllFaces() {
		for (auto p : m_faces) {
			if (p) {
				delete p;
				p = nullptr;
			}
		}
		m_faces.clear();
	}

	// Use temporarily for edges now.
	template<typename IntType>
	IntType GetUniqueId() {
		static std::atomic<IntType> count_atomic(0);
		return count_atomic.fetch_add(IntType(1));
	}


private:
	std::vector<HE_edge*> m_edges;
	std::vector<HE_vert*> m_vertices;
	std::vector<HE_face*> m_faces;
	std::unique_ptr<AdjacencyInfo> m_adjacency_info;
};

#endif //OPENGLPLAYGROUND_TRIMESH_H
