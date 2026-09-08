#pragma once
// ==============================================================================================
// File: Props.h
// Description: Defines environmental story-telling props, decorative ruined architectural pieces,
//              and a real-time CPU particle system simulating flickering torch fire.
// ==============================================================================================

#include "Mesh.h"
#include "MeshRenderer.h"
#include "TempleRoom.h"
#include <vector>

// ==============================================================================================
// Struct: FireParticle
// Purpose: Stores state for a single animated flame/spark particle.
// ==============================================================================================
struct FireParticle {
    float x, y, z;       // Current 3D position relative to the torch cup center
    float vx, vy, vz;    // 3D velocity (speed and direction of motion per second)
    float life;          // Elapsed lifetime in seconds
    float maxLife;       // Total lifespan before disappearing and respawning
    float initialSize;   // Starting size when born at the base of the fire
    float size;          // Current size (shrinks as it rises)
    float r, g, b, a;    // Color (Red, Green, Blue) and Alpha (Transparency, 1.0=opaque, 0.0=invisible)
};

// ==============================================================================================
// Class: Props
// Purpose: Manages generation, physics updates, obstacle registration, and rendering for all props.
// ==============================================================================================
class Props {
public:
    // Constructor
    Props();

    // Builds 3D meshes for sconces, rubble, pedestal, and skeleton, and spawns initial particles.
    void init();

    // Physics step: advances particle lifespans, applies velocity draft and convection turbulence,
    // interpolates flame colors, and respawns dead particles.
    // dt: Delta time in seconds since the last frame (defaults to ~16ms = 60 FPS).
    void update(float dt = 0.016f);

    // Passes loaded texture IDs from the main application.
    void setTextures(const TempleTextures& textures);

    // Draws all props (sconces, fire particles, rubble field, rune pedestal, explorer remains).
    void drawAll(MeshRenderer& renderer, bool isWireframe, float timer = 0.0f);

    // Returns a list of BoxCollider bounding boxes for props to be added to TempleRoom collision map.
    std::vector<BoxCollider> getObstacles() const;

    // Constants
    static const int NUM_TORCHES = 8;          // 8 wall sconces placed symmetrically around chamber
    static const int PARTICLES_PER_TORCH = 24;  // 24 simultaneous active flame particles per sconce

private:
    // Helper to render mesh in solid or wireframe
    void renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe);

    // Binds a texture and sets standard matte stone lighting parameters
    void bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB, bool isWireframe);

    // --- Shaders & Material Presets ---
    void setDarkMetalMaterial();                       // Forged iron finish for sconces and eye sockets
    void setStoneRubbleMaterial(bool isWireframe);      // Weathered stone texture for fallen capital & rubble
    void setPillarMaterial(bool isWireframe);           // Column stone texture for fallen column segments
    void setCarvedPlinthMaterial(bool isWireframe);     // Dais stone texture for the rune pedestal
    void setEmissiveGlyphMaterial(float pulse);         // Self-illuminated cyan glow for rune plate
    void setBoneMaterial();                            // Matte aged ivory material for skeleton remains
    void resetEmission();                              // Clears emission back to black

    // --- Particle Routines ---
    // Resets a dead particle back to the base of the torch with randomized angle, velocity, and lifespan.
    void respawnParticle(FireParticle& p, int sconceIndex, bool randomLife = false);

    // Renders the active fire particles for a sconce as camera-facing billboard quads with additive blending.
    void drawTorchParticles(int sconceIndex);

    // --- Prop Rendering Routines ---
    void drawTorchSconces(MeshRenderer& renderer, bool isWireframe, float timer);
    void drawSingleSconce(MeshRenderer& renderer, bool isWireframe, float timer, int sconceIndex);
    void drawFallenColumnAndRubble(MeshRenderer& renderer, bool isWireframe);
    void drawRunePedestal(MeshRenderer& renderer, bool isWireframe, float timer);
    void drawFallenExplorer(MeshRenderer& renderer, bool isWireframe);

    TempleTextures textures; // Active texture IDs
    FireParticle particles[NUM_TORCHES][PARTICLES_PER_TORCH]; // 2D array storing all 192 flame particles

    // Sconce Meshes
    Mesh sconceBracketMesh; // Flat iron wall mounting plate
    Mesh sconceArmMesh;     // Angled iron rod supporting the cup
    Mesh sconceCupMesh;     // Cylindrical brazier cup holding fuel and fire

    // Rubble & Fallen Column Meshes
    Mesh fallenColSegmentMesh; // Cylindrical section of shattered column
    Mesh fallenCapitalMesh;    // Broken decorative column top block
    Mesh rubbleLargeMesh;      // Large fractured stone boulder
    Mesh rubbleSmallMesh;      // Small scattered stone piece

    // Rune Pedestal Meshes
    Mesh pedestalBaseMesh;     // Stepped stone base
    Mesh pedestalPillarMesh;   // Inscribed vertical plinth shaft
    Mesh pedestalCapMesh;      // Stone cap tabletop
    Mesh glyphPlateMesh;       // Floating rune inscription plate

    // Explorer Skeleton Meshes
    Mesh skullCraniumMesh;     // Box approximation of human skull
    Mesh eyeSocketMesh;        // Indented dark eye cavities
    Mesh spineMesh;            // Vertebral column cylinder
    Mesh ribMesh;              // Curved arch rib bones
    Mesh limbBoneMesh;         // Arm and leg bone cylinders
};
