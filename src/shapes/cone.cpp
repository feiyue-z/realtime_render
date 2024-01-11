#include "cone.h"

void Cone::setVertexData() {
    makeCone();
}

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cone::makeCone() {
    float r = 0.5f;
    float h = 0.5f;
    float subTheta = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currTheta = subTheta * i;
        float nextTheta = subTheta * (i + 1);

        makeSide(currTheta, nextTheta, r, h);
        makeCap(currTheta, nextTheta, r, -h, false);
    }
}

void Cone::makeSide(float currTheta, float nextTheta, float r, float h) {
    glm::vec3 top = glm::vec3(0, h, 0);

    float stepTheta = 2 * M_PI / m_param2 * 0.5;
    glm::vec3 steppedTop = glm::vec3(
        sin(currTheta + stepTheta),
        -h,
        cos(currTheta + stepTheta));

    glm::vec3 left = glm::vec3(
        r * sin(currTheta),
        -h,
        r * cos(currTheta));

    glm::vec3 right = glm::vec3(
        r * sin(nextTheta),
        -h,
        r * cos(nextTheta));

    makeRadialNormalFace(top, left, right, steppedTop);
}

void Cone::makeRadialNormalFace(glm::vec3 top,
                                glm::vec3 left,
                                glm::vec3 right,
                                glm::vec3 steppedTop) {
    glm::vec3 newLeft = interpolate(top, left, 1.f / m_param1);
    glm::vec3 newRight = interpolate(top, right, 1.f / m_param1);

    // triangle at the top
    makeTipTriangle(top, newLeft, newRight, steppedTop);

    glm::vec3 prevLeft = newLeft;
    glm::vec3 prevRight = newRight;

    for (int i = 0; i < m_param1 - 1; i++) {
        float factor = (float) (i + 2) / m_param1;

        newLeft = interpolate(top, left, factor);
        newRight = interpolate(top, right, factor);

        makeRadialNormalTriangle(prevLeft, newLeft, prevRight);
        makeRadialNormalTriangle(prevRight, newLeft, newRight);

        prevLeft = newLeft;
        prevRight = newRight;
    }
}

void Cone::makeTipTriangle(glm::vec3 top,
                           glm::vec3 left,
                           glm::vec3 right,
                           glm::vec3 steppedTop) {
    insertVec3(m_vertexData, top);
    insertVec3(m_vertexData, getRadialNormal(steppedTop));

    insertVec3(m_vertexData, left);
    insertVec3(m_vertexData, getRadialNormal(left));

    insertVec3(m_vertexData, right);
    insertVec3(m_vertexData, getRadialNormal(right));
}

void Cone::makeRadialNormalTriangle(glm::vec3 top,
                                    glm::vec3 left,
                                    glm::vec3 right) {
    insertVec3(m_vertexData, top);
    insertVec3(m_vertexData, getRadialNormal(top));

    insertVec3(m_vertexData, left);
    insertVec3(m_vertexData, getRadialNormal(left));

    insertVec3(m_vertexData, right);
    insertVec3(m_vertexData, getRadialNormal(right));
}

glm::vec3 Cone::getRadialNormal(glm::vec3 point) {
    float radialLen = sqrt(point.x * point.x + point.z * point.z);

    float x = point.x / radialLen;
    float y = 0.5;
    float z = point.z / radialLen;

    glm::vec3 normal = glm::vec3(x, y, z);
    return normalize(normal);
}

//glm::vec3 Cone::getRadialNormal(glm::vec3 point) {
//    float height = 1;
//    float radius = 0.5;

//    // Calculate the slope of the cone's side
//    float slope = height / radius;

//    // Calculate the normal vector components
//    float radialLen = sqrt(point.x * point.x + point.z * point.z);
//    float x = point.x / radialLen;
//    float z = point.z / radialLen;
//    float y = 1 / slope;

//    glm::vec3 normal = glm::vec3(x, y, z);
//    return glm::normalize(normal);
//}

void Cone::makeCap(float currTheta, float nextTheta, float r, float h, bool yIsPositive) {
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
        makePlaneNormalFace(top, left, right);
    } else {
        makePlaneNormalFace(top, right, left);
    }
}

void Cone::makePlaneNormalFace(glm::vec3 top,
                               glm::vec3 left,
                               glm::vec3 right) {
    glm::vec3 newLeft = interpolate(top, left, 1.f / m_param1);
    glm::vec3 newRight = interpolate(top, right, 1.f / m_param1);

    // triangle at the top
    makePlaneNormalTriangle(top, newLeft, newRight);

    glm::vec3 prevLeft = newLeft;
    glm::vec3 prevRight = newRight;

    for (int i = 0; i < m_param1 - 1; i++) {
        float factor = (float) (i + 2) / m_param1;

        newLeft = interpolate(top, left, factor);
        newRight = interpolate(top, right, factor);

        makePlaneNormalTriangle(prevLeft, newLeft, prevRight);
        makePlaneNormalTriangle(prevRight, newLeft, newRight);

        prevLeft = newLeft;
        prevRight = newRight;
    }
}

void Cone::makePlaneNormalTriangle(glm::vec3 top,
                                   glm::vec3 left,
                                   glm::vec3 right) {
    glm::vec3 a, b;

    insertVec3(m_vertexData, top);
    a = left - top;
    b = right - top;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, left);
    a = right - left;
    b = top - left;
    insertVec3(m_vertexData, normalize(cross(a, b)));

    insertVec3(m_vertexData, right);
    a = top - right;
    b = left - right;
    insertVec3(m_vertexData, normalize(cross(a, b)));
}

glm::vec3 Cone::interpolate(glm::vec3 x, glm::vec3 y, float a) {
    return x * (1 - a) + y * a;
}

void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
