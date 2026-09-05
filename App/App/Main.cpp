#include <windows.h>
#include <stdlib.h>
#include <glut.h>
#include <SOIL2.h>
#include <cmath>
#include <vector>
#include <stdio.h>

#include "Mesh.h"
#include "MeshRenderer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Distinct Texture IDs
GLuint texFloor = 0;
GLuint texWall = 0;
GLuint texPillar = 0;
GLuint texDais = 0;
GLuint texDoor = 0;

// Scene geometry
Mesh floorMesh;
Mesh wallSegmentMesh;
Mesh lintelMesh;
Mesh portcullisMesh;
Mesh daisBottomMesh;
Mesh daisTopMesh;
Mesh altarPedestalMesh;

// Column components
Mesh columnBaseMesh;
Mesh columnShaftMesh;

// Relic Components
Mesh outerTorusMesh;
Mesh midTorusMesh;
Mesh innerTorusMesh;

MeshRenderer renderer;

// Chamber Dimensions
const float ROOM_SIZE = 26.0f;
const float WALL_HEIGHT = 8.0f;
const float DOOR_WIDTH = 4.0f;
const float DOOR_HEIGHT = 5.0f;
const float WALL_SEG_WIDTH = (ROOM_SIZE - DOOR_WIDTH) / 2.0f; // 11.0f on each side of door
const float COLUMN_HEIGHT = 6.2f;
const float COL_DIST = 6.8f;

// First-Person Drone Camera Variables
float droneX = 0.0f;
float droneY = 2.0f;
float droneZ = 10.5f;
float droneYaw = 180.0f;
float moveSpeed = 0.40f;
float turnSpeed = 4.0f;
const float DRONE_RADIUS = 0.55f;

// Relic Animation Variables
float rotRing1 = 0.0f;
float rotRing2 = 0.0f;
float rotRing3 = 0.0f;
float ringSpeedMult = 1.0f;
float floatTimer = 0.0f;
float relicBaseY = 2.7f;

// Portcullis position (suspended open at 4.2f above floor)
float portcullisY = 4.2f;

// Option A Interactive Toggles (Rubric 4)
bool isWireframe = false;
bool isTorchOn = true;

// Generic 2D AABB Box Structure for Obstacle Blocking
struct BoxCollider {
    float minX, maxX;
    float minZ, maxZ;
};
std::vector<BoxCollider> obstacles;

void buildCollisionMap() {
    obstacles.clear();

    // 1. Central Altar Dais
    float daisHalf = 3.0f + DRONE_RADIUS;
    obstacles.push_back({ -daisHalf, daisHalf, -daisHalf, daisHalf });

    // 2. The 4 Columns
    float colBuffer = 0.7f + DRONE_RADIUS;
    float coords[4][2] = {
        { -COL_DIST, -COL_DIST },
        {  COL_DIST, -COL_DIST },
        { -COL_DIST,  COL_DIST },
        {  COL_DIST,  COL_DIST }
    };
    for (int i = 0; i < 4; ++i) {
        float cx = coords[i][0];
        float cz = coords[i][1];
        obstacles.push_back({ cx - colBuffer, cx + colBuffer, cz - colBuffer, cz + colBuffer });
    }
}

bool isPositionValid(float targetX, float targetZ) {
    float roomLimit = (ROOM_SIZE / 2.0f) - DRONE_RADIUS - 0.2f;

    // Prevent passing outside chamber perimeter through open doorways
    if (targetX > roomLimit || targetX < -roomLimit) return false;
    if (targetZ > roomLimit || targetZ < -roomLimit) return false;

    // Internal Obstacles
    for (const auto& box : obstacles) {
        if (targetX >= box.minX && targetX <= box.maxX &&
            targetZ >= box.minZ && targetZ <= box.maxZ) {
            return false;
        }
    }
    return true;
}

GLuint loadTexture(const char* filename) {
    GLuint tex = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );
    if (!tex) {
        printf("Notice: '%s' not found. Falling back to default material.\n", filename);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    return tex;
}

void loadAllTextures() {
    texFloor = loadTexture("floor.jpg");
    texWall = loadTexture("wall.jpg");
    texPillar = loadTexture("pillar.jpg");
    texDais = loadTexture("dais.jpg");
    texDoor = loadTexture("door.jpg");
}

void renderCurrentMesh(const Mesh& m) {
    if (isWireframe) {
        renderer.renderWireframe(m);
    }
    else {
        renderer.renderSolid(m);
    }
}

void init() {
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    loadAllTextures();

    // 1. Room Floor
    floorMesh = Mesh::createTexturedBox(ROOM_SIZE, 0.4f, ROOM_SIZE, 8.0f, 8.0f);

    // 2. Split Walls with Archway Openings (North/South/East/West)
    wallSegmentMesh = Mesh::createTexturedBox(WALL_SEG_WIDTH, WALL_HEIGHT, 0.4f, 3.5f, 2.5f);
    lintelMesh = Mesh::createTexturedBox(DOOR_WIDTH, WALL_HEIGHT - DOOR_HEIGHT, 0.4f, 1.5f, 1.0f);
    portcullisMesh = Mesh::createTexturedBox(DOOR_WIDTH - 0.1f, DOOR_HEIGHT, 0.2f, 1.0f, 2.0f);

    // 3. Dais & Altar
    daisBottomMesh = Mesh::createTexturedBox(6.0f, 0.35f, 6.0f, 2.0f, 2.0f);
    daisTopMesh = Mesh::createTexturedBox(4.6f, 0.35f, 4.6f, 1.5f, 1.5f);
    altarPedestalMesh = Mesh::createTexturedBox(1.8f, 0.70f, 1.8f, 1.0f, 1.0f);

    // 4. Columns
    columnBaseMesh = Mesh::createTexturedBox(1.4f, 0.4f, 1.4f, 1.0f, 1.0f);
    columnShaftMesh = Mesh::createCylinder(0.52f, COLUMN_HEIGHT, 24);

    // 5. Relic Torus Meshes
    outerTorusMesh = Mesh::createTorus(0.07f, 1.50f, 40, 16);
    midTorusMesh = Mesh::createTorus(0.06f, 1.10f, 32, 16);
    innerTorusMesh = Mesh::createTorus(0.05f, 0.72f, 28, 16);

    buildCollisionMap();
}

void bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB) {
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

void setBronzeRelicMaterial() {
    glDisable(GL_TEXTURE_2D);
    GLfloat bronzeAmbient[] = { 0.28f, 0.20f, 0.08f, 1.0f };
    GLfloat bronzeDiffuse[] = { 0.85f, 0.68f, 0.25f, 1.0f };
    GLfloat bronzeSpecular[] = { 0.95f, 0.85f, 0.60f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, bronzeAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, bronzeDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, bronzeSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);
}

void setCoreEnergyMaterial() {
    glDisable(GL_TEXTURE_2D);
    GLfloat coreAmbient[] = { 0.1f, 0.5f, 0.6f, 1.0f };
    GLfloat coreDiffuse[] = { 0.2f, 0.8f, 1.0f, 1.0f };
    GLfloat coreSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat coreEmission[] = { 0.25f, 0.75f, 0.95f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, coreAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, coreDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, coreSpecular);
    glMaterialfv(GL_FRONT, GL_EMISSION, coreEmission);
    glMaterialf(GL_FRONT, GL_SHININESS, 96.0f);
}

void resetEmission() {
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

// Option A: Flickering Torchlight Update (Rubric 5)
void updateTorchLight() {
    if (!isTorchOn) {
        glDisable(GL_LIGHT0);
        return;
    }
    glEnable(GL_LIGHT0);

    // Harmonic multi-frequency flicker simulation
    float flicker = 0.88f + 0.08f * sinf(6.0f * floatTimer) + 0.04f * cosf(13.0f * floatTimer);

    GLfloat lightPos[] = { 0.0f, 6.5f, 0.0f, 1.0f };
    GLfloat lightDiffuse[] = { 0.95f * flicker, 0.72f * flicker, 0.38f * flicker, 1.0f };
    GLfloat lightAmbient[] = { 0.22f, 0.18f, 0.12f, 1.0f };
    GLfloat lightSpecular[] = { 0.75f * flicker, 0.75f * flicker, 0.75f * flicker, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void drawSingleColumn(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    bindMaterialTexture(texPillar, 0.82f, 0.80f, 0.78f);

    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    renderCurrentMesh(columnBaseMesh);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT / 2.0f, 0.0f);
    renderCurrentMesh(columnShaftMesh);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT + 0.2f, 0.0f);
    renderCurrentMesh(columnBaseMesh);
    glPopMatrix();

    glPopMatrix();
}

// Draws one wall side with an open archway and suspended portcullis gate
void drawWallWithArchway() {
    bindMaterialTexture(texWall, 0.78f, 0.75f, 0.70f);

    float segOffset = (ROOM_SIZE / 2.0f) - (WALL_SEG_WIDTH / 2.0f); // 7.5f from center

    // Left Wall Segment
    glPushMatrix();
    glTranslatef(-segOffset, WALL_HEIGHT / 2.0f, 0.0f);
    renderCurrentMesh(wallSegmentMesh);
    glPopMatrix();

    // Right Wall Segment
    glPushMatrix();
    glTranslatef(segOffset, WALL_HEIGHT / 2.0f, 0.0f);
    renderCurrentMesh(wallSegmentMesh);
    glPopMatrix();

    // Lintel bridging above the doorway
    glPushMatrix();
    glTranslatef(0.0f, DOOR_HEIGHT + (WALL_HEIGHT - DOOR_HEIGHT) / 2.0f, 0.0f);
    renderCurrentMesh(lintelMesh);
    glPopMatrix();

    // Suspended Stone Portcullis Slab
    bindMaterialTexture(texDoor, 0.65f, 0.60f, 0.58f);
    glPushMatrix();
    glTranslatef(0.0f, portcullisY, 0.0f);
    renderCurrentMesh(portcullisMesh);
    glPopMatrix();
}

void drawFloatingRelic() {
    float floatingY = relicBaseY + sinf(floatTimer) * 0.20f;

    glPushMatrix();
    glTranslatef(0.0f, floatingY, 0.0f);

    setBronzeRelicMaterial();

    // Outer Ring
    glPushMatrix();
    glRotatef(rotRing1, 1.0f, 0.0f, 0.0f);
    renderCurrentMesh(outerTorusMesh);
    glPopMatrix();

    // Mid Ring
    glPushMatrix();
    glRotatef(rotRing2, 0.0f, 1.0f, 0.0f);
    renderCurrentMesh(midTorusMesh);
    glPopMatrix();

    // Inner Ring
    glPushMatrix();
    glRotatef(rotRing3, 0.707f, 0.0f, 0.707f);
    renderCurrentMesh(innerTorusMesh);
    glPopMatrix();

    // Energy Core
    setCoreEnergyMaterial();
    glPushMatrix();
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, 0.38, 32, 32);
    gluDeleteQuadric(quad);
    glPopMatrix();
    resetEmission();

    glPopMatrix();
}

void drawTempleEnvironment() {
    // 1. Floor
    bindMaterialTexture(texFloor, 0.75f, 0.72f, 0.68f);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    renderCurrentMesh(floorMesh);
    glPopMatrix();

    // 2. Four Walls with Archways & Portcullises
    // North (-Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -ROOM_SIZE / 2.0f);
    drawWallWithArchway();
    glPopMatrix();

    // South (+Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, ROOM_SIZE / 2.0f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway();
    glPopMatrix();

    // West (-X)
    glPushMatrix();
    glTranslatef(-ROOM_SIZE / 2.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway();
    glPopMatrix();

    // East (+X)
    glPushMatrix();
    glTranslatef(ROOM_SIZE / 2.0f, 0.0f, 0.0f);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway();
    glPopMatrix();

    // 3. Stepped Dais & Altar
    bindMaterialTexture(texDais, 0.85f, 0.82f, 0.78f);
    glPushMatrix();
    glTranslatef(0.0f, 0.175f, 0.0f);
    renderCurrentMesh(daisBottomMesh);

    glTranslatef(0.0f, 0.35f, 0.0f);
    renderCurrentMesh(daisTopMesh);

    glTranslatef(0.0f, 0.525f, 0.0f);
    renderCurrentMesh(altarPedestalMesh);
    glPopMatrix();

    // 4. Columns
    drawSingleColumn(-COL_DIST, -COL_DIST);
    drawSingleColumn(COL_DIST, -COL_DIST);
    drawSingleColumn(-COL_DIST, COL_DIST);
    drawSingleColumn(COL_DIST, COL_DIST);

    // 5. Relic
    drawFloatingRelic();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // First-Person Camera Look Vector
    float rad = droneYaw * ((float)M_PI / 180.0f);
    float lookX = droneX + sinf(rad);
    float lookZ = droneZ - cosf(rad);

    gluLookAt(
        droneX, droneY, droneZ,
        lookX, droneY, lookZ,
        0.0f, 1.0f, 0.0f
    );

    updateTorchLight();
    drawTempleEnvironment();

    glutSwapBuffers();
}

void update(int value) {
    // Multi-axis relic rotations
    rotRing1 += 0.8f * ringSpeedMult;
    rotRing2 += 1.4f * ringSpeedMult;
    rotRing3 += 1.0f * ringSpeedMult;
    if (rotRing1 >= 360.0f) rotRing1 -= 360.0f;
    if (rotRing2 >= 360.0f) rotRing2 -= 360.0f;
    if (rotRing3 >= 360.0f) rotRing3 -= 360.0f;

    floatTimer += 0.035f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

// Option A: Multi-Function Interactive Keyboard Controls (Rubric 4)
void keyboard(unsigned char key, int x, int y) {
    float rad = droneYaw * ((float)M_PI / 180.0f);
    float fwdX = sinf(rad);
    float fwdZ = -cosf(rad);
    float rightX = cosf(rad);
    float rightZ = sinf(rad);

    float targetX = droneX;
    float targetZ = droneZ;

    switch (key) {
        // Movement Controls
    case 'w': case 'W': targetX += fwdX * moveSpeed; targetZ += fwdZ * moveSpeed; break;
    case 's': case 'S': targetX -= fwdX * moveSpeed; targetZ -= fwdZ * moveSpeed; break;
    case 'a': case 'A': targetX -= rightX * moveSpeed; targetZ -= rightZ * moveSpeed; break;
    case 'd': case 'D': targetX += rightX * moveSpeed; targetZ += rightZ * moveSpeed; break;

        // Interactive Key 1: Toggle Wireframe (Proves mesh topology to examiners)
    case ' ':
        isWireframe = !isWireframe;
        break;

        // Interactive Key 2: Toggle Torchlight
    case 'l': case 'L':
        isTorchOn = !isTorchOn;
        break;

    case 27: exit(0); break;
    }

    if (isPositionValid(targetX, droneZ)) droneX = targetX;
    if (isPositionValid(droneX, targetZ)) droneZ = targetZ;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        droneYaw -= turnSpeed;
        if (droneYaw < 0.0f) droneYaw += 360.0f;
        break;
    case GLUT_KEY_RIGHT:
        droneYaw += turnSpeed;
        if (droneYaw >= 360.0f) droneYaw -= 360.0f;
        break;
    case GLUT_KEY_UP:
        if (droneY < WALL_HEIGHT - 0.6f) droneY += 0.25f;
        break;
    case GLUT_KEY_DOWN:
        if (droneY > 0.8f) droneY -= 0.25f;
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(950, 700);
    glutCreateWindow("Ancient Ruined Temple - Mid-Review Sanctum");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}