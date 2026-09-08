#include <windows.h>
#include <stdlib.h>
#include <glut.h>
#include <SOIL2.h>
#include <stdio.h>
#include <cmath>

#include "Mesh.h"
#include "MeshRenderer.h"
#include "TempleRoom.h"
#include "Relic.h"
#include "Props.h"
#include "Camera.h"

// ==============================================================================================
// Core Subsystem Instances (Global Singletons)
//     - templeRoom: The building (walls, floor, ceiling, pillars, archways, and colliders).
//     - relic:      The floating multi-ring artifact in the center of the room.
//     - props:      The torches, fire particles, rubble field, pedestal, and skeleton.
//     - camera:     The first-person drone through which the player sees the 3D world.
//                   Starts at (X=0.0m, Y=2.0m, Z=10.5m) facing North (Yaw=180 degrees).
// ==============================================================================================
MeshRenderer renderer;
TempleRoom templeRoom;
Relic relic;
Props props;
Camera camera(0.0f, 2.0f, 10.5f, 0.0f);

// ==============================================================================================
// Interactive Toggles (User Controls)
// ==============================================================================================
bool isWireframe = false; // Toggled by [SPACEBAR]: switches between solid filled and wireframe rendering
bool isTorchOn = true;    // Toggled by [L]: switches dynamic ceiling torchlight on and off

// ==============================================================================================
// Function: loadTexture
// ==============================================================================================
GLuint loadTexture(const char* filename) {
    GLuint tex = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );

    // Fallback error check: If the file is missing, warn the user in the console
    if (!tex) {
        printf("Notice: '%s' not found. Falling back to default material.\n", filename);
    }
    else {
        // Configure texture wrapping modes on both axes to repeat seamlessly
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    return tex;
}

// ==============================================================================================
// Function: loadAllTextures
// ==============================================================================================
void loadAllTextures() {
    TempleTextures tex;
    tex.floor = loadTexture("floor.jpg");     // Stone floor pavers
    tex.wall = loadTexture("wall.jpg");       // Weathered carved stone wall blocks
    tex.pillar = loadTexture("pillar.jpg");   // Fluted classical marble pillar texture
    tex.dais = loadTexture("dais.jpg");       // Ornate altar dais stone
    tex.door = loadTexture("door.jpg");       // Heavy security portcullis slab
    templeRoom.setTextures(tex);
    props.setTextures(tex);
}

// ==============================================================================================
// Function: updateTorchLight
//   frequencies (6.0 and 13.0 radians):
//     flicker = 0.88 + 0.08 * sin(6.0 * t) + 0.04 * cos(13.0 * t)
//   Because 6 and 13 are prime to each other, the waves interfere chaotically, creating an
//   organic, non-repeating flame flicker
// ==============================================================================================
void updateTorchLight() {
    // If the user toggled torch light off with [L], disable the light source entirely
    if (!isTorchOn) {
        glDisable(GL_LIGHT0);
        return;
    }
    glEnable(GL_LIGHT0);

    float floatTimer = relic.getFloatTimer(); // Use the continuous animation timer
    // Chaotic flame flicker formula combining two out-of-phase trigonometric waves
    float flicker = 0.88f + 0.08f * sinf(6.0f * floatTimer) + 0.04f * cosf(13.0f * floatTimer);

    // Light source 3D position: hanging from the central ceiling at (0, 6.5, 0)
    GLfloat lightPos[] = { 0.0f, 6.5f, 0.0f, 1.0f };
    // Warm golden-orange firelight (Red=0.95, Green=0.72, Blue=0.38)
    GLfloat lightDiffuse[] = { 0.95f * flicker, 0.72f * flicker, 0.38f * flicker, 1.0f };
    // Low ambient background fill
    GLfloat lightAmbient[] = { 0.22f, 0.18f, 0.12f, 1.0f };
    // Specular highlight intensity
    GLfloat lightSpecular[] = { 0.75f * flicker, 0.75f * flicker, 0.75f * flicker, 1.0f };

    // Send light properties
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

// ==============================================================================================
// Function: init
//   - glClearColor(r, g, b, a): Sets the background color used when clearing the screen (dark night sky).
//   - glEnable(GL_DEPTH_TEST): Enables the Z-buffer (depth buffer). Without depth testing, distant
//     objects drawn later would overwrite nearby objects drawn earlier!
//   - glEnable(GL_LIGHTING): Tells OpenGL to calculate light rays striking surfaces based on normals.
//   - glEnable(GL_LIGHT0): Activates light source #0.
//   - glShadeModel(GL_SMOOTH): Enables Gouraud/smooth shading, interpolating lighting smoothly across
//     polygons instead of giving them a flat, faceted look (GL_FLAT).
// ==============================================================================================
void init() {
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f); // Dark moody atmospheric background color
    glEnable(GL_DEPTH_TEST);                  // Enable 3D depth buffer
    glEnable(GL_LIGHTING);                    // Enable OpenGL lighting calculations
    glEnable(GL_LIGHT0);                      // Turn on light source 0
    glShadeModel(GL_SMOOTH);                  // Smooth lighting interpolation across polygons

    // Initialize subsystems and load resources
    loadAllTextures();
    templeRoom.init();
    relic.init();
    props.init();

    // Register props (fallen column, rubble, rune pedestal) with TempleRoom collision system
    templeRoom.addObstacles(props.getObstacles());
}

// ==============================================================================================
// Function: drawTempleEnvironment
// Renders all static architecture, animated relics, and decorative props.
// ==============================================================================================
void drawTempleEnvironment() {
    templeRoom.draw(renderer, isWireframe);
    relic.draw(renderer, isWireframe);
    props.drawAll(renderer, isWireframe, relic.getFloatTimer());
}

// ==============================================================================================
// Function: display (GLUT Display Callback)
//     - "Back Buffer": The GPU draws the entire scene off-screen invisibly.
//     - "Front Buffer": The monitor displays the previously completed image.
//     - glutSwapBuffers(): Instantly swaps the front and back buffers once drawing is 100% complete
// ==============================================================================================
void display() {
    // Clear both the color buffer (wiping previous frame) and depth buffer (resetting distance checks)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Switch to the Modelview matrix (handles camera view and object world placement)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset matrix to identity (origin)

    // 1. Position and orient the camera view
    camera.apply();

    // 2. Update dynamic flickering point light
    updateTorchLight();

    // 3. Render all 3D scene geometry
    drawTempleEnvironment();

    // Swap back buffer to front buffer for clean, tear-free presentation
    glutSwapBuffers();
}

// ==============================================================================================
// Function: update (GLUT Timer Callback)
//   16 milliseconds per frame = 1000ms / 16ms ~ 62.5 FPS.
//   Each tick, update the relic ring rotation, advance the flame particle physics by dt=0.016s,
//   request a screen redraw (glutPostRedisplay), and schedule the next timer tick in 16ms!
// ==============================================================================================
void update(int value) {
    relic.update();                // Spin the relic rings and increment levitation sine timer
    props.update(0.016f);          // Simulate 16ms of flame particle physics and turbulence
    glutPostRedisplay();           // Request GLUT to call display() to render the updated frame
    glutTimerFunc(16, update, 0);  // Schedule next update tick in 16 milliseconds
}

// ==============================================================================================
// Function: reshape (GLUT Window Resize Callback)
//     - Field of View (FOV): 60.0 degrees vertical viewing angle.
//     - Aspect Ratio: w / h (prevents the scene from stretching or squishing when resizing window).
//     - Near Clipping Plane: 0.1m (geometry closer than 10cm is clipped).
//     - Far Clipping Plane: 100.0m (geometry beyond 100m is not rendered).
// ==============================================================================================
void reshape(int w, int h) {
    if (h == 0) h = 1; // Prevent division by zero
    glViewport(0, 0, w, h); // Map the 3D rendering area to the entire window area

    // Switch to Projection matrix to configure the 3D camera lens
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

    // Switch back to Modelview matrix for standard object transformations
    glMatrixMode(GL_MODELVIEW);
}

// ==============================================================================================
// Function: keyboard (Standard ASCII Keyboard Input Callback)
// Keys:
//   - W, A, S, D: Forward, Strafe Left, Backward, Strafe Right.
//   - [SPACEBAR]: Toggle wireframe rendering mode on / off.
//   - L / l:      Toggle dynamic torch lighting on / off.
//   - [ESC] (27): Clean exit from application.
// ==============================================================================================
void keyboard(unsigned char key, int x, int y) {
    // First, check if the key is a camera drone movement command (W, A, S, D)
    if (camera.handleMovement(key, templeRoom)) {
        glutPostRedisplay(); // Redraw immediately to show new camera position
        return;
    }

    // Process interactive toggle keys
    switch (key) {
    case ' ': // Spacebar: toggle between wireframe mode and solid textured mode
        isWireframe = !isWireframe;
        break;
    case 'l': case 'L': // 'L' key: toggle dynamic torchlight on and off
        isTorchOn = !isTorchOn;
        break;
    case 27: // ASCII 27 is the Escape key: quit application
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// ==============================================================================================
// Function: specialKeys (GLUT Special Key Callback)
// Keys:
//   - LEFT ARROW:  Turn camera left (Yaw rotation).
//   - RIGHT ARROW: Turn camera right (Yaw rotation).
//   - UP ARROW:    Ascend / fly upward.
//   - DOWN ARROW:  Descend / fly downward.
// ==============================================================================================
void specialKeys(int key, int x, int y) {
    if (camera.handleSpecial(key, TempleRoom::WALL_HEIGHT)) {
        glutPostRedisplay();
    }
}

// ==============================================================================================
// Function: main
// ==============================================================================================
int main(int argc, char** argv) {
    // Initialize the GLUT toolkit
    glutInit(&argc, argv);

    // Configure display mode: Double buffering, RGB color buffer, and Depth buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Set initial window size (950x700 pixels) and title
    glutInitWindowSize(950, 700);
    glutCreateWindow("Ancient Ruined Temple");

    // Run our custom scene initialization
    init();

    // Register GLUT event callback functions
    glutDisplayFunc(display);       // Called whenever the window needs rendering
    glutReshapeFunc(reshape);       // Called when window is resized
    glutKeyboardFunc(keyboard);     // Called when a standard key is pressed
    glutSpecialFunc(specialKeys);   // Called when a special key (arrow keys) is pressed
    glutTimerFunc(0, update, 0);    // Start the recurring 60 FPS animation timer loop

    // Enter the infinite GLUT event processing loop (never returns until window closed or exit(0))
    glutMainLoop();
    return 0;
}