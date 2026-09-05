#pragma once
#include "Mesh.h"

class MeshRenderer {
public:
    void renderSolid(const Mesh& mesh);
    void renderWireframe(const Mesh& mesh);
};