// ==============================================================================================
// File: Mesh.cpp
// Description: Implementation of 3D mesh data structures, normal vector calculations,
//              and procedural geometric shape generation algorithms (Cubes, Boxes,
//              Cylinders, Tori, and Architectural Arches).
// ==============================================================================================

#include "pch.h"
#include "Mesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==============================================================================================
// Method: Mesh::addVertex
// Purpose: Appends a new 3D point and its texture coordinates into the vertices list.
// Parameters:
//   x, y, z: 3D position in local object coordinates.
//   u, v:    2D texture coordinates (horizontal U and vertical V on an image).
// ==============================================================================================
void Mesh::addVertex(float x, float y, float z, float u, float v) {
    vertices.push_back({ x, y, z, u, v });
}

// ==============================================================================================
// Method: Mesh::addFace
// Purpose: Defines a polygonal face by storing an ordered list of vertex indices.
// Parameters:
//   indices: A list of integers where each integer is the index of a vertex in 'vertices'.
//            Order determines the front-facing direction (counter-clockwise winding rule).
// ==============================================================================================
void Mesh::addFace(const std::vector<int>& indices) {
    faces.push_back({ indices });
}

// ==============================================================================================
// Method: Mesh::computeNormals
// Purpose: Automatically computes surface normal vectors for all vertices in the mesh.
// ==============================================================================================
void Mesh::computeNormals() {
    // Initialize the normals array with zero vectors, one for each vertex.
    normals.assign(vertices.size(), { 0.0f, 0.0f, 0.0f });

    // Step 1: Loop through each polygon face in the mesh
    for (const auto& face : faces) {
        // A polygon must have at least 3 vertices to form a 2D plane in 3D space.
        if (face.indices.size() < 3) continue;

        // Retrieve the first 3 vertices of this face
        Vertex v0 = vertices[face.indices[0]];
        Vertex v1 = vertices[face.indices[1]];
        Vertex v2 = vertices[face.indices[2]];

        // Vector A: from vertex 0 to vertex 1
        float ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        // Vector B: from vertex 0 to vertex 2
        float bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;

        // Calculate Cross Product: Vector N = Vector A x Vector B
        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        // Accumulate this face normal into each vertex that belongs to this face.
        // If a vertex is shared between multiple faces, their normals add up (creating smooth shading).
        for (int idx : face.indices) {
            normals[idx].nx += nx;
            normals[idx].ny += ny;
            normals[idx].nz += nz;
        }
    }

    // Step 2: Normalize all accumulated normals so their length is exactly 1.0 (unit vectors).
    for (auto& n : normals) {
        // 3D vector length formula: sqrt(x^2 + y^2 + z^2)
        float length = std::sqrt(n.nx * n.nx + n.ny * n.ny + n.nz * n.nz);
        // Avoid division by zero if a vertex had no faces or degenerate geometry
        if (length > 0.00001f) {
            n.nx /= length;
            n.ny /= length;
            n.nz /= length;
        }
    }
}

// ==============================================================================================
// Method: Mesh::createCube
// Purpose: Convenience function to create an equilateral cube where width = height = depth = size.
// ==============================================================================================
Mesh Mesh::createCube(float size) {
    return createTexturedBox(size, size, size, 1.0f, 1.0f);
}

// ==============================================================================================
// Method: Mesh::createTexturedBox
// Purpose: Generates a 6-sided 3D rectangular box centered at (0, 0, 0).
// Parameters:
//   w: Width along X axis (left to right).
//   h: Height along Y axis (bottom to top).
//   d: Depth along Z axis (front to back).
//   tileU: Number of times the texture repeats horizontally across each face.
//   tileV: Number of times the texture repeats vertically across each face.
// ==============================================================================================
Mesh Mesh::createTexturedBox(float w, float h, float d, float tileU, float tileV) {
    Mesh mesh;
    // Half-extents from the center (0, 0, 0)
    float x = w / 2.0f, y = h / 2.0f, z = d / 2.0f;

    // --- Face 1: Front (+Z direction) ---
    mesh.addVertex(-x, -y, z, 0.0f, 0.0f);    // Bottom-Left
    mesh.addVertex( x, -y, z, tileU, 0.0f);   // Bottom-Right
    mesh.addVertex( x,  y, z, tileU, tileV);  // Top-Right
    mesh.addVertex(-x,  y, z, 0.0f, tileV);   // Top-Left
    mesh.addFace({ 0, 1, 2, 3 });

    // --- Face 2: Back (-Z direction) ---
    mesh.addVertex(-x, -y, -z, tileU, 0.0f);  // Bottom-Right from behind
    mesh.addVertex(-x,  y, -z, tileU, tileV); // Top-Right from behind
    mesh.addVertex( x,  y, -z, 0.0f, tileV);  // Top-Left from behind
    mesh.addVertex( x, -y, -z, 0.0f, 0.0f);   // Bottom-Left from behind
    mesh.addFace({ 4, 5, 6, 7 });

    // --- Face 3: Top (+Y direction) ---
    mesh.addVertex(-x, y, -z, 0.0f, tileV);
    mesh.addVertex(-x, y,  z, 0.0f, 0.0f);
    mesh.addVertex( x, y,  z, tileU, 0.0f);
    mesh.addVertex( x, y, -z, tileU, tileV);
    mesh.addFace({ 8, 9, 10, 11 });

    // --- Face 4: Bottom (-Y direction) ---
    mesh.addVertex(-x, -y, -z, 0.0f, 0.0f);
    mesh.addVertex( x, -y, -z, tileU, 0.0f);
    mesh.addVertex( x, -y,  z, tileU, tileV);
    mesh.addVertex(-x, -y,  z, 0.0f, tileV);
    mesh.addFace({ 12, 13, 14, 15 });

    // --- Face 5: Right (+X direction) ---
    mesh.addVertex(x, -y, -z, 0.0f, 0.0f);
    mesh.addVertex(x,  y, -z, tileU, 0.0f);
    mesh.addVertex(x,  y,  z, tileU, tileV);
    mesh.addVertex(x, -y,  z, 0.0f, tileV);
    mesh.addFace({ 16, 17, 18, 19 });

    // --- Face 6: Left (-X direction) ---
    mesh.addVertex(-x, -y, -z, tileU, 0.0f);
    mesh.addVertex(-x, -y,  z, 0.0f, 0.0f);
    mesh.addVertex(-x,  y,  z, 0.0f, tileV);
    mesh.addVertex(-x,  y, -z, tileU, tileV);
    mesh.addFace({ 20, 21, 22, 23 });

    // Compute lighting normals for all 24 vertices
    mesh.computeNormals();
    return mesh;
}

// ==============================================================================================
// Method: Mesh::createTexturedBoxUV
// Purpose: Creates a 3D box with explicit texture UV coordinate ranges.
// ==============================================================================================
Mesh Mesh::createTexturedBoxUV(float w, float h, float d, float uMin, float uMax, float vMin, float vMax) {
    Mesh mesh;
    float x = w / 2.0f, y = h / 2.0f, z = d / 2.0f;

    // --- Front Face (+Z) with custom UV bounds ---
    mesh.addVertex(-x, -y, z, uMin, vMin);
    mesh.addVertex( x, -y, z, uMax, vMin);
    mesh.addVertex( x,  y, z, uMax, vMax);
    mesh.addVertex(-x,  y, z, uMin, vMax);
    mesh.addFace({ 0, 1, 2, 3 });

    // --- Back Face (-Z) with custom UV bounds ---
    mesh.addVertex( x, -y, -z, uMin, vMin);
    mesh.addVertex(-x, -y, -z, uMax, vMin);
    mesh.addVertex(-x,  y, -z, uMax, vMax);
    mesh.addVertex( x,  y, -z, uMin, vMax);
    mesh.addFace({ 4, 5, 6, 7 });

    // --- Top Face (+Y) ---
    mesh.addVertex(-x, y, -z, uMin, vMax);
    mesh.addVertex(-x, y,  z, uMin, vMin);
    mesh.addVertex( x, y,  z, uMax, vMin);
    mesh.addVertex( x, y, -z, uMax, vMax);
    mesh.addFace({ 8, 9, 10, 11 });

    // --- Bottom Face (-Y) ---
    mesh.addVertex(-x, -y, -z, uMin, vMin);
    mesh.addVertex( x, -y, -z, uMax, vMin);
    mesh.addVertex( x, -y,  z, uMax, vMax);
    mesh.addVertex(-x, -y,  z, uMin, vMax);
    mesh.addFace({ 12, 13, 14, 15 });

    // --- Right Face (+X) ---
    mesh.addVertex(x, -y, -z, 0.0f, vMin);
    mesh.addVertex(x,  y, -z, 1.0f, vMin);
    mesh.addVertex(x,  y,  z, 1.0f, vMax);
    mesh.addVertex(x, -y,  z, 0.0f, vMax);
    mesh.addFace({ 16, 17, 18, 19 });

    // --- Left Face (-X) ---
    mesh.addVertex(-x, -y, -z, 1.0f, vMin);
    mesh.addVertex(-x, -y,  z, 0.0f, vMin);
    mesh.addVertex(-x,  y,  z, 0.0f, vMax);
    mesh.addVertex(-x,  y, -z, 1.0f, vMax);
    mesh.addFace({ 20, 21, 22, 23 });

    mesh.computeNormals();
    return mesh;
}

// ==============================================================================================
// Method: Mesh::createTorus
// Purpose: Generates a 3D torus (a ring or donut shape).
// Parameters:
//   r:     Radius of the tube itself (thickness of the ring).
//   R:     Distance from the central donut hole to the center of the tube.
//   rings: Number of circular segments along the main circumference (around the ring).
//   sides: Number of circular segments around the tube cross-section.
// ==============================================================================================
Mesh Mesh::createTorus(float r, float R, int rings, int sides) {
    Mesh mesh;
    // Loop around the main donut ring (u)
    for (int i = 0; i < rings; ++i) {
        float u1 = i * (2.0f * (float)M_PI / rings);
        float u2 = (i + 1) * (2.0f * (float)M_PI / rings);

        // Loop around the tube cross-section (v)
        for (int j = 0; j < sides; ++j) {
            float v1 = j * (2.0f * (float)M_PI / sides);
            float v2 = (j + 1) * (2.0f * (float)M_PI / sides);

            int baseIdx = (int)mesh.vertices.size();

            // 4 vertices forming a small quad on the curved surface of the torus
            mesh.addVertex((R + r * cosf(v1)) * cosf(u1), r * sinf(v1), (R + r * cosf(v1)) * sinf(u1));
            mesh.addVertex((R + r * cosf(v1)) * cosf(u2), r * sinf(v1), (R + r * cosf(v1)) * sinf(u2));
            mesh.addVertex((R + r * cosf(v2)) * cosf(u2), r * sinf(v2), (R + r * cosf(v2)) * sinf(u2));
            mesh.addVertex((R + r * cosf(v2)) * cosf(u1), r * sinf(v2), (R + r * cosf(v2)) * sinf(u1));

            // Connect the 4 points to form a polygon face
            mesh.addFace({ baseIdx, baseIdx + 1, baseIdx + 2, baseIdx + 3 });
        }
    }
    mesh.computeNormals();
    return mesh;
}

// ==============================================================================================
// Method: Mesh::createCylinder
// Purpose: Generates a hollow 3D cylinder tube (used for temple pillars, torch sconce arms, bones).
// Parameters:
//   radius: Radius of the circular tube.
//   height: Total vertical length of the cylinder.
//   slices: Number of vertical strips around the cylinder (higher = smoother circle).
// ==============================================================================================
Mesh Mesh::createCylinder(float radius, float height, int slices) {
    Mesh mesh;
    float halfH = height / 2.0f; // Half-height so cylinder is vertically centered at Y = 0

    for (int i = 0; i < slices; ++i) {
        // Angles around the circle for this slice
        float theta1 = i * (2.0f * (float)M_PI / slices);
        float theta2 = (i + 1) * (2.0f * (float)M_PI / slices);

        // Normalized texture coordinates (U ranges from 0.0 to 1.0 around the tube)
        float u1 = (float)i / slices;
        float u2 = (float)(i + 1) / slices;

        int baseIdx = (int)mesh.vertices.size();
        // Bottom vertex 1
        mesh.addVertex(radius * cosf(theta1), -halfH, radius * sinf(theta1), u1 * 2.0f, 0.0f);
        // Bottom vertex 2
        mesh.addVertex(radius * cosf(theta2), -halfH, radius * sinf(theta2), u2 * 2.0f, 0.0f);
        // Top vertex 2
        mesh.addVertex(radius * cosf(theta2),  halfH, radius * sinf(theta2), u2 * 2.0f, 2.0f);
        // Top vertex 1
        mesh.addVertex(radius * cosf(theta1),  halfH, radius * sinf(theta1), u1 * 2.0f, 2.0f);

        mesh.addFace({ baseIdx, baseIdx + 1, baseIdx + 2, baseIdx + 3 });
    }
    mesh.computeNormals();
    return mesh;
}

// ==============================================================================================
// Method: Mesh::createArch
// Purpose: Generates a curved 3D architectural arch (semicircular ring segment) for doorways.
// Parameters:
//   innerRadius: Radius of the inner open curved edge.
//   outerRadius: Radius of the outer stone border.
//   depth:       Wall thickness along the Z axis.
//   segments:    Subdivisions along the 180-degree semicircular curve.
// ==============================================================================================
Mesh Mesh::createArch(float innerRadius, float outerRadius, float depth, int segments) {
    Mesh mesh;
    float halfD = depth / 2.0f;

    for (int i = 0; i < segments; ++i) {
        // Step theta from 0 to PI (180 degrees, a complete semicircle)
        float theta1 = i * ((float)M_PI / segments);
        float theta2 = (i + 1) * ((float)M_PI / segments);

        float cos1 = cosf(theta1), sin1 = sinf(theta1);
        float cos2 = cosf(theta2), sin2 = sinf(theta2);

        float u1 = ((float)i / segments) * 4.0f;
        float u2 = ((float)(i + 1) / segments) * 4.0f;

        // 1. Front face (+Z = halfD) - counter-clockwise winding for outward normal
        int fIdx = (int)mesh.vertices.size();
        mesh.addVertex(innerRadius * cos1, innerRadius * sin1, halfD, u1, 0.0f);
        mesh.addVertex(innerRadius * cos2, innerRadius * sin2, halfD, u2, 0.0f);
        mesh.addVertex(outerRadius * cos2, outerRadius * sin2, halfD, u2, 1.0f);
        mesh.addVertex(outerRadius * cos1, outerRadius * sin1, halfD, u1, 1.0f);
        mesh.addFace({ fIdx, fIdx + 3, fIdx + 2, fIdx + 1 });

        // 2. Back face (-Z = -halfD) - outward normal pointing toward -Z
        int bIdx = (int)mesh.vertices.size();
        mesh.addVertex(outerRadius * cos1, outerRadius * sin1, -halfD, u1, 1.0f);
        mesh.addVertex(outerRadius * cos2, outerRadius * sin2, -halfD, u2, 1.0f);
        mesh.addVertex(innerRadius * cos2, innerRadius * sin2, -halfD, u2, 0.0f);
        mesh.addVertex(innerRadius * cos1, innerRadius * sin1, -halfD, u1, 0.0f);
        mesh.addFace({ bIdx, bIdx + 3, bIdx + 2, bIdx + 1 });

        // 3. Inner soffit (curved underside facing inward toward doorway opening)
        int inIdx = (int)mesh.vertices.size();
        mesh.addVertex(innerRadius * cos1, innerRadius * sin1, -halfD, u1, 0.0f);
        mesh.addVertex(innerRadius * cos2, innerRadius * sin2, -halfD, u2, 0.0f);
        mesh.addVertex(innerRadius * cos2, innerRadius * sin2,  halfD, u2, 1.0f);
        mesh.addVertex(innerRadius * cos1, innerRadius * sin1,  halfD, u1, 1.0f);
        mesh.addFace({ inIdx, inIdx + 1, inIdx + 2, inIdx + 3 });

        // 4. Outer extrados (outer rim facing upward and outward)
        int outIdx = (int)mesh.vertices.size();
        mesh.addVertex(outerRadius * cos1, outerRadius * sin1,  halfD, u1, 1.0f);
        mesh.addVertex(outerRadius * cos2, outerRadius * sin2,  halfD, u2, 1.0f);
        mesh.addVertex(outerRadius * cos2, outerRadius * sin2, -halfD, u2, 0.0f);
        mesh.addVertex(outerRadius * cos1, outerRadius * sin1, -halfD, u1, 0.0f);
        mesh.addFace({ outIdx, outIdx + 1, outIdx + 2, outIdx + 3 });
    }

    mesh.computeNormals();
    return mesh;
}

// ==============================================================================================
// Method: Mesh::createArchSpandrels
// Purpose: Fills the rectangular wall area directly above a curved archway.
// Parameters:
//   archRadius: Radius of the semicircular arch opening.
//   w, h:       Width and height of the bounding rectangular wall section.
//   depth:      Thickness of the wall.
//   uMin, uMax, vMin, vMax: Texture coordinates of this region within the whole wall.
//   segments:   Subdivisions along the curve.
// ==============================================================================================
Mesh Mesh::createArchSpandrels(float archRadius, float w, float h, float depth, float uMin, float uMax, float vMin, float vMax, int segments) {
    Mesh mesh;
    float halfD = depth / 2.0f;
    float halfW = w / 2.0f;

    // Helper lambda: Given an angle 'theta', finds where the ray from (0, 0) intersects
    // the rectangular boundary defined by x in [-halfW, halfW] and y in [0, h].
    auto getBoxPt = [&](float theta) -> std::pair<float, float> {
        float cosT = cosf(theta);
        float sinT = sinf(theta);
        if (sinT < 1e-4f) sinT = 1e-4f; // Prevent division by zero at horizontal angles

        // 1. Check intersection with top horizontal edge (y = h)
        float tx = h * cosT / sinT;
        if (tx >= -halfW && tx <= halfW) {
            return { tx, h };
        }
        // 2. Check intersection with right vertical edge (x = halfW)
        if (cosT > 0.0f) {
            float ty = halfW * sinT / cosT;
            if (ty < 0.0f) ty = 0.0f;
            if (ty > h) ty = h;
            return { halfW, ty };
        }
        // 3. Check intersection with left vertical edge (x = -halfW)
        float ty = -halfW * sinT / cosT;
        if (ty < 0.0f) ty = 0.0f;
        if (ty > h) ty = h;
        return { -halfW, ty };
    };

    // Helper lambda: Computes linearly interpolated UV texture coordinates for any (px, py)
    auto calcUV = [&](float px, float py) -> std::pair<float, float> {
        float u = uMin + ((px + halfW) / w) * (uMax - uMin);
        float v = vMin + (py / h) * (vMax - vMin);
        return { u, v };
    };

    // Construct quads connecting the inner curved arch perimeter to the outer rectangular box
    for (int i = 0; i < segments; ++i) {
        float t1 = i * ((float)M_PI / segments);
        float t2 = (i + 1) * ((float)M_PI / segments);

        // Inner circular arch points
        float inX1 = archRadius * cosf(t1), inY1 = archRadius * sinf(t1);
        float inX2 = archRadius * cosf(t2), inY2 = archRadius * sinf(t2);

        // Outer rectangular box intersection points
        auto out1 = getBoxPt(t1);
        auto out2 = getBoxPt(t2);

        // Calculate smooth continuous UV coordinates for all four points
        auto uvIn1 = calcUV(inX1, inY1);
        auto uvIn2 = calcUV(inX2, inY2);
        auto uvOut1 = calcUV(out1.first, out1.second);
        auto uvOut2 = calcUV(out2.first, out2.second);

        // 1. Front Wall Face (+Z)
        int fIdx = (int)mesh.vertices.size();
        mesh.addVertex(inX1, inY1, halfD, uvIn1.first, uvIn1.second);
        mesh.addVertex(inX2, inY2, halfD, uvIn2.first, uvIn2.second);
        mesh.addVertex(out2.first, out2.second, halfD, uvOut2.first, uvOut2.second);
        mesh.addVertex(out1.first, out1.second, halfD, uvOut1.first, uvOut1.second);
        mesh.addFace({ fIdx, fIdx + 3, fIdx + 2, fIdx + 1 });

        // 2. Back Wall Face (-Z)
        int bIdx = (int)mesh.vertices.size();
        mesh.addVertex(out1.first, out1.second, -halfD, uvOut1.first, uvOut1.second);
        mesh.addVertex(out2.first, out2.second, -halfD, uvOut2.first, uvOut2.second);
        mesh.addVertex(inX2, inY2, -halfD, uvIn2.first, uvIn2.second);
        mesh.addVertex(inX1, inY1, -halfD, uvIn1.first, uvIn1.second);
        mesh.addFace({ bIdx, bIdx + 3, bIdx + 2, bIdx + 1 });

        // 3. Inner curved boundary (soffit contact surface)
        int inIdx = (int)mesh.vertices.size();
        mesh.addVertex(inX1, inY1, -halfD, uvIn1.first, 0.0f);
        mesh.addVertex(inX2, inY2, -halfD, uvIn2.first, 0.0f);
        mesh.addVertex(inX2, inY2,  halfD, uvIn2.first, 1.0f);
        mesh.addVertex(inX1, inY1,  halfD, uvIn1.first, 1.0f);
        mesh.addFace({ inIdx, inIdx + 1, inIdx + 2, inIdx + 3 });
    }

    mesh.computeNormals();
    return mesh;
}