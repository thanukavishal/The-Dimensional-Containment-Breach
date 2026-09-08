// ==============================================================================================
// Connects user interaction to 3D visual perception:
//     - Calculates camera look-at vectors using trigonometry on the horizontal yaw angle.
//     - Decomposes movement into Forward and Right directional vectors.
//     - Performs independent X and Z axis collision checks to allow "wall sliding" when hitting obstacles.
// ==============================================================================================

#include "Camera.h"
#include <glut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==============================================================================================
// Constructor: Camera::Camera
// Purpose: Sets initial drone coordinates and speed parameters.
// Parameters:
//   startX, startY, startZ: Initial 3D spawn position.
//   startYaw: Initial rotation angle in degrees.
// ==============================================================================================
Camera::Camera(float startX, float startY, float startZ, float startYaw)
    : droneX(startX)
    , droneY(startY)
    , droneZ(startZ)
    , droneYaw(startYaw)
    , moveSpeed(0.40f)    // Move 40 cm per key press
    , turnSpeed(4.0f)     // Rotate 4 degrees per arrow key press
    , droneRadius(0.55f)  // 55 cm radius physical collision volume
{
}

// ==============================================================================================
// Method: Camera::apply
// Purpose: Configures the OpenGL view matrix using gluLookAt.
// ==============================================================================================
void Camera::apply() const {
    // Convert yaw angle from degrees to radians (standard C++ math functions require radians)
    float rad = droneYaw * ((float)M_PI / 180.0f);

    // Compute target look-at point 1 unit forward along the horizontal heading
    float lookX = droneX + sinf(rad);
    float lookZ = droneZ - cosf(rad);

    // Set the view matrix
    gluLookAt(
        droneX, droneY, droneZ, // Eye position
        lookX, droneY, lookZ,   // Look-at point
        0.0f, 1.0f, 0.0f        // World Up vector (+Y)
    );
}

// ==============================================================================================
// Method: Camera::handleMovement
// Purpose: Moves the drone forward, backward, left, or right relative to its current facing angle.
// Parameters:
//   key: Character code of the pressed key ('w', 'a', 's', 'd').
//   room: Reference to the TempleRoom for collision boundary checks.
// Returns:
//   true: If a movement key was processed.
//   false: If the key was unrelated to movement.
// ==============================================================================================
bool Camera::handleMovement(unsigned char key, const TempleRoom& room) {
    float rad = droneYaw * ((float)M_PI / 180.0f);

    // Direction unit vectors based on current yaw heading
    float fwdX = sinf(rad);
    float fwdZ = -cosf(rad);
    float rightX = cosf(rad);
    float rightZ = sinf(rad);

    float targetX = droneX;
    float targetZ = droneZ;
    bool moved = false;

    // Process directional input
    switch (key) {
    case 'w': case 'W': // Move Forward
        targetX += fwdX * moveSpeed;
        targetZ += fwdZ * moveSpeed;
        moved = true;
        break;
    case 's': case 'S': // Move Backward
        targetX -= fwdX * moveSpeed;
        targetZ -= fwdZ * moveSpeed;
        moved = true;
        break;
    case 'a': case 'A': // Strafe Left
        targetX -= rightX * moveSpeed;
        targetZ -= rightZ * moveSpeed;
        moved = true;
        break;
    case 'd': case 'D': // Strafe Right
        targetX += rightX * moveSpeed;
        targetZ += rightZ * moveSpeed;
        moved = true;
        break;
    default:
        return false; // Not a recognized movement key
    }

    // Apply movement with independent axis collision checking (Wall Sliding physics)
    if (moved) {
        // Can we move along X while keeping our current Z?
        if (room.isPositionValid(targetX, droneZ, droneRadius)) droneX = targetX;
        // Can we move along Z while keeping our current X?
        if (room.isPositionValid(droneX, targetZ, droneRadius)) droneZ = targetZ;
    }
    return true;
}

// ==============================================================================================
// Method: Camera::handleSpecial
// Purpose: Handles arrow key presses for turning (yaw) and flying up/down (altitude).
// Parameters:
//   key: GLUT special key code (GLUT_KEY_LEFT, GLUT_KEY_RIGHT, GLUT_KEY_UP, GLUT_KEY_DOWN).
//   maxWallHeight: Ceiling limit to clamp vertical ascent.
// ==============================================================================================
bool Camera::handleSpecial(int key, float maxWallHeight) {
    switch (key) {
    case GLUT_KEY_LEFT: // Rotate camera left (counter-clockwise)
        droneYaw -= turnSpeed;
        if (droneYaw < 0.0f) droneYaw += 360.0f; // Wrap around [0, 360)
        return true;

    case GLUT_KEY_RIGHT: // Rotate camera right (clockwise)
        droneYaw += turnSpeed;
        if (droneYaw >= 360.0f) droneYaw -= 360.0f; // Wrap around [0, 360)
        return true;

    case GLUT_KEY_UP: // Ascend (fly upward toward ceiling)
        // Clamp height so the drone does not fly through the ceiling slab
        if (droneY < maxWallHeight - 0.6f) droneY += 0.25f;
        return true;

    case GLUT_KEY_DOWN: // Descend (fly downward toward floor)
        // Clamp height so the drone does not clip through the floor
        if (droneY > 0.8f) droneY -= 0.25f;
        return true;

    default:
        return false;
    }
}
