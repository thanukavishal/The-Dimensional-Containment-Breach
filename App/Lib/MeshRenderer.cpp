#include "pch.h"
#include "MeshRenderer.h"
#include <glut.h>

void MeshRenderer::renderSolid(const Mesh& mesh) {
    for (const auto& face : mesh.faces) {
        glBegin(GL_POLYGON);
        for (int idx : face.indices) {
            if (idx >= 0 && (size_t)idx < mesh.normals.size()) {
                glNormal3f(mesh.normals[idx].nx, mesh.normals[idx].ny, mesh.normals[idx].nz);
            }
            if (idx >= 0 && (size_t)idx < mesh.vertices.size()) {
                glTexCoord2f(mesh.vertices[idx].u, mesh.vertices[idx].v);
                glVertex3f(mesh.vertices[idx].x, mesh.vertices[idx].y, mesh.vertices[idx].z);
            }
        }
        glEnd();
    }
}

void MeshRenderer::renderWireframe(const Mesh& mesh) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    renderSolid(mesh);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}