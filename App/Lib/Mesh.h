#pragma once
#include <vector>

struct Vertex {
    float x, y, z;
    float u, v; // Texture mapping coordinates
};

struct Normal {
    float nx, ny, nz;
};

struct Face {
    std::vector<int> indices;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<Face> faces;

    void addVertex(float x, float y, float z, float u = 0.0f, float v = 0.0f);
    void addFace(const std::vector<int>& indices);
    void computeNormals();

    static Mesh createCube(float size);
    static Mesh createTexturedBox(float w, float h, float d, float tileU = 1.0f, float tileV = 1.0f);
    static Mesh createCylinder(float radius, float height, int slices);
    static Mesh createTorus(float r, float R, int rings, int sides);
};