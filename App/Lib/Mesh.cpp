#include "pch.h"
#include "Mesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Mesh::addVertex(float x, float y, float z, float u, float v) {
    vertices.push_back({ x, y, z, u, v });
}

void Mesh::addFace(const std::vector<int>& indices) {
    faces.push_back({ indices });
}

void Mesh::computeNormals() {
    normals.assign(vertices.size(), { 0.0f, 0.0f, 0.0f });

    for (const auto& face : faces) {
        if (face.indices.size() < 3) continue;

        Vertex v0 = vertices[face.indices[0]];
        Vertex v1 = vertices[face.indices[1]];
        Vertex v2 = vertices[face.indices[2]];

        float ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        float bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        for (int idx : face.indices) {
            normals[idx].nx += nx;
            normals[idx].ny += ny;
            normals[idx].nz += nz;
        }
    }

    for (auto& n : normals) {
        float length = std::sqrt(n.nx * n.nx + n.ny * n.ny + n.nz * n.nz);
        if (length > 0.00001f) {
            n.nx /= length;
            n.ny /= length;
            n.nz /= length;
        }
    }
}

Mesh Mesh::createCube(float size) {
    return createTexturedBox(size, size, size, 1.0f, 1.0f);
}

Mesh Mesh::createTexturedBox(float w, float h, float d, float tileU, float tileV) {
    Mesh mesh;
    float x = w / 2.0f, y = h / 2.0f, z = d / 2.0f;

    // Front (+Z)
    mesh.addVertex(-x, -y, z, 0.0f, 0.0f);
    mesh.addVertex(x, -y, z, tileU, 0.0f);
    mesh.addVertex(x, y, z, tileU, tileV);
    mesh.addVertex(-x, y, z, 0.0f, tileV);
    mesh.addFace({ 0, 1, 2, 3 });

    // Back (-Z)
    mesh.addVertex(-x, -y, -z, tileU, 0.0f);
    mesh.addVertex(-x, y, -z, tileU, tileV);
    mesh.addVertex(x, y, -z, 0.0f, tileV);
    mesh.addVertex(x, -y, -z, 0.0f, 0.0f);
    mesh.addFace({ 4, 5, 6, 7 });

    // Top (+Y)
    mesh.addVertex(-x, y, -z, 0.0f, tileV);
    mesh.addVertex(-x, y, z, 0.0f, 0.0f);
    mesh.addVertex(x, y, z, tileU, 0.0f);
    mesh.addVertex(x, y, -z, tileU, tileV);
    mesh.addFace({ 8, 9, 10, 11 });

    // Bottom (-Y)
    mesh.addVertex(-x, -y, -z, 0.0f, 0.0f);
    mesh.addVertex(x, -y, -z, tileU, 0.0f);
    mesh.addVertex(x, -y, z, tileU, tileV);
    mesh.addVertex(-x, -y, z, 0.0f, tileV);
    mesh.addFace({ 12, 13, 14, 15 });

    // Right (+X)
    mesh.addVertex(x, -y, -z, 0.0f, 0.0f);
    mesh.addVertex(x, y, -z, tileU, 0.0f);
    mesh.addVertex(x, y, z, tileU, tileV);
    mesh.addVertex(x, -y, z, 0.0f, tileV);
    mesh.addFace({ 16, 17, 18, 19 });

    // Left (-X)
    mesh.addVertex(-x, -y, -z, tileU, 0.0f);
    mesh.addVertex(-x, -y, z, 0.0f, 0.0f);
    mesh.addVertex(-x, y, z, 0.0f, tileV);
    mesh.addVertex(-x, y, -z, tileU, tileV);
    mesh.addFace({ 20, 21, 22, 23 });

    mesh.computeNormals();
    return mesh;
}

Mesh Mesh::createTorus(float r, float R, int rings, int sides) {
    Mesh mesh;
    for (int i = 0; i < rings; ++i) {
        float u1 = i * (2.0f * (float)M_PI / rings);
        float u2 = (i + 1) * (2.0f * (float)M_PI / rings);

        for (int j = 0; j < sides; ++j) {
            float v1 = j * (2.0f * (float)M_PI / sides);
            float v2 = (j + 1) * (2.0f * (float)M_PI / sides);

            int baseIdx = (int)mesh.vertices.size();

            mesh.addVertex((R + r * cosf(v1)) * cosf(u1), r * sinf(v1), (R + r * cosf(v1)) * sinf(u1));
            mesh.addVertex((R + r * cosf(v1)) * cosf(u2), r * sinf(v1), (R + r * cosf(v1)) * sinf(u2));
            mesh.addVertex((R + r * cosf(v2)) * cosf(u2), r * sinf(v2), (R + r * cosf(v2)) * sinf(u2));
            mesh.addVertex((R + r * cosf(v2)) * cosf(u1), r * sinf(v2), (R + r * cosf(v2)) * sinf(u1));

            mesh.addFace({ baseIdx, baseIdx + 1, baseIdx + 2, baseIdx + 3 });
        }
    }
    mesh.computeNormals();
    return mesh;
}

Mesh Mesh::createCylinder(float radius, float height, int slices) {
    Mesh mesh;
    float halfH = height / 2.0f;
    for (int i = 0; i < slices; ++i) {
        float theta1 = i * (2.0f * (float)M_PI / slices);
        float theta2 = (i + 1) * (2.0f * (float)M_PI / slices);

        float u1 = (float)i / slices;
        float u2 = (float)(i + 1) / slices;

        int baseIdx = (int)mesh.vertices.size();
        mesh.addVertex(radius * cosf(theta1), -halfH, radius * sinf(theta1), u1 * 2.0f, 0.0f);
        mesh.addVertex(radius * cosf(theta2), -halfH, radius * sinf(theta2), u2 * 2.0f, 0.0f);
        mesh.addVertex(radius * cosf(theta2), halfH, radius * sinf(theta2), u2 * 2.0f, 2.0f);
        mesh.addVertex(radius * cosf(theta1), halfH, radius * sinf(theta1), u1 * 2.0f, 2.0f);

        mesh.addFace({ baseIdx, baseIdx + 1, baseIdx + 2, baseIdx + 3 });
    }
    mesh.computeNormals();
    return mesh;
}