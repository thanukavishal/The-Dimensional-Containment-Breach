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

void Relic::init() {
    outerTorusMesh = Mesh::createTorus(0.07f, 1.50f, 40, 16);
    midTorusMesh = Mesh::createTorus(0.06f, 1.10f, 32, 16);
    innerTorusMesh = Mesh::createTorus(0.05f, 0.72f, 28, 16);

    // 3D Carved Stone Idol Core Meshes
    idolHeadMesh = Mesh::createTexturedBox(0.68f, 0.88f, 0.55f, 1.0f, 1.0f);
    idolCrownMesh = Mesh::createTexturedBox(0.58f, 0.14f, 0.48f, 1.0f, 1.0f);
    idolBrowMesh = Mesh::createTexturedBox(0.62f, 0.12f, 0.10f, 1.0f, 1.0f);
    idolNoseMesh = Mesh::createTexturedBox(0.14f, 0.24f, 0.12f, 1.0f, 1.0f);
    idolEyeSocketMesh = Mesh::createTorus(0.025f, 0.09f, 16, 12);
    idolMouthFrameMesh = Mesh::createTexturedBox(0.42f, 0.16f, 0.08f, 1.0f, 1.0f);
    idolToothMesh = Mesh::createTexturedBox(0.06f, 0.06f, 0.05f, 1.0f, 1.0f);
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
// Method: Relic::drawStoneIdolHead
// Purpose: Renders the carved 3D stone monolith face
// ==============================================================================================
void Relic::drawStoneIdolHead(MeshRenderer& renderer, bool isWireframe, GLuint stoneTex) {
    if (!isWireframe && stoneTex != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, stoneTex);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    // Stone material properties
    GLfloat matAmb[] = { 0.30f, 0.28f, 0.25f, 1.0f };
    GLfloat matDif[] = { 0.75f, 0.72f, 0.68f, 1.0f };
    GLfloat matSpc[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpc);
    glMaterialf(GL_FRONT, GL_SHININESS, 12.0f);

    // Main Cranium Head Block
    renderMesh(renderer, idolHeadMesh, isWireframe);

    // Stepped Crown
    glPushMatrix();
    glTranslatef(0.0f, 0.50f, 0.0f);
    renderMesh(renderer, idolCrownMesh, isWireframe);
    glPopMatrix();

    // Heavy Protruding Brow
    glPushMatrix();
    glTranslatef(0.0f, 0.20f, 0.28f);
    renderMesh(renderer, idolBrowMesh, isWireframe);
    glPopMatrix();

    // Central Nose Wedge
    glPushMatrix();
    glTranslatef(0.0f, 0.02f, 0.30f);
    renderMesh(renderer, idolNoseMesh, isWireframe);
    glPopMatrix();

    // Raised Parametric Torus Eye Frames with Glowing Pupils
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 0.18f, 0.12f, 0.28f);
        renderMesh(renderer, idolEyeSocketMesh, isWireframe);

        // Glowing energy pupils nested inside sockets
        setCoreEnergyMaterial();
        glPushMatrix();
        GLUquadric* q = gluNewQuadric();
        gluSphere(q, 0.045, 12, 12);
        gluDeleteQuadric(q);
        glPopMatrix();
        resetEmission();

        glPopMatrix();
    }

    // Carved Mouth Frame & Teeth
    if (!isWireframe && stoneTex != 0) glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(0.0f, -0.22f, 0.28f);
    renderMesh(renderer, idolMouthFrameMesh, isWireframe);

    // --- Carved Teeth Material Customization ---
    glDisable(GL_TEXTURE_2D); // Disable stone texture to show pure tooth color

    GLfloat toothAmbient[] = { 0.35f, 0.25f, 0.05f, 1.0f }; // Shadow tint
    GLfloat toothDiffuse[] = { 0.95f, 0.75f, 0.15f, 1.0f }; // Main surface color
    GLfloat toothSpecular[] = { 0.90f, 0.80f, 0.40f, 1.0f }; // Shiny highlight
    GLfloat toothShininess = 64.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, toothAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, toothDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, toothSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, toothShininess);

    // Row of carved stone teeth
    for (int t = -1; t <= 1; ++t) {
        glPushMatrix();
        glTranslatef(t * 0.09f, 0.01f, 0.04f);
        renderMesh(renderer, idolToothMesh, isWireframe);
        glPopMatrix();
    }
    glPopMatrix();
}

// ==============================================================================================
// Method: Relic::draw
// Purpose: Draw complete relic with its components and rings
// ==============================================================================================
void Relic::draw(MeshRenderer& renderer, bool isWireframe, GLuint stoneTex) {
    float floatingY = relicBaseY + sinf(floatTimer) * 0.20f;

    glPushMatrix();
    glTranslatef(0.0f, floatingY, 0.0f);

    setBronzeRelicMaterial();

    // Outer Torus Ring 
    glPushMatrix();
    glRotatef(rotRing1, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, outerTorusMesh, isWireframe);
    glPopMatrix();

    // Middle Torus Ring 
    //glPushMatrix();
    //glRotatef(rotRing2, 0.0f, 1.0f, 0.0f);
    //renderMesh(renderer, midTorusMesh, isWireframe);
    //glPopMatrix();

    // Inner Torus Ring 
    glPushMatrix();
    glRotatef(rotRing3, 0.707f, 0.0f, 0.707f);
    renderMesh(renderer, innerTorusMesh, isWireframe);
    glPopMatrix();

    // Central 3D Stone Idol 
    drawStoneIdolHead(renderer, isWireframe, stoneTex);

    glPopMatrix();

}