#include "cylinder.h"

void Cylinder::setVertexData() {
    makeCylinder();
}

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::makeCylinder() {
    float r = 0.5f;
    float h = 0.5f;
    float subTheta = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currTheta = subTheta * i;
        float nextTheta = subTheta * (i + 1);

        makeSide(currTheta, nextTheta, r, h);
        makeCap(currTheta, nextTheta, r, h, true);
        makeCap(currTheta, nextTheta, r, -h, false);
    }
}

void Cylinder::makeSide(float currTheta, float nextTheta,
                        float r, float h) {
    glm::vec3 topLeft = glm::vec3(
        r * sin(currTheta),
        h,
        r * cos(currTheta));

    glm::vec3 topRight = glm::vec3(
        r * sin(nextTheta),
        h,
        r * cos(nextTheta));

    glm::vec3 bottomLeft = glm::vec3(
        r * sin(currTheta),
        -h,
        r * cos(currTheta));

    glm::vec3 bottomRight = glm::vec3(
        r * sin(nextTheta),
        -h,
        r * cos(nextTheta));

    makeTileFace(topLeft, topRight, bottomLeft, bottomRight);
}

void Cylinder::makeTileFace(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight) {
    for (int i = 0; i < m_param1; i++) {
        // vertical
        float vFactor = (float) i / m_param1;

        glm::vec3 newTopLeft =
            interpolate(topLeft, bottomLeft, vFactor);

        glm::vec3 newTopRight =
            interpolate(topRight, bottomRight, vFactor);

        glm::vec3 newBottomLeft =
            interpolate(topLeft, bottomLeft, vFactor + 1.f / m_param1);

        glm::vec3 newBottomRight =
            interpolate(topRight, bottomRight, vFactor + 1.f / m_param1);

        makeTile(newTopLeft, newTopRight, newBottomLeft, newBottomRight);
    }
}

void Cylinder::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {
    // triangle 1
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, getRadialNormal(topLeft));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, getRadialNormal(bottomLeft));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, getRadialNormal(topRight));

    // triangle 2
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, getRadialNormal(topRight));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, getRadialNormal(bottomLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, getRadialNormal(bottomRight));
}

glm::vec3 Cylinder::getRadialNormal(glm::vec3 point) {
    glm::vec3 normal = glm::vec3(point.x / 0.5, 0, point.z / 0.5);
    return normalize(normal);
}

void Cylinder::makeCap(float currTheta,
                       float nextTheta,
                       float r, float h,
                       bool yIsPositive) {
    glm::vec3 top = glm::vec3(0, h, 0);

    glm::vec3 left = glm::vec3(
        r * sin(currTheta),
        h,
        r * cos(currTheta));

    glm::vec3 right = glm::vec3(
        r * sin(nextTheta),
        h,
        r * cos(nextTheta));

    if (yIsPositive) {
        makeTriangleFace(top, left, right);
    } else {
        makeTriangleFace(top, right, left);
    }
}

void Cylinder::makeTriangleFace(glm::vec3 top,
                                glm::vec3 left,
                                glm::vec3 right) {
    // triangle at the top
    glm::vec3 newLeft = interpolate(top, left, 1.f / m_param1);
    glm::vec3 newRight = interpolate(top, right, 1.f / m_param1);

    makeTriangle(top, newLeft, newRight);

    glm::vec3 prevLeft = newLeft;
    glm::vec3 prevRight = newRight;

    for (int i = 0; i < m_param1 - 1; i++) {
        float factor = (float) (i + 2) / m_param1;

        newLeft = interpolate(top, left, factor);
        newRight = interpolate(top, right, factor);

        makeTriangle(prevLeft, newLeft, prevRight);
        makeTriangle(prevRight, newLeft, newRight);

        prevLeft = newLeft;
        prevRight = newRight;
    }
}

void Cylinder::makeTriangle(glm::vec3 center,
                            glm::vec3 left,
                            glm::vec3 right) {
    glm::vec3 a, b;

    insertVec3(m_vertexData, center);
    a = left - center;
    b = right - center;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, left);
    a = right - left;
    b = center - left;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, right);
    a = center - right;
    b = left - right;
    insertVec3(m_vertexData, normalize(cross(a, b)));
}

glm::vec3 Cylinder::interpolate(glm::vec3 x, glm::vec3 y, float a) {
    return x * (1 - a) + y * a;
}

void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
