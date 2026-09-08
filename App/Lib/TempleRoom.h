#pragma once
#include "Mesh.h"
#include "MeshRenderer.h"
#include <vector>
#include <glut.h>

// ==============================================================================================
// Struct: BoxCollider
// Purpose: Represents an Axis-Aligned Bounding Box (AABB) obstacle on the 2D floor plane (X-Z).
// ==============================================================================================
struct BoxCollider {
    float minX, maxX; // Minimum and maximum X extents of the blocking box
    float minZ, maxZ; // Minimum and maximum Z extents of the blocking box
};

// ==============================================================================================
// Struct: TempleTextures
// Purpose: Stores OpenGL texture handle IDs for all temple surface materials.
// ==============================================================================================
struct TempleTextures {
    GLuint floor = 0;   // Texture ID for stone tiled floor
    GLuint wall = 0;    // Texture ID for carved ancient masonry walls
    GLuint pillar = 0;  // Texture ID for fluted classical stone columns
    GLuint dais = 0;    // Texture ID for ornate stepped altar dais
    GLuint door = 0;    // Texture ID for iron/stone portcullis trapdoor slab
    GLuint relic = 0;   // Texrure ID for center relic
};

// ==============================================================================================
// Class: TempleRoom
// Purpose: Builds, textures, and renders the entire temple environment and handles collision.
// ==============================================================================================
class TempleRoom {
public:
    TempleRoom();

    void init();

    // Assigns loaded OpenGL texture IDs to the room materials.
    void setTextures(const TempleTextures& textures);

    // Returns the loaded OpenGL texture IDs for the room materials.
    const TempleTextures& getTextures() const { return textures; }

    // Draws all structural meshes (floor, walls, ceiling, beams, dais, columns).
    void draw(MeshRenderer& renderer, bool isWireframe);

    // Checks whether a target (X, Z) position is valid for the camera drone to move to,
    // ensuring the drone does not pass through outer walls, columns, or the central dais.
    // targetX, targetZ: Proposed new position of the drone.
    // droneRadius: Personal collision radius around the drone camera.
    // Returns: true if valid to move, false if a collision occurred.
    bool isPositionValid(float targetX, float targetZ, float droneRadius = 0.55f) const;

    // Sets vertical height (Y) of the entrance portcullis slab (allows lowering/raising trapdoors).
    void setPortcullisY(float y) { portcullisY = y; }
    // Gets vertical height of the entrance portcullis slab.
    float getPortcullisY() const { return portcullisY; }

    // Adds additional obstacle colliders (such as props, fallen pillars, and pedestals).
    void addObstacles(const std::vector<BoxCollider>& newObstacles);

    // Returns the list of all active box colliders in the room.
    const std::vector<BoxCollider>& getObstacles() const { return obstacles; }

    // -----------------------------------------------------------------------------------------
    // Architectural Dimensional Constants (in 3D world meters)
    // -----------------------------------------------------------------------------------------
    static constexpr float ROOM_SIZE = 26.0f;       // Total square room width & length (26m x 26m)
    static constexpr float WALL_HEIGHT = 8.0f;     // Total vertical room wall height (8m)
    static constexpr float DOOR_WIDTH = 4.0f;      // Width of the arched entrance doorways (4m)
    static constexpr float DOOR_HEIGHT = 5.0f;     // Height to top of arch doorway lintel (5m)
    static constexpr float WALL_SEG_WIDTH = (ROOM_SIZE - DOOR_WIDTH) / 2.0f; // Width of flanking wall sections (11m each)
    static constexpr float COLUMN_HEIGHT = 6.2f;   // Height of the vertical column shafts (6.2m)
    static constexpr float COL_DIST = 6.8f;        // Distance of each column from the room center (+/- 6.8m)

private:
    // Draws a mesh using solid or wireframe mode.
    void renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe);

    // Sets OpenGL material properties and binds the active 2D texture map.
    void bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB, bool isWireframe);

    // Draws one complete modular wall side featuring an arched doorway, spandrels, and lintel.
    void drawWallWithArchway(MeshRenderer& renderer, bool isWireframe);

    // Draws a single classical column consisting of base, cylindrical shaft, and capital.
    void drawSingleColumn(MeshRenderer& renderer, float x, float z, bool isWireframe);

    // Draws the ceiling slab, overhead architrave crossbeams, and the diagonal collapsed beam.
    void drawCeilingAndBeams(MeshRenderer& renderer, bool isWireframe);

    // Initializes the default list of Axis-Aligned Bounding Box obstacles.
    void buildCollisionMap();

    TempleTextures textures; // Loaded OpenGL texture handles

    // --- Room Geometry Meshes ---
    Mesh floorMesh;             // Ground floor slab
    Mesh ceilingMesh;           // Overhead ceiling slab
    Mesh wallLeftMesh;          // Wall section to the left of the doorway
    Mesh wallRightMesh;         // Wall section to the right of the doorway
    Mesh archSpandrelsMesh;     // Seamless stone wall infill above the curved arch opening
    Mesh archMoldingMesh;       // Decorative extruded carved molding framing the arch
    Mesh lintelMesh;            // Continuous stone beam bridging above the doorway to the ceiling
    Mesh portcullisMesh;        // Heavy stone/iron security portcullis slab
    Mesh crossbeamMesh;         // Long structural architrave crossbeams supporting the ceiling
    Mesh brokenCrossbeamMesh;   // Fractured collapsed beam lying diagonally on the floor
    Mesh daisBottomMesh;        // Lower tier of the stepped ceremonial dais platform
    Mesh daisTopMesh;           // Upper tier of the stepped ceremonial dais platform
    Mesh altarPedestalMesh;     // Central elevated altar plinth directly beneath the relic

    // --- Column Component Meshes ---
    Mesh columnBaseMesh;        // Square stone plinth for column base and capital
    Mesh columnShaftMesh;       // Fluted cylindrical pillar shaft

    float portcullisY;                  // Vertical position of the portcullis trapdoor
    std::vector<BoxCollider> obstacles; // List of all colliders blocking the drone
};
