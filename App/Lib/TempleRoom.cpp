#include "pch.h"
#include "TempleRoom.h"
#include <cmath>

// ==============================================================================================
// Constructor: TempleRoom::TempleRoom
//   portcullisY is set to 12.0f (high above the 8m ceiling) so the entrance trapdoors start
//   hidden away inside the ceiling recess.
// ==============================================================================================
TempleRoom::TempleRoom()
    : portcullisY(12.0f) // Concealed trap: parked high inside ceiling recess
{
}

// ==============================================================================================
// Method: TempleRoom::init
// Purpose: Procedurally builds all meshes for the room and initializes collision bounds.
// ==============================================================================================
void TempleRoom::init() {
    // 1. Room Floor & Ceiling Slabs (26m x 26m, 0.4m thick, repeating texture 8 times)
    floorMesh = Mesh::createTexturedBox(ROOM_SIZE, 0.4f, ROOM_SIZE, 8.0f, 8.0f);
    ceilingMesh = Mesh::createTexturedBox(ROOM_SIZE, 0.4f, ROOM_SIZE, 8.0f, 8.0f);

    // 2. Seamless Continuous Walls & Overhead Lintel
    const float tileUTotal = 3.5f; // Total horizontal texture repeats across the whole 26m wall
    const float tileVTotal = 2.5f; // Total vertical texture repeats across the 8m height
    // Calculate horizontal texture boundaries corresponding to the door opening
    const float uSplit1 = (WALL_SEG_WIDTH / ROOM_SIZE) * tileUTotal;
    const float uSplit2 = ((WALL_SEG_WIDTH + DOOR_WIDTH) / ROOM_SIZE) * tileUTotal;
    // Calculate vertical texture boundaries for the arch curve and lintel
    const float vArchBottom = (3.0f / WALL_HEIGHT) * tileVTotal;
    const float vLintelBottom = (DOOR_HEIGHT / WALL_HEIGHT) * tileVTotal;

    // Left wall segment: spans from X=0 to uSplit1
    wallLeftMesh = Mesh::createTexturedBoxUV(WALL_SEG_WIDTH, WALL_HEIGHT, 0.4f, 0.0f, uSplit1, 0.0f, tileVTotal);
    // Overhead lintel: spans between uSplit1 and uSplit2, above the doorway (Y = 5m to 8m)
    lintelMesh = Mesh::createTexturedBoxUV(DOOR_WIDTH, WALL_HEIGHT - DOOR_HEIGHT, 0.4f, uSplit1, uSplit2, vLintelBottom, tileVTotal);
    // Right wall segment: spans from uSplit2 to tileUTotal
    wallRightMesh = Mesh::createTexturedBoxUV(WALL_SEG_WIDTH, WALL_HEIGHT, 0.4f, uSplit2, tileUTotal, 0.0f, tileVTotal);

    // Arch Spandrels filling wall space above curved opening (Y = 3m to 5m) with continuous UVs
    archSpandrelsMesh = Mesh::createArchSpandrels(2.0f, DOOR_WIDTH, 2.0f, 0.4f, uSplit1, uSplit2, vArchBottom, vLintelBottom, 24);

    // Carved Stone Arch Molding framing the curved opening
    archMoldingMesh = Mesh::createArch(1.92f, 2.18f, 0.45f, 24);

    // Heavy Stone/Iron Portcullis Trapdoor Slab
    portcullisMesh = Mesh::createTexturedBox(DOOR_WIDTH - 0.1f, DOOR_HEIGHT, 0.2f, 1.0f, 2.0f);

    // 3. Heavy Stone Architrave Beams & Broken Diagonal Crossbeam
    crossbeamMesh = Mesh::createTexturedBox(0.75f, 0.65f, ROOM_SIZE, 1.0f, 8.0f);
    brokenCrossbeamMesh = Mesh::createTexturedBox(0.70f, 0.60f, 9.6f, 1.0f, 3.0f);

    // 4. Stepped Ceremonial Dais & Central Altar Platform
    daisBottomMesh = Mesh::createTexturedBox(6.0f, 0.35f, 6.0f, 2.0f, 2.0f);      // Bottom stepped tier (6m x 6m)
    daisTopMesh = Mesh::createTexturedBox(4.6f, 0.35f, 4.6f, 1.5f, 1.5f);         // Middle stepped tier (4.6m x 4.6m)
    altarPedestalMesh = Mesh::createTexturedBox(1.8f, 0.70f, 1.8f, 1.0f, 1.0f);   // Central altar block (1.8m x 1.8m)

    // 5. Classical Column Components
    columnBaseMesh = Mesh::createTexturedBox(1.4f, 0.4f, 1.4f, 1.0f, 1.0f); // Square base and capital plinths
    columnShaftMesh = Mesh::createCylinder(0.52f, COLUMN_HEIGHT, 24);         // Fluted cylindrical pillar shaft

    // Initialize collision boundaries for all major structures
    buildCollisionMap();
}

// ==============================================================================================
// Method: TempleRoom::setTextures
// Purpose: Stores the texture IDs loaded by SOIL2 in Main.cpp.
// ==============================================================================================
void TempleRoom::setTextures(const TempleTextures& tex) {
    textures = tex;
}

// ==============================================================================================
// Method: TempleRoom::buildCollisionMap
// Purpose: Generates Axis-Aligned Bounding Box (AABB) colliders for all static room obstacles.
// ==============================================================================================
void TempleRoom::buildCollisionMap() {
    obstacles.clear();
    const float DRONE_RADIUS = 0.55f;

    // 1. Central Altar Dais (approx. 6m x 6m + drone radius buffer)
    float daisHalf = 3.0f + DRONE_RADIUS;
    obstacles.push_back({ -daisHalf, daisHalf, -daisHalf, daisHalf });

    // 2. The 4 Corner Columns (centered at +/-COL_DIST, +/-COL_DIST)
    float colBuffer = 0.7f + DRONE_RADIUS;
    float coords[4][2] = {
        { -COL_DIST, -COL_DIST }, // Northwest column
        {  COL_DIST, -COL_DIST }, // Northeast column
        { -COL_DIST,  COL_DIST }, // Southwest column
        {  COL_DIST,  COL_DIST }  // Southeast column
    };
    for (int i = 0; i < 4; ++i) {
        float cx = coords[i][0];
        float cz = coords[i][1];
        obstacles.push_back({ cx - colBuffer, cx + colBuffer, cz - colBuffer, cz + colBuffer });
    }

    // 3. Broken fallen diagonal crossbeam footprint on the floor
    obstacles.push_back({ -7.0f, -2.8f, -0.6f, 6.1f });
}

// ==============================================================================================
// Method: TempleRoom::addObstacles
// Purpose: Allows external subsystems (like Props) to register their own colliders with the room.
// ==============================================================================================
void TempleRoom::addObstacles(const std::vector<BoxCollider>& newObstacles) {
    obstacles.insert(obstacles.end(), newObstacles.begin(), newObstacles.end());
}

// ==============================================================================================
// Method: TempleRoom::isPositionValid
// Purpose: Evaluates whether a proposed 2D position (targetX, targetZ) is walkable / flyable.
// Parameters:
//   targetX, targetZ: The candidate coordinates the camera wants to move into.
//   droneRadius: Safety buffer around the drone.
// Returns:
//   true:  Position is clear, drone can move here.
//   false: Blocked by an outer wall or obstacle collider.
// ==============================================================================================
bool TempleRoom::isPositionValid(float targetX, float targetZ, float droneRadius) const {
    // Room perimeter boundary: walls are at +/- ROOM_SIZE / 2 (+/- 13.0m).
    // Subtract droneRadius and safety margin to keep drone inside the chamber.
    float roomLimit = (ROOM_SIZE / 2.0f) - droneRadius - 0.2f;

    // Prevent passing outside chamber perimeter through open doorways
    if (targetX > roomLimit || targetX < -roomLimit) return false;
    if (targetZ > roomLimit || targetZ < -roomLimit) return false;

    // Check collision against all registered internal obstacles (AABB point-in-box test)
    for (const auto& box : obstacles) {
        if (targetX >= box.minX && targetX <= box.maxX &&
            targetZ >= box.minZ && targetZ <= box.maxZ) {
            return false; // Drone is inside an obstacle box!
        }
    }
    return true; // Position is completely free
}

// ==============================================================================================
// Method: TempleRoom::renderMesh
// Purpose: Helper that forwards the draw call to MeshRenderer based on wireframe toggle.
// ==============================================================================================
void TempleRoom::renderMesh(MeshRenderer& renderer, const Mesh& mesh, bool isWireframe) {
    if (isWireframe) {
        renderer.renderWireframe(mesh);
    }
    else {
        renderer.renderSolid(mesh);
    }
}

// ==============================================================================================
// Method: TempleRoom::bindMaterialTexture
// Purpose: Sets OpenGL lighting material colors and binds the specified 2D texture.
// Parameters:
//   texID: OpenGL texture handle ID (e.g. textures.wall).
//   diffuseR, diffuseG, diffuseB: Base surface reflectance color tint.
//   isWireframe: If true, disables texturing so wireframe lines stand out clearly.
// ==============================================================================================
void TempleRoom::bindMaterialTexture(GLuint texID, float diffuseR, float diffuseG, float diffuseB, bool isWireframe) {
    // Only bind and enable 2D texturing when in solid mode and a valid texture ID exists
    if (!isWireframe && texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    // Material reflection properties:
    // Ambient is dimmed (35% of diffuse) to represent indirect shadow illumination.
    GLfloat matAmbient[] = { diffuseR * 0.35f, diffuseG * 0.35f, diffuseB * 0.35f, 1.0f };
    GLfloat matDiffuse[] = { diffuseR, diffuseG, diffuseB, 1.0f };
    GLfloat matSpecular[] = { 0.15f, 0.15f, 0.15f, 1.0f }; // Low specular for rough ancient stone
    GLfloat matShininess = 10.0f;                           // Dull, matte finish

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, matShininess);
}

// ==============================================================================================
// Method: TempleRoom::drawWallWithArchway
// Purpose: Draws one complete modular wall unit (Left segment, Right segment, Archway, Lintel,
//          and Portcullis).
// ==============================================================================================
void TempleRoom::drawWallWithArchway(MeshRenderer& renderer, bool isWireframe) {
    bindMaterialTexture(textures.wall, 0.78f, 0.75f, 0.70f, isWireframe);

    // segOffset is the distance from center (0) to the center of each flanking wall segment
    float segOffset = (ROOM_SIZE / 2.0f) - (WALL_SEG_WIDTH / 2.0f); // 7.5m from center

    // 1. Left Wall Segment (X = -13m to -2m)
    glPushMatrix();
    glTranslatef(-segOffset, WALL_HEIGHT / 2.0f, 0.0f);
    renderMesh(renderer, wallLeftMesh, isWireframe);
    glPopMatrix();

    // 2. Right Wall Segment (X = +2m to +13m)
    glPushMatrix();
    glTranslatef(segOffset, WALL_HEIGHT / 2.0f, 0.0f);
    renderMesh(renderer, wallRightMesh, isWireframe);
    glPopMatrix();

    // 3. Seamless Spandrels filling wall space above the arch curve (Y = 3m to 5m)
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    renderMesh(renderer, archSpandrelsMesh, isWireframe);
    glPopMatrix();

    // 4. Carved Stone Arch Molding framing the curved opening
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    renderMesh(renderer, archMoldingMesh, isWireframe);
    glPopMatrix();

    // 5. Seamless Continuous Lintel bridging above the doorway up to ceiling (Y = 5m to 8m)
    glPushMatrix();
    glTranslatef(0.0f, DOOR_HEIGHT + (WALL_HEIGHT - DOOR_HEIGHT) / 2.0f, 0.0f);
    renderMesh(renderer, lintelMesh, isWireframe);
    glPopMatrix();

    // 6. Concealed Suspended Stone Portcullis Slab (Parked high in ceiling recess)
    bindMaterialTexture(textures.door, 0.65f, 0.60f, 0.58f, isWireframe);
    glPushMatrix();
    glTranslatef(0.0f, portcullisY, 0.0f);
    renderMesh(renderer, portcullisMesh, isWireframe);
    glPopMatrix();
}

// ==============================================================================================
// Method: TempleRoom::drawSingleColumn
// Purpose: Renders a classical column at world position (x, z).
// Components:
//   - Base: Square stone block resting on the floor (Y = 0.2m).
//   - Shaft: Vertical cylinder (Y = 0.4m to 6.6m).
//   - Capital: Decorative square stone cap atop the column supporting the ceiling beams.
// ==============================================================================================
void TempleRoom::drawSingleColumn(MeshRenderer& renderer, float x, float z, bool isWireframe) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    bindMaterialTexture(textures.pillar, 0.82f, 0.80f, 0.78f, isWireframe);

    // 1. Column Base Plinth
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    renderMesh(renderer, columnBaseMesh, isWireframe);
    glPopMatrix();

    // 2. Column Fluted Shaft
    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT / 2.0f, 0.0f);
    renderMesh(renderer, columnShaftMesh, isWireframe);
    glPopMatrix();

    // 3. Column Capital (Top Cap)
    glPushMatrix();
    glTranslatef(0.0f, 0.4f + COLUMN_HEIGHT + 0.2f, 0.0f);
    renderMesh(renderer, columnBaseMesh, isWireframe);
    glPopMatrix();

    glPopMatrix();
}

// ==============================================================================================
// Method: TempleRoom::drawCeilingAndBeams
// Purpose: Renders the overhead ceiling slab, structural crossbeams, and fallen collapsed beam.
// ==============================================================================================
void TempleRoom::drawCeilingAndBeams(MeshRenderer& renderer, bool isWireframe) {
    // 1. Textured Stone Ceiling Slab at Y = 8.0m
    bindMaterialTexture(textures.floor, 0.68f, 0.65f, 0.60f, isWireframe);
    glPushMatrix();
    glTranslatef(0.0f, WALL_HEIGHT + 0.2f, 0.0f);
    renderMesh(renderer, ceilingMesh, isWireframe);
    glPopMatrix();

    // 2. Heavy Stone Architrave Beams Connecting Columns and Outer Walls
    bindMaterialTexture(textures.wall, 0.74f, 0.71f, 0.66f, isWireframe);
    float beamY = WALL_HEIGHT - 0.55f; // Y ~ 7.45m directly underneath ceiling

    // Longitudinal Beams (extending north-south along Z axis)
    glPushMatrix();
    glTranslatef(-COL_DIST, beamY, 0.0f);
    renderMesh(renderer, crossbeamMesh, isWireframe);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(COL_DIST, beamY, 0.0f);
    renderMesh(renderer, crossbeamMesh, isWireframe);
    glPopMatrix();

    // Transverse Beams (extending east-west along X axis, rotated 90 degrees)
    glPushMatrix();
    glTranslatef(0.0f, beamY, -COL_DIST);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    renderMesh(renderer, crossbeamMesh, isWireframe);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, beamY, COL_DIST);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    renderMesh(renderer, crossbeamMesh, isWireframe);
    glPopMatrix();

    // 3. Collapsed / Broken Crossbeam Fallen Diagonally from Ceiling to Floor
    // Adds atmospheric ruin aesthetics: rotated on both X and Z axes to look naturally fallen.
    bindMaterialTexture(textures.wall, 0.65f, 0.62f, 0.58f, isWireframe);
    glPushMatrix();
    // Origin near west column beam, sloping downward diagonally to floor
    glTranslatef(-COL_DIST + 1.2f, 3.4f, 0.8f);
    glRotatef(-32.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(18.0f, 0.0f, 0.0f, 1.0f);
    renderMesh(renderer, brokenCrossbeamMesh, isWireframe);
    glPopMatrix();
}

// ==============================================================================================
// Method: TempleRoom::draw
// Purpose: Master rendering function that draws the entire temple environment.
// ==============================================================================================
void TempleRoom::draw(MeshRenderer& renderer, bool isWireframe) {
    // 1. Floor (centered at Y = -0.2m so top surface sits exactly at ground level Y = 0.0m)
    bindMaterialTexture(textures.floor, 0.75f, 0.72f, 0.68f, isWireframe);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    renderMesh(renderer, floorMesh, isWireframe);
    glPopMatrix();

    // 2. Ceiling & Heavy Overhead Crossbeams
    drawCeilingAndBeams(renderer, isWireframe);

    // 3. Four Walls with Archways & Portcullises (Modular layout rotated around chamber)
    // North Wall (-Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -ROOM_SIZE / 2.0f);
    drawWallWithArchway(renderer, isWireframe);
    glPopMatrix();

    // South Wall (+Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, ROOM_SIZE / 2.0f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway(renderer, isWireframe);
    glPopMatrix();

    // West Wall (-X)
    glPushMatrix();
    glTranslatef(-ROOM_SIZE / 2.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway(renderer, isWireframe);
    glPopMatrix();

    // East Wall (+X)
    glPushMatrix();
    glTranslatef(ROOM_SIZE / 2.0f, 0.0f, 0.0f);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    drawWallWithArchway(renderer, isWireframe);
    glPopMatrix();

    // 4. Stepped Dais & Altar (Multi-tiered ceremonial pedestal at room center)
    bindMaterialTexture(textures.dais, 0.85f, 0.82f, 0.78f, isWireframe);
    glPushMatrix();
    // Lower tier
    glTranslatef(0.0f, 0.175f, 0.0f);
    renderMesh(renderer, daisBottomMesh, isWireframe);

    // Upper tier
    glTranslatef(0.0f, 0.35f, 0.0f);
    renderMesh(renderer, daisTopMesh, isWireframe);

    // Central altar pedestal block
    glTranslatef(0.0f, 0.525f, 0.0f);
    renderMesh(renderer, altarPedestalMesh, isWireframe);
    glPopMatrix();

    // 5. Four Classical Supporting Columns
    drawSingleColumn(renderer, -COL_DIST, -COL_DIST, isWireframe); // Northwest
    drawSingleColumn(renderer,  COL_DIST, -COL_DIST, isWireframe); // Northeast
    drawSingleColumn(renderer, -COL_DIST,  COL_DIST, isWireframe); // Southwest
    drawSingleColumn(renderer,  COL_DIST,  COL_DIST, isWireframe); // Southeast
}
