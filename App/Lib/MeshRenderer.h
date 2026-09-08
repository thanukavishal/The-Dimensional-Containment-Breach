#pragma once
// ==============================================================================================
// File: MeshRenderer.h
// Description: Defines the MeshRenderer class responsible for drawing 3D Mesh objects with OpenGL.
// ==============================================================================================

#include "Mesh.h"

class MeshRenderer {
public:
    // Draws a mesh as solid filled polygons with lighting normals and texture coordinates applied.
    void renderSolid(const Mesh& mesh);

    // Draws a mesh in wireframe mode (showing the polygonal edges/wireframe grid).
    void renderWireframe(const Mesh& mesh);
};