// ==============================================================================================
// File: MeshRenderer.cpp
// Description: Implementation of the OpenGL rendering pipeline for Mesh objects.
// ==============================================================================================

#include "pch.h"
#include "MeshRenderer.h"
#include <glut.h>

// ==============================================================================================
// Method: MeshRenderer::renderSolid
// Purpose: Sends mesh geometry to the GPU using OpenGL immediate mode.
// Parameters:
//   mesh: Reference to the Mesh object containing vertices, normals, and polygon faces.
// ==============================================================================================
void MeshRenderer::renderSolid(const Mesh& mesh) {
    // Iterate through every polygon face defined in the mesh
    for (const auto& face : mesh.faces) {
        glBegin(GL_POLYGON);
        for (int idx : face.indices) {
            // Safety check: ensure index is valid within the normals array
            if (idx >= 0 && (size_t)idx < mesh.normals.size()) {
                glNormal3f(mesh.normals[idx].nx, mesh.normals[idx].ny, mesh.normals[idx].nz);
            }
            // Safety check: ensure index is valid within the vertices array
            if (idx >= 0 && (size_t)idx < mesh.vertices.size()) {
                glTexCoord2f(mesh.vertices[idx].u, mesh.vertices[idx].v);
                glVertex3f(mesh.vertices[idx].x, mesh.vertices[idx].y, mesh.vertices[idx].z);
            }
        }
        glEnd();
    }
}

// ==============================================================================================
// Method: MeshRenderer::renderWireframe
// Purpose: Renders the mesh as a wireframe outline (edges only, no solid filled interior).
// ==============================================================================================
void MeshRenderer::renderWireframe(const Mesh& mesh) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe line drawing
    renderSolid(mesh);                         // Draw geometry as lines
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore solid fill mode
}