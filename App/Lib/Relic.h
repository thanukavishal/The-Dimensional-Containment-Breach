#pragma once
#include <windows.h>
#include <stdlib.h>
#include <glut.h>
#include "Mesh.h"
#include "MeshRenderer.h"

class Relic {
public:
    // Constructor: Initializes rotation angles, timers, and default vertical altitude.
    Relic();

    // Creates the procedural torus ring meshes with appropriate radii and segment counts.
    void init();

    // Advances the animation each frame: rotates each ring and updates the floating sine timer.
    // speedMult: Optional multiplier to speed up or slow down the animation.
    void update(float speedMult = 1.0f);

    // Draws the levitating relic, its materials, rings, and glowing energy core using OpenGL.
    void draw(MeshRenderer& renderer, bool isWireframe, GLuint stoneTex = 0);

    // Returns the current floating timer value (also used by Main.cpp to sync torch flickering).
    float getFloatTimer() const { return floatTimer; }

    // Sets the base vertical height (Y coordinate) around which the relic levitates.
    void setBaseY(float y) { relicBaseY = y; }

    // Gets the base vertical height of the relic.
    float getBaseY() const { return relicBaseY; }

private:
    // Helper function that chooses between solid and wireframe mesh rendering.
    void renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe);

    // Configures OpenGL material parameters for antique reflective bronze alloy.
    void setBronzeRelicMaterial();

    // Configures material for an ethereal glowing blue-cyan power core.
    void setCoreEnergyMaterial();

    // Resets material emission back to black so other objects do not glow unintentionally.
    void resetEmission();

    // Helper function to render center relic stone idol.
    void drawStoneIdolHead(MeshRenderer& renderer, bool isWireframe, GLuint stoneTex);

    // 3D procedural meshes for the concentric rings
    Mesh outerTorusMesh; // Largest outer rotating ring
    Mesh midTorusMesh;   // Middle rotating ring
    Mesh innerTorusMesh; // Smallest inner rotating ring

    // 3D procedural meshes for the central stone idol head
    Mesh idolHeadMesh;
    Mesh idolCrownMesh;
    Mesh idolBrowMesh;
    Mesh idolNoseMesh;
    Mesh idolEyeSocketMesh;
    Mesh idolMouthFrameMesh;
    Mesh idolToothMesh;

    // Rotation angles (in degrees, 0 to 360) for each ring
    float rotRing1;      // Outer ring angle (rotates about the X axis)
    float rotRing2;      // Middle ring angle (rotates about the Y axis)
    float rotRing3;      // Inner ring angle (rotates about a diagonal X-Z axis)

    float ringSpeedMult; // Speed modifier for ring rotation
    float floatTimer;    // Accumulated time used as the input to the levitation sine wave: sin(t)
    float relicBaseY;    // Resting vertical altitude of the relic above the altar dais
};
