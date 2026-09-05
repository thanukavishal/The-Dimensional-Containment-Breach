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

// Scene geometry
Mesh floorMesh;
Mesh wallMesh;
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
float floatTimer = 0.0f;
float relicBaseY = 2.7f;

struct BoxCollider {
    float minX, maxX;
    float minZ, maxZ;
};
std::vector<BoxCollider> obstacles;

void buildCollisionMap() {
    obstacles.clear();
    float daisHalf = 3.0f + DRONE_RADIUS;
    obstacles.push_back({ -daisHalf, daisHalf, -daisHalf, daisHalf });

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
    if (targetX > roomLimit || targetX < -roomLimit) return false;
    if (targetZ > roomLimit || targetZ < -roomLimit) return false;

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
    texDais = loadTexture("floor.jpg");
}

void init() {
    glClearColor(0.06f, 0.06f, 0.09f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    // Warm Torchlight
    GLfloat lightPos[] = { 0.0f, 6.5f, 0.0f, 1.0f };
    GLfloat lightDiffuse[] = { 0.95f, 0.75f, 0.45f, 1.0f };
    GLfloat lightAmbient[] = { 0.30f, 0.25f, 0.20f, 1.0f };
    GLfloat lightSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    loadAllTextures();

    // Adjusted tiling for visual proportion
    floorMesh = Mesh::createTexturedBox(ROOM_SIZE, 0.4f, ROOM_SIZE, 8.0f, 8.0f);
    wallMesh = Mesh::createTexturedBox(ROOM_SIZE, WALL_HEIGHT, 0.4f, 4.0f, 2.0f);
    daisBottomMesh = Mesh::createTexturedBox(6.0f, 0.35f, 6.0f, 2.0f, 2.0f);
    daisTopMesh = Mesh::createTexturedBox(4.6f, 0.35f, 4.6f, 1.5f, 1.5f);
    altarPedestalMesh = Mesh::createTexturedBox(1.8f, 0.7f, 1.8f, 1.0f, 1.0f);

    columnBaseMesh = Mesh::createTexturedBox(1.4f, 0.4f, 1.4f, 1.0f, 1.0f);
    columnShaftMesh = Mesh::createCylinder(0.52f, COLUMN_HEIGHT, 24);

    outerTorusMesh = Mesh::createTorus(0.07f, 1.50f, 40, 16);
    midTorusMesh = Mesh::createTorus(0.06f, 1.10f, 32, 16);
    innerTorusMesh = Mesh::createTorus(0.05f, 0.72f, 28, 16);

    buildCollisionMap();
}

// Material Helpers for Texture Switching
void bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB) {
    if (texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    GLfloat matAmbient[] = { diffuseR * 0.4f, diffuseG * 0.4f, diffuseB * 0.4f, 1.0f };
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

void drawSingleColumn(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    bindMaterialTexture(texPillar, 0.82f, 0.80f, 0.78f);

    // Plinth base
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    renderer.renderSolid(columnBaseMesh);
    glPopMatrix();

    // Column shaft
    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT / 2.0f, 0.0f);
    renderer.renderSolid(columnShaftMesh);
    glPopMatrix();

    // Capital top
    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT + 0.2f, 0.0f);
    renderer.renderSolid(columnBaseMesh);
    glPopMatrix();

    glPopMatrix();
}

void drawFloatingRelic() {
    float floatingY = relicBaseY + sinf(floatTimer) * 0.20f;

    glPushMatrix();
    glTranslatef(0.0f, floatingY, 0.0f);

    setBronzeRelicMaterial();

    glPushMatrix();
    glRotatef(rotRing1, 1.0f, 0.0f, 0.0f);
    renderer.renderSolid(outerTorusMesh);
    glPopMatrix();

    glPushMatrix();
    glRotatef(rotRing2, 0.0f, 1.0f, 0.0f);
    renderer.renderSolid(midTorusMesh);
    glPopMatrix();

    glPushMatrix();
    glRotatef(rotRing3, 0.707f, 0.0f, 0.707f);
    renderer.renderSolid(innerTorusMesh);
    glPopMatrix();

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
    // 1. Floor with texFloor
    bindMaterialTexture(texFloor, 0.75f, 0.72f, 0.68f);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    renderer.renderSolid(floorMesh);
    glPopMatrix();

    // 2. Walls with texWall
    bindMaterialTexture(texWall, 0.78f, 0.75f, 0.70f);
    glPushMatrix();
    glTranslatef(0.0f, WALL_HEIGHT / 2.0f, -ROOM_SIZE / 2.0f);
    renderer.renderSolid(wallMesh);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, WALL_HEIGHT / 2.0f, ROOM_SIZE / 2.0f);
    renderer.renderSolid(wallMesh);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-ROOM_SIZE / 2.0f, WALL_HEIGHT / 2.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    renderer.renderSolid(wallMesh);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(ROOM_SIZE / 2.0f, WALL_HEIGHT / 2.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    renderer.renderSolid(wallMesh);
    glPopMatrix();

    // 3. Stepped Dais & Altar with texDais
    bindMaterialTexture(texDais, 0.85f, 0.82f, 0.78f);
    glPushMatrix();
    glTranslatef(0.0f, 0.175f, 0.0f);
    renderer.renderSolid(daisBottomMesh);

    glTranslatef(0.0f, 0.35f, 0.0f);
    renderer.renderSolid(daisTopMesh);

    glTranslatef(0.0f, 0.525f, 0.0f);
    renderer.renderSolid(altarPedestalMesh);
    glPopMatrix();

    // 4. Columns with texPillar
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

    float rad = droneYaw * ((float)M_PI / 180.0f);
    float lookX = droneX + sinf(rad);
    float lookZ = droneZ - cosf(rad);

    gluLookAt(
        droneX, droneY, droneZ,
        lookX, droneY, lookZ,
        0.0f, 1.0f, 0.0f
    );

    drawTempleEnvironment();

    glutSwapBuffers();
}

void update(int value) {
    rotRing1 += 0.8f;
    rotRing2 += 1.4f;
    rotRing3 += 1.0f;
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

void keyboard(unsigned char key, int x, int y) {
    float rad = droneYaw * ((float)M_PI / 180.0f);
    float fwdX = sinf(rad);
    float fwdZ = -cosf(rad);
    float rightX = cosf(rad);
    float rightZ = sinf(rad);

    float targetX = droneX;
    float targetZ = droneZ;

    switch (key) {
    case 'w': case 'W': targetX += fwdX * moveSpeed; targetZ += fwdZ * moveSpeed; break;
    case 's': case 'S': targetX -= fwdX * moveSpeed; targetZ -= fwdZ * moveSpeed; break;
    case 'a': case 'A': targetX -= rightX * moveSpeed; targetZ -= rightZ * moveSpeed; break;
    case 'd': case 'D': targetX += rightX * moveSpeed; targetZ += rightZ * moveSpeed; break;
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