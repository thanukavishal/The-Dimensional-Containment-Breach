#include "pch.h"
#include "Props.h"
#include <glut.h>
#include <cmath>
#include <stdlib.h>

// ==============================================================================================
// Constructor: Props::Props
// ==============================================================================================
Props::Props() {
}

// ==============================================================================================
// Method: Props::respawnParticle
// Purpose: Resets an expired flame particle back to the base of a torch sconce cup.
// Parameters:
//   p: Reference to the particle to re-initialize.
//   sconceIndex: Index of the torch sconce (0 to 7).
//   randomLife: If true (during initialization), staggers initial lifespans so all particles
//               do not spawn and die simultaneously in unison.
// ==============================================================================================
void Props::respawnParticle(FireParticle& p, int sconceIndex, bool randomLife) {
    // Pick a random angle around the full 360-degree circle (6.283185 radians)
    float angle = ((float)rand() / (float)RAND_MAX) * 6.283185f;
    // Square root of random creates uniform distribution within the circular radius of 0.05m
    float dist = sqrtf((float)rand() / (float)RAND_MAX) * 0.05f;

    // Local spawn position at the top rim of the torch cup
    p.x = cosf(angle) * dist;
    p.y = 0.20f + ((float)rand() / (float)RAND_MAX) * 0.03f;
    p.z = sinf(angle) * dist;

    // Upward velocity draft with slight turbulent horizontal drift
    p.vx = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.09f;
    p.vy = 0.65f + ((float)rand() / (float)RAND_MAX) * 0.60f; // Hot convection: 0.65 to 1.25 m/s upwards
    p.vz = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.09f;

    // Particle lifetime: each particle burns for between 0.35 and 0.70 seconds
    p.maxLife = 0.35f + ((float)rand() / (float)RAND_MAX) * 0.35f;
    // If starting up the simulation, randomize life so they are evenly distributed along their paths
    p.life = randomLife ? (((float)rand() / (float)RAND_MAX) * p.maxLife) : 0.0f;

    // Initial size of flame particle: 18cm to 28cm wide
    p.initialSize = 0.18f + ((float)rand() / (float)RAND_MAX) * 0.10f;
    p.size = p.initialSize;

    // Starting color: hot, bright incandescent white-yellow
    p.r = 1.0f;
    p.g = 0.95f;
    p.b = 0.40f;
    p.a = 0.95f;
}

// ==============================================================================================
// Method: Props::init
// Purpose: Procedurally builds all 3D meshes for props and initializes all fire particles.
// ==============================================================================================
void Props::init() {
    // Torch Sconce Meshes (for 8 wall sconces)
    sconceBracketMesh = Mesh::createTexturedBox(0.25f, 0.48f, 0.06f, 1.0f, 1.0f); // Wall mount plate
    sconceArmMesh = Mesh::createCylinder(0.04f, 0.42f, 12);                         // Angled supporting rod
    sconceCupMesh = Mesh::createCylinder(0.14f, 0.18f, 16);                         // Fuel brazier bowl

    // Fallen Column & Stone Rubble Field Meshes (UV-mapped with stone textures)
    fallenColSegmentMesh = Mesh::createCylinder(0.52f, 2.4f, 24);                   // Broken column shaft
    fallenCapitalMesh = Mesh::createTexturedBox(1.3f, 0.4f, 1.3f, 1.5f, 1.5f);     // Shattered capital block
    rubbleLargeMesh = Mesh::createTexturedBox(0.55f, 0.35f, 0.45f, 1.0f, 1.0f);     // Large stone fragment
    rubbleSmallMesh = Mesh::createTexturedBox(0.30f, 0.20f, 0.25f, 1.0f, 1.0f);     // Small stone fragment

    // Ancient Rune Pedestal Meshes (UV-mapped)
    pedestalBaseMesh = Mesh::createTexturedBox(1.25f, 0.35f, 1.25f, 1.5f, 1.5f);   // Stepped base
    pedestalPillarMesh = Mesh::createTexturedBox(0.85f, 1.30f, 0.85f, 1.0f, 2.0f); // Vertical column
    pedestalCapMesh = Mesh::createTexturedBox(1.10f, 0.20f, 1.10f, 1.2f, 1.2f);     // Top slab
    glyphPlateMesh = Mesh::createTexturedBox(0.68f, 0.08f, 0.68f, 1.0f, 1.0f);     // glowing rune slab

    // Skeleton Remains Meshes
    skullCraniumMesh = Mesh::createTexturedBox(0.24f, 0.28f, 0.26f, 1.0f, 1.0f);   // Cranium
    eyeSocketMesh = Mesh::createTexturedBox(0.06f, 0.06f, 0.05f, 1.0f, 1.0f);      // Eye socket hollow
    spineMesh = Mesh::createCylinder(0.05f, 0.70f, 10);                             // Vertebral column
    ribMesh = Mesh::createArch(0.16f, 0.22f, 0.04f, 12);                           // Curved rib cage bones
    limbBoneMesh = Mesh::createCylinder(0.045f, 0.55f, 10);                         // Femur / humerus limbs

    // Initialize fire particles for all 8 sconces with staggered initial lifespans
    for (int t = 0; t < NUM_TORCHES; ++t) {
        for (int p = 0; p < PARTICLES_PER_TORCH; ++p) {
            respawnParticle(particles[t][p], t, true);
        }
    }
}

// ==============================================================================================
// Method: Props::update
// Purpose: Physics simulation step for all active fire particles.
// Parameters:
//   dt: Delta time in seconds since the last frame.
// ==============================================================================================
void Props::update(float dt) {
    // Clamp delta time to prevent physics explosions if a frame takes too long to render
    if (dt <= 0.0f) dt = 0.016f;
    if (dt > 0.1f) dt = 0.1f;

    // Loop through all 8 torches
    for (int t = 0; t < NUM_TORCHES; ++t) {
        // Loop through all 24 particles in this torch
        for (int i = 0; i < PARTICLES_PER_TORCH; ++i) {
            FireParticle& p = particles[t][i];
            p.life += dt; // Advance elapsed time

            // If particle lived its full lifespan, respawn it at the base of the fire
            if (p.life >= p.maxLife) {
                respawnParticle(p, t, false);
            }
            else {
                // Normalized lifetime progress (0.0 = newborn flame, 1.0 = burned out tip)
                float progress = p.life / p.maxLife;

                // Move upward with turbulent convection drift (sin and cos add realistic air turbulence)
                p.x += p.vx * dt + 0.015f * sinf(p.life * 14.0f) * dt;
                p.y += p.vy * dt;
                p.z += p.vz * dt + 0.015f * cosf(p.life * 14.0f) * dt;

                // Scale down size as particle rises (shrinks by up to 72% at the tip)
                p.size = p.initialSize * (1.0f - 0.72f * progress);

                // --- Thermal Color Transitions ---
                if (progress < 0.22f) {
                    // Stage 1: Initial hot incandescent yellow
                    float f = progress / 0.22f; // Fraction of stage completed (0.0 to 1.0)
                    p.r = 1.0f;
                    p.g = 0.96f - 0.38f * f;   // Transition from 0.96 down to 0.58
                    p.b = 0.45f - 0.40f * f;   // Transition from 0.45 down to 0.05
                    p.a = 0.95f;               // Opaque bright flame
                }
                else if (progress < 0.65f) {
                    // Stage 2: Vibrant flame orange into rich ember red
                    float f = (progress - 0.22f) / 0.43f;
                    p.r = 1.0f - 0.12f * f;    // 1.00 down to 0.88
                    p.g = 0.58f - 0.42f * f;   // 0.58 down to 0.16
                    p.b = 0.05f - 0.05f * f;   // 0.05 down to 0.00
                    p.a = 0.95f - 0.35f * f;   // 0.95 down to 0.60
                }
                else {
                    // Stage 3: Deep ember red fading out to transparent smoke
                    float f = (progress - 0.65f) / 0.35f;
                    p.r = 0.88f - 0.58f * f;   // 0.88 down to 0.30
                    p.g = 0.16f - 0.12f * f;   // 0.16 down to 0.04
                    p.b = 0.0f;
                    p.a = 0.60f * (1.0f - f);  // Fade transparency from 0.60 down to 0.00 (invisible)
                }
            }
        }
    }
}

// ==============================================================================================
// Method: Props::setTextures
// Purpose: Stores active texture handles.
// ==============================================================================================
void Props::setTextures(const TempleTextures& tex) {
    textures = tex;
}

// ==============================================================================================
// Method: Props::getObstacles
// Purpose: Defines collision bounding boxes for props so the camera drone cannot fly through them.
// ==============================================================================================
std::vector<BoxCollider> Props::getObstacles() const {
    std::vector<BoxCollider> obs;

    // 1. Fallen Column & Rubble Field in Northwest Quadrant
    obs.push_back({ -7.4f, -3.6f, -5.8f, -2.0f });
    obs.push_back({ -5.8f, -2.0f, -6.8f, -3.2f });

    // 2. Ancient Rune Pedestal in Southeast Corner Alcove (at X=9.4m, Z=9.4m)
    obs.push_back({ 8.2f, 10.6f, 8.2f, 10.6f });

    // 3. Fallen Explorer Remains at Southwest Column (at X=-5.8m, Z=6.3m)
    obs.push_back({ -6.5f, -5.1f, 5.7f, 7.1f });

    return obs;
}

// ==============================================================================================
// Method: Props::renderMesh
// Purpose: Forwards draw calls to MeshRenderer.
// ==============================================================================================
void Props::renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe) {
    if (isWireframe) {
        renderer.renderWireframe(mesh);
    }
    else {
        renderer.renderSolid(mesh);
    }
}

// ==============================================================================================
// Method: Props::bindMaterialTexture
// Purpose: Helper to configure OpenGL material lighting and bind a texture.
// ==============================================================================================
void Props::bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB, bool isWireframe) {
    if (!isWireframe && texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    GLfloat matAmbient[] = { diffuseR * 0.35f, diffuseG * 0.35f, diffuseB * 0.35f, 1.0f };
    GLfloat matDiffuse[] = { diffuseR, diffuseG, diffuseB, 1.0f };
    GLfloat matSpecular[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    GLfloat matShininess = 10.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, matShininess);
}

// ==============================================================================================
// Material Preset Methods
// ==============================================================================================

// Forged antique iron finish for torch brackets, sconce arms, and skeleton eye sockets
void Props::setPillarMaterial(bool isWireframe) {
    bindMaterialTexture(textures.pillar, 0.82f, 0.80f, 0.78f, isWireframe);
}

// Weathered masonry texture for fallen capital and scattered stone rubble
void Props::setStoneRubbleMaterial(bool isWireframe) {
    bindMaterialTexture(textures.wall, 0.75f, 0.72f, 0.68f, isWireframe);
}

// Ornate carved stone texture for the rune pedestal
void Props::setCarvedPlinthMaterial(bool isWireframe) {
    bindMaterialTexture(textures.dais, 0.85f, 0.82f, 0.78f, isWireframe);
}

// Dark untextured metal for wall sconce ironwork
void Props::setDarkMetalMaterial() {
    glDisable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 0.16f, 0.16f, 0.16f, 1.0f };
    GLfloat diffuse[] = { 0.36f, 0.36f, 0.36f, 1.0f };
    GLfloat specular[] = { 0.55f, 0.55f, 0.55f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.0f);
}

// Self-illuminated pulsing cyan glow for the mysterious glyph plate
void Props::setEmissiveGlyphMaterial(float pulse) {
    glDisable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 0.05f, 0.30f, 0.40f, 1.0f };
    GLfloat diffuse[] = { 0.10f, 0.70f, 0.90f, 1.0f };
    GLfloat specular[] = { 0.80f, 0.95f, 1.0f, 1.0f };
    // The pulse parameter modulates the emission color, creating a breathing/throbbing light effect
    GLfloat emission[] = { 0.15f * pulse, 0.85f * pulse, 1.0f * pulse, 1.0f }; //cyan
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);
}

// Matte aged ivory bone material for the explorer skeleton
void Props::setBoneMaterial() {
    glDisable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 0.42f, 0.40f, 0.35f, 1.0f };
    GLfloat diffuse[] = { 0.82f, 0.80f, 0.73f, 1.0f };
    GLfloat specular[] = { 0.25f, 0.25f, 0.22f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 18.0f);
}

// Clears emission back to 0
void Props::resetEmission() {
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

// ==============================================================================================
// Method: Props::drawTorchParticles
// Purpose: Renders flame particles as camera facing billboard quads with additive alpha blending.
// ==============================================================================================
void Props::drawTorchParticles(int sconceIndex) {
    // Extract camera Right and Up basis vectors from current OpenGL modelview matrix
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);

    float rightX = mat[0], rightY = mat[4], rightZ = mat[8];
    float upX = mat[1], upY = mat[5], upZ = mat[9];

    // Normalize Right vector
    float rLen = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rLen > 1e-4f) { rightX /= rLen; rightY /= rLen; rightZ /= rLen; }

    // Normalize Up vector
    float uLen = sqrtf(upX * upX + upY * upY + upZ * upZ);
    if (uLen > 1e-4f) { upX /= uLen; upY /= uLen; upZ /= uLen; }

    // Configure OpenGL state machine for glowing additive transparency
    glDisable(GL_LIGHTING);            // Particles emit light; they should not receive shadows!
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending: Source Color * Alpha + Destination Color
    glDepthMask(GL_FALSE);             // Do not block objects behind transparent particles

    // Render each particle as a 4-vertex quadrilateral (quad)
    glBegin(GL_QUADS);
    for (int i = 0; i < PARTICLES_PER_TORCH; ++i) {
        const FireParticle& p = particles[sconceIndex][i];
        if (p.a <= 0.005f) continue; // Skip completely faded/invisible particles

        glColor4f(p.r, p.g, p.b, p.a); // Apply dynamic flame color & transparency
        float hs = p.size * 0.5f;     // Half-size offset

        // 4 Quad vertices offset along camera Right and Up directions:
        // Vertex 1: Bottom-Left
        glVertex3f(p.x - rightX * hs - upX * hs, p.y - rightY * hs - upY * hs, p.z - rightZ * hs - upZ * hs);
        // Vertex 2: Bottom-Right
        glVertex3f(p.x + rightX * hs - upX * hs, p.y + rightY * hs - upY * hs, p.z + rightZ * hs - upZ * hs);
        // Vertex 3: Top-Right
        glVertex3f(p.x + rightX * hs + upX * hs, p.y + rightY * hs + upY * hs, p.z + rightZ * hs + upZ * hs);
        // Vertex 4: Top-Left
        glVertex3f(p.x - rightX * hs + upX * hs, p.y - rightY * hs + upY * hs, p.z - rightZ * hs + upZ * hs);
    }
    glEnd();

    // Restore standard OpenGL rendering state
    glDepthMask(GL_TRUE);    // Re-enable depth buffer writing
    glDisable(GL_BLEND);     // Disable alpha blending
    glEnable(GL_LIGHTING);   // Re-enable lighting for subsequent scene geometry
}

// ==============================================================================================
// Method: Props::drawSingleSconce
// Purpose: Renders one complete torch assembly (Bracket, Arm, Cup, and animated Fire Particles).
// ==============================================================================================
void Props::drawSingleSconce(MeshRenderer& renderer, bool isWireframe, float timer, int sconceIndex) {
    setDarkMetalMaterial();

    // 1. Wall Bracket Plate
    glPushMatrix();
    renderMesh(renderer, sconceBracketMesh, isWireframe);
    glPopMatrix();

    // 2. Angled Torch Arm (tilted 35 degrees forward from wall)
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.15f);
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, sconceArmMesh, isWireframe);
    glPopMatrix();

    // 3. Sconce Brazier Cup
    glPushMatrix();
    glTranslatef(0.0f, 0.18f, 0.30f);
    renderMesh(renderer, sconceCupMesh, isWireframe);

    // 4. Dynamic Particle System: Billboard flame quads
    if (!isWireframe) {
        drawTorchParticles(sconceIndex);
    }

    glPopMatrix();
}

// ==============================================================================================
// Method: Props::drawTorchSconces
// Purpose: Positions and draws all 8 torch sconces flanking doorways across the 4 room walls.
// ==============================================================================================
void Props::drawTorchSconces(MeshRenderer& renderer, bool isWireframe, float timer) {
    const float wallDist = 12.82f; // Distance from room center to wall surface
    const float sconceY = 3.6f;    // Mounting height on wall (3.6m above floor)
    const float doorFlank = 2.6f;  // Lateral offset from doorway center (+/- 2.6m)

    // Position and rotation table for all 8 sconces (2 per wall)
    struct SconceSetup { float x, y, z, rotY; };
    SconceSetup sconces[8] = {
        // North Wall (-Z): facing south (rotY = 0)
        { -doorFlank, sconceY, -wallDist,   0.0f },
        {  doorFlank, sconceY, -wallDist,   0.0f },
        // South Wall (+Z): facing north (rotY = 180)
        { -doorFlank, sconceY,  wallDist, 180.0f },
        {  doorFlank, sconceY,  wallDist, 180.0f },
        // West Wall (-X): facing east (rotY = 90)
        { -wallDist, sconceY, -doorFlank,  90.0f },
        { -wallDist, sconceY,  doorFlank,  90.0f },
        // East Wall (+X): facing west (rotY = -90)
        {  wallDist, sconceY, -doorFlank, -90.0f },
        {  wallDist, sconceY,  doorFlank, -90.0f }
    };

    for (int i = 0; i < 8; ++i) {
        glPushMatrix();
        glTranslatef(sconces[i].x, sconces[i].y, sconces[i].z);
        glRotatef(sconces[i].rotY, 0.0f, 1.0f, 0.0f);
        drawSingleSconce(renderer, isWireframe, timer, i);
        glPopMatrix();
    }
}

// ==============================================================================================
// Method: Props::drawFallenColumnAndRubble
// Purpose: Renders the ruined, collapsed column and scattered debris field in northwest chamber.
// ==============================================================================================
void Props::drawFallenColumnAndRubble(MeshRenderer& renderer, bool isWireframe) {
    // 1. Ruined Fallen Column Segments (Textured with pillar texture)
    setPillarMaterial(isWireframe);

    // Segment 1: Main column body lying horizontally on the floor
    glPushMatrix();
    glTranslatef(-4.4f, 0.52f, -4.8f);
    glRotatef(68.0f, 0.0f, 1.0f, 0.0f);  // Angled diagonally across floor
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Lying flat horizontally
    renderMesh(renderer, fallenColSegmentMesh, isWireframe);
    glPopMatrix();

    // Segment 2: Fractured broken piece lying nearby
    glPushMatrix();
    glTranslatef(-6.0f, 0.52f, -3.6f);
    glRotatef(35.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, fallenColSegmentMesh, isWireframe);
    glPopMatrix();

    // 2. Shattered Capital & Stone Rubble Field (Textured with masonry wall texture)
    setStoneRubbleMaterial(isWireframe);

    // Shattered Capital Block tilted at an angle
    glPushMatrix();
    glTranslatef(-3.1f, 0.20f, -5.7f);
    glRotatef(22.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, fallenCapitalMesh, isWireframe);
    glPopMatrix();

    // Scattered Stone Rubble Blocks of varying sizes and rotations
    struct RubbleEntry { float x, y, z, rotY, rotX; bool large; };
    RubbleEntry rubble[] = {
        { -4.0f, 0.17f, -3.6f,  42.0f, 10.0f, true },
        { -5.2f, 0.18f, -5.9f, -30.0f,  0.0f, true },
        { -3.4f, 0.10f, -4.2f,  80.0f, 15.0f, false },
        { -4.8f, 0.10f, -2.9f, -15.0f,  5.0f, false },
        { -6.4f, 0.11f, -4.7f,  60.0f,  0.0f, false },
        { -2.8f, 0.10f, -3.8f, -45.0f,  8.0f, false },
        { -5.7f, 0.10f, -6.4f,  15.0f,  0.0f, false }
    };

    for (const auto& r : rubble) {
        glPushMatrix();
        glTranslatef(r.x, r.y, r.z);
        glRotatef(r.rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(r.rotX, 1.0f, 0.0f, 0.0f);
        if (r.large) {
            renderMesh(renderer, rubbleLargeMesh, isWireframe);
        }
        else {
            renderMesh(renderer, rubbleSmallMesh, isWireframe);
        }
        glPopMatrix();
    }
}

// ==============================================================================================
// Method: Props::drawRunePedestal
// Purpose: Renders the ancient inscribed pedestal in the southeast alcove with a pulsing glyph.
// ==============================================================================================
void Props::drawRunePedestal(MeshRenderer& renderer, bool isWireframe, float timer) {
    // Situated in Southeast corner alcove
    const float px = 9.4f;
    const float pz = 9.4f;

    setCarvedPlinthMaterial(isWireframe);

    glPushMatrix();
    glTranslatef(px, 0.0f, pz);
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f); // Angled diagonally facing sanctum center

    // 1. Stepped Base
    glPushMatrix();
    glTranslatef(0.0f, 0.175f, 0.0f);
    renderMesh(renderer, pedestalBaseMesh, isWireframe);
    glPopMatrix();

    // 2. Carved Pillar Core with Inscribed Rune Relief
    glPushMatrix();
    glTranslatef(0.0f, 0.35f + 0.65f, 0.0f);
    renderMesh(renderer, pedestalPillarMesh, isWireframe);
    glPopMatrix();

    // 3. Plinth Cap Tabletop
    glPushMatrix();
    glTranslatef(0.0f, 0.35f + 1.30f + 0.10f, 0.0f);
    renderMesh(renderer, pedestalCapMesh, isWireframe);
    glPopMatrix();

    // 4. Emissive Glowing Rune Glyph Plate
    // Calculate pulsing brightness using sine wave: oscillates between 0.30 and 1.00
    float pulse = 0.65f + 0.35f * sinf(timer * 2.8f);
    setEmissiveGlyphMaterial(pulse);

    glPushMatrix();
    glTranslatef(0.0f, 0.35f + 1.30f + 0.20f + 0.04f, 0.0f);
    renderMesh(renderer, glyphPlateMesh, isWireframe);
    glPopMatrix();

    // Reset emission so subsequent scene objects do not glow cyan
    resetEmission();

    glPopMatrix();
}

// ==============================================================================================
// Method: Props::drawFallenExplorer
// Purpose: Renders skeletal remains of an unfortunate explorer slumped against southwest column.
// ==============================================================================================
void Props::drawFallenExplorer(MeshRenderer& renderer, bool isWireframe) {
    // Resting against the base of Southwest Column (-COL_DIST, COL_DIST)
    const float cx = -6.8f;
    const float cz =  6.8f;

    setBoneMaterial(); // Apply ivory bone material

    glPushMatrix();
    glTranslatef(cx + 1.0f, 0.0f, cz - 0.5f);
    glRotatef(130.0f, 0.0f, 1.0f, 0.0f); // Leaning back against the column base

    // 1. Ribbed Chest Frame (Spine + 4 branching rib arches)
    glPushMatrix();
    glTranslatef(0.0f, 0.32f, 0.0f);
    glRotatef(-28.0f, 1.0f, 0.0f, 0.0f); // Slumped back posture
    renderMesh(renderer, spineMesh, isWireframe);

    // Rib segments branching laterally
    for (int r = 0; r < 4; ++r) {
        glPushMatrix();
        glTranslatef(0.0f, -0.18f + r * 0.10f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        renderMesh(renderer, ribMesh, isWireframe);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. Skull Cranium with Dark Eye Sockets
    glPushMatrix();
    glTranslatef(0.0f, 0.72f, -0.15f);
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
    renderMesh(renderer, skullCraniumMesh, isWireframe);

    // Indented dark eye sockets
    setDarkMetalMaterial();
    glPushMatrix();
    glTranslatef(-0.06f, 0.03f, 0.12f);
    renderMesh(renderer, eyeSocketMesh, isWireframe); // Left eye cavity
    glTranslatef(0.12f, 0.0f, 0.0f);
    renderMesh(renderer, eyeSocketMesh, isWireframe); // Right eye cavity
    glPopMatrix();
    setBoneMaterial();
    glPopMatrix();

    // 3. Angled Limb Bones (Legs stretched out on floor)
    glPushMatrix();
    glTranslatef(-0.16f, 0.08f, 0.28f);
    glRotatef(65.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
    renderMesh(renderer, limbBoneMesh, isWireframe); // Left leg
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.16f, 0.08f, 0.28f);
    glRotatef(65.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
    renderMesh(renderer, limbBoneMesh, isWireframe); // Right leg
    glPopMatrix();

    // Slumped arm bone resting at the side
    glPushMatrix();
    glTranslatef(-0.30f, 0.16f, 0.05f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    renderMesh(renderer, limbBoneMesh, isWireframe);
    glPopMatrix();

    glPopMatrix();
}

// ==============================================================================================
// Method: Props::drawAll
// Purpose: Master drawing method that renders all environmental props.
// ==============================================================================================
void Props::drawAll(MeshRenderer& renderer, bool isWireframe, float timer) {
    drawTorchSconces(renderer, isWireframe, timer);
    drawFallenColumnAndRubble(renderer, isWireframe);
    drawRunePedestal(renderer, isWireframe, timer);
    drawFallenExplorer(renderer, isWireframe);
}
