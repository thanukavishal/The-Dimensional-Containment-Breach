#pragma once
#include "TempleRoom.h"

class Camera {
public:
    // Constructor: Initializes the camera drone at a given 3D position and horizontal yaw angle.
    // Default: (0.0, 2.0, 10.5) with Yaw = 180.0 degrees (standing near the south door, looking north toward the relic).
    Camera(float startX = 0.0f, float startY = 2.0f, float startZ = 10.5f, float startYaw = 180.0f);

    // Applies the camera view matrix to the current OpenGL Modelview matrix using gluLookAt.
    // Must be called each frame before drawing any 3D objects in the scene!
    void apply() const;

    // Handles standard keyboard movement keys (W = forward, S = backward, A = strafe left, D = strafe right).
    // Tests proposed positions against TempleRoom collision obstacles.
    // Returns: true if the key was a movement key that changed position, false otherwise.
    bool handleMovement(unsigned char key, const TempleRoom& room);

    // Handles GLUT special keys (Arrow Left/Right to rotate yaw, Arrow Up/Down to ascend/descend altitude).
    // maxWallHeight: Ceiling limit preventing the drone from flying through the roof.
    // Returns: true if the key was processed, false otherwise.
    bool handleSpecial(int key, float maxWallHeight = 8.0f);

    // Getters for position and orientation
    float getX() const { return droneX; }
    float getY() const { return droneY; }
    float getZ() const { return droneZ; }
    float getYaw() const { return droneYaw; }

    // Setters for position and orientation
    void setPosition(float x, float y, float z) { droneX = x; droneY = y; droneZ = z; }
    void setYaw(float yaw) { droneYaw = yaw; }

private:
    float droneX;       // Current horizontal position (left / right)
    float droneY;       // Current altitude / height (up / down)
    float droneZ;       // Current depth position (forward / backward)
    float droneYaw;     // Current compass direction / heading angle in degrees (0 = looking South along +Z, 180 = looking North along -Z)

    float moveSpeed;    // Distance traveled per movement key press (0.40m per step)
    float turnSpeed;    // Angle rotated per turn key press (4.0 degrees per press)
    float droneRadius;  // Physical radius of the drone (0.55m) used for collision clearance
};
