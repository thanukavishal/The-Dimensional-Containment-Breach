// ==============================================================================================
// File: Relic.cpp
// Description: Implementation of the ancient floating multi-ring relic, its dynamic rotation,
//              sinusoidal floating physics, bronze materials, and glowing energy core.
// ==============================================================================================

#include "pch.h"
#include "Relic.h"
#include <glut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==============================================================================================
// Constructor: Relic::Relic
// Purpose: Sets initial state for all animation variables.
// ==============================================================================================
Relic::Relic()
    : rotRing1(0.0f)       // Initial rotation of outer ring (degrees)
    , rotRing2(0.0f)       // Initial rotation of middle ring (degrees)
    , rotRing3(0.0f)       // Initial rotation of inner ring (degrees)
    , ringSpeedMult(1.0f)  // Default rotation speed multiplier
    , floatTimer(0.0f)     // Levitation timer starts at 0
    , relicBaseY(2.7f)     // Base floating altitude centered directly above the altar dais
{
}

// ==============================================================================================
// Method: Relic::init
// Purpose: Constructs the procedural torus (donut) meshes for the 3 nested rings.
// Parameters to Mesh::createTorus(tubeRadius, ringRadius, segmentsAroundRing, segmentsAroundTube):
//   - Outer Ring: tube radius = 0.07m, main ring radius = 1.50m (largest, outermost ring).
//   - Middle Ring: tube radius = 0.06m, main ring radius = 1.10m (fits neatly inside outer ring).
//   - Inner Ring: tube radius = 0.05m, main ring radius = 0.72m (innermost ring surrounding core).
// ==============================================================================================
void Relic::init() {
    outerTorusMesh = Mesh::createTorus(0.07f, 1.50f, 40, 16);
    midTorusMesh = Mesh::createTorus(0.06f, 1.10f, 32, 16);
    innerTorusMesh = Mesh::createTorus(0.05f, 0.72f, 28, 16);
}

// ==============================================================================================
// Method: Relic::update
// Purpose: Advances the animation state by updating angles and time.
// Parameters:
//   speedMult: Multiplier that scales rotation speed (allows speeding up or slowing down).
// ==============================================================================================
void Relic::update(float speedMult) {
    // Increment rotation angles each frame
    rotRing1 += 0.8f * speedMult; // Outer ring turns at 0.8 deg/frame
    rotRing2 += 1.4f * speedMult; // Middle ring turns faster at 1.4 deg/frame
    rotRing3 += 1.0f * speedMult; // Inner ring turns at 1.0 deg/frame

    // Keep angles within the [0, 360) degree range
    if (rotRing1 >= 360.0f) rotRing1 -= 360.0f;
    if (rotRing2 >= 360.0f) rotRing2 -= 360.0f;
    if (rotRing3 >= 360.0f) rotRing3 -= 360.0f;

    // Advance levitation timer by a constant step each frame
    floatTimer += 0.035f;
}

// ==============================================================================================
// Method: Relic::renderMesh
// Purpose: Helper that forwards the draw call to MeshRenderer based on wireframe toggle.
// ==============================================================================================
void Relic::renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe) {
    if (isWireframe) {
        renderer.renderWireframe(mesh);
    }
    else {
        renderer.renderSolid(mesh);
    }
}

// ==============================================================================================
// Method: Relic::setBronzeRelicMaterial
// Purpose: Configures OpenGL lighting material properties for polished antique bronze alloy.
// ==============================================================================================
void Relic::setBronzeRelicMaterial() {
    glDisable(GL_TEXTURE_2D); // Bronze rings use pure procedural Phong lighting, not bitmap textures
    GLfloat bronzeAmbient[] = { 0.28f, 0.20f, 0.08f, 1.0f };
    GLfloat bronzeDiffuse[] = { 0.85f, 0.68f, 0.25f, 1.0f };
    GLfloat bronzeSpecular[] = { 0.95f, 0.85f, 0.60f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, bronzeAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, bronzeDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, bronzeSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 64.0f); // High shininess for reflective metal
}

// ==============================================================================================
// Method: Relic::setCoreEnergyMaterial
// Purpose: Configures material for an ethereal glowing blue-cyan power core.
// ==============================================================================================
void Relic::setCoreEnergyMaterial() {
    glDisable(GL_TEXTURE_2D);
    GLfloat coreAmbient[] = { 0.1f, 0.5f, 0.6f, 1.0f };
    GLfloat coreDiffuse[] = { 0.2f, 0.8f, 1.0f, 1.0f };
    GLfloat coreSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat coreEmission[] = { 0.25f, 0.75f, 0.95f, 1.0f }; // Self-illuminated cyan glow
    glMaterialfv(GL_FRONT, GL_AMBIENT, coreAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, coreDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, coreSpecular);
    glMaterialfv(GL_FRONT, GL_EMISSION, coreEmission);
    glMaterialf(GL_FRONT, GL_SHININESS, 96.0f);
}

// ==============================================================================================
// Method: Relic::resetEmission
// Purpose: Resets material emission back to 0 (black).
// ==============================================================================================
void Relic::resetEmission() {
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

// ==============================================================================================
// Method: Relic::draw
// Purpose: Renders the entire animated relic system with nested rotating rings and core.
// ==============================================================================================
void Relic::draw(MeshRenderer& renderer, bool isWireframe) {
    // Calculate vertical bobbing motion using trigonometry:
    // sin(floatTimer) produces smooth oscillation between -1.0 and +1.0.
    // Multiplying by 0.20f means the relic bobs up and down by +/- 20 centimeters.
    float floatingY = relicBaseY + sinf(floatTimer) * 0.20f;

    // Save current world matrix
    glPushMatrix();

    // Move the entire relic assembly up to its floating altitude
    glTranslatef(0.0f, floatingY, 0.0f);

    // Apply reflective bronze metal material for all 3 rings
    setBronzeRelicMaterial();

    // --- 1. Outer Torus Ring ---
    glPushMatrix();
    // Rotate outer ring around the local X axis (pitch / front-back flip)
    glRotatef(rotRing1, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, outerTorusMesh, isWireframe);
    glPopMatrix(); // Restore matrix to unrotated floating center

    // --- 2. Middle Torus Ring ---
    glPushMatrix();
    // Rotate middle ring around the local Y axis (yaw / horizontal spin)
    glRotatef(rotRing2, 0.0f, 1.0f, 0.0f);
    renderMesh(renderer, midTorusMesh, isWireframe);
    glPopMatrix(); // Restore matrix to unrotated floating center

    // --- 3. Inner Torus Ring ---
    glPushMatrix();
    // Rotate inner ring around a diagonal 45-degree axis (X + Z vector: 0.707, 0, 0.707)
    glRotatef(rotRing3, 0.707f, 0.0f, 0.707f);
    renderMesh(renderer, innerTorusMesh, isWireframe);
    glPopMatrix(); // Restore matrix to unrotated floating center

    // --- 4. Central Glowing Energy Core ---
    setCoreEnergyMaterial(); // Switch material to glowing cyan emission
    glPushMatrix();
    // Use GLU (OpenGL Utility Library) Quadrics to render a mathematically smooth 3D sphere
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH); // Request smooth vertex normals across sphere
    gluSphere(quad, 0.38, 32, 32);       // Radius = 0.38m, 32 longitudinal slices, 32 latitudinal stacks
    gluDeleteQuadric(quad);              // Free GLU quadric memory
    glPopMatrix();

    // Clean up emission state so subsequent objects do not glow
    resetEmission();

    // Restore coordinate system back to the world origin
    glPopMatrix();
}
