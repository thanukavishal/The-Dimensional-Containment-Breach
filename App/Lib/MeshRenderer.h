#pragma once
#include "Mesh.h"

class MeshRenderer {
public:
    // Draws a mesh as solid filled polygons with lighting normals and texture coordinates applied.
    void renderSolid(const Mesh& mesh);

    // Draws a mesh in wireframe mode (showing the polygonal edges/wireframe grid).
    void renderWireframe(const Mesh& mesh);
};