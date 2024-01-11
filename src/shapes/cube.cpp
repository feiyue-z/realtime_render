#include "cube.h"

void Cube::setVertexData() {
    // face 1: + xy plane
    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    // face 2: - xy plane
    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    // face 3: + yz plane
    makeFace(glm::vec3(0.5f,  -0.5f,  0.5f),
             glm::vec3(0.5f,   0.5f,  0.5f),
             glm::vec3(0.5f,  -0.5f, -0.5f),
             glm::vec3(0.5f,   0.5f, -0.5f));

    // face 4: - yz plane
    makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    // face 5: + xz plane
    makeFace(glm::vec3( 0.5f, 0.5f,  0.5f),
             glm::vec3(-0.5f, 0.5f,  0.5f),
             glm::vec3( 0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f));

    // face 6: - xz plane
    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));
}

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    glm::vec3 height = bottomLeft - topLeft;

    for (int i = 0; i < m_param1; i++) {
        // vertical
        float vFactor = (float) i / m_param1;

        for (int j = 0; j < m_param1; j++) {
            // horizontal
            float hFactor = (float) j / m_param1;

            glm::vec3 newTopLeft =
                interpolate(topLeft, topRight, hFactor) + height * vFactor;

            glm::vec3 newTopRight =
                interpolate(topLeft, topRight, hFactor + 1.f / m_param1) + height * vFactor;

            glm::vec3 newBottomLeft =
                interpolate(topLeft, topRight, hFactor) + height * (vFactor + 1.f / m_param1);

            glm::vec3 newBottomRight =
                interpolate(topLeft, topRight, hFactor + 1.f / m_param1) + height * (vFactor + 1.f / m_param1);

            makeTile(newTopLeft, newTopRight, newBottomLeft, newBottomRight);
        }
    }
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    glm::vec3 a, b;

    // triangle 1
    insertVec3(m_vertexData, topLeft);
    a = bottomLeft - topLeft;
    b = topRight - topLeft;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, bottomRight);
    a = topRight - bottomRight;
    b = bottomLeft - bottomRight;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, topRight);
    a = topLeft - topRight;
    b = bottomRight - topRight;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    // triangle 2
    insertVec3(m_vertexData, topLeft);
    a = bottomLeft - topLeft;
    b = topRight - topLeft;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, bottomLeft);
    a = bottomRight - bottomLeft;
    b = topLeft - bottomLeft;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, bottomRight);
    a = topRight - bottomRight;
    b = bottomLeft - bottomRight;
    insertVec3(m_vertexData, normalize(cross(a, b)));
}

glm::vec3 Cube::interpolate(glm::vec3 x, glm::vec3 y, float a) {
    return x * (1 - a) + y * a;
}

void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
