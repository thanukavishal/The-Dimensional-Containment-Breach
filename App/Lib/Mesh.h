#pragma once
// ==============================================================================================
// File: Mesh.h - Defines core data structures and classes for 3D procedural geometry.
// ==============================================================================================

#include <vector>

// ==============================================================================================
// Struct: Vertex
// Purpose: Represents a single point in 3D space, along with its 2D texture mapping coordinate.
// ==============================================================================================
struct Vertex {
    float x, y, z; // 3D spatial coordinates in local object space
    float u, v;    // 2D Texture mapping coordinates (U = horizontal, V = vertical)
};

// ==============================================================================================
// Struct: Normal
// Purpose: Stores a 3D unit vector perpendicular to a surface at a vertex.
// ==============================================================================================
struct Normal {
    float nx, ny, nz; // Unit directional vector perpendicular to the surface (length = 1.0)
};

// ==============================================================================================
// Struct: Face
// Purpose: Defines a polygon (typically a quad or triangle) by referencing vertex indices.
// ==============================================================================================
struct Face {
    std::vector<int> indices; // List of zero-based indices pointing to vertices in Mesh::vertices
};

// ==============================================================================================
// Class: Mesh
// Purpose: Container for a 3D model geometry and a procedural geometry generator.
// ==============================================================================================
class Mesh {
public:
    // -----------------------------------------------------------------------------------------
    // Member Variables
    // -----------------------------------------------------------------------------------------
    std::vector<Vertex> vertices; // List of all 3D points and their texture coordinates
    std::vector<Normal> normals;   // List of surface normal vectors (one per vertex)
    std::vector<Face> faces;       // List of polygonal faces connecting the vertices

    // -----------------------------------------------------------------------------------------
    // Construction & Computation Methods
    // -----------------------------------------------------------------------------------------

    // Adds a new vertex to the internal vertices list.
    // x, y, z: 3D spatial position.
    // u, v: 2D texture coordinates (defaulting to 0.0 if not specified).
    void addVertex(float x, float y, float z, float u = 0.0f, float v = 0.0f);

    // Adds a polygon face made of vertex indices (e.g., {0, 1, 2, 3} for a 4-sided quad).
    void addFace(const std::vector<int>& indices);

    // Automatically calculates smooth lighting normals for every vertex by computing the
    // cross-product of adjacent edge vectors for each face, accumulating them, and normalizing.
    void computeNormals();

    // -----------------------------------------------------------------------------------------
    // Procedural Shape Generators (Static Factory Methods)
    // -----------------------------------------------------------------------------------------

    // Creates an axis-aligned 3D cube where width, height, and depth all equal 'size'.
    static Mesh createCube(float size);

    // Creates a rectangular 3D box with custom width (w), height (h), and depth (d),
    // repeating the texture 'tileU' times horizontally and 'tileV' times vertically.
    static Mesh createTexturedBox(float w, float h, float d, float tileU = 1.0f, float tileV = 1.0f);

    // Creates a rectangular 3D box with explicit texture UV coordinate bounds [uMin..uMax, vMin..vMax].
    // Used for seamless wall segments where different parts of one large texture cover different boxes.
    static Mesh createTexturedBoxUV(float w, float h, float d, float uMin, float uMax, float vMin, float vMax);

    // Creates a 3D cylinder centered at the origin, specified by radius, height, and number of radial slices.
    static Mesh createCylinder(float radius, float height, int slices);

    // Creates a 3D torus (donut ring), used for the relic rotating rings and eye frames.
    // r: tube radius (thickness of the ring).
    // R: main radius (distance from center of donut hole to center of the tube).
    // rings: number of segments around the main ring circumference.
    // sides: number of segments around the tube cross-section.
    static Mesh createTorus(float r, float R, int rings, int sides);

    // Creates a curved 3D architectural archway between an inner radius and outer radius.
    // innerRadius: radius of the open curved doorway opening.
    // outerRadius: radius to the outer edge of the stone arch.
    // depth: thickness of the wall/arch along the Z axis.
    // segments: number of radial subdivisions forming the curve.
    static Mesh createArch(float innerRadius, float outerRadius, float depth, int segments);

    // Creates the triangular/rectangular stone wall fill (spandrels) above a curved archway.
    // Bridges the gap between the semicircular door opening and the rectangular outer wall boundary,
    // calculating smooth interpolated UVs so the wall texture flows continuously across it.
    static Mesh createArchSpandrels(float archRadius, float w, float h, float depth, float uMin, float uMax, float vMin, float vMax, int segments = 24);
};