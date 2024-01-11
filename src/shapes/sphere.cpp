#include "sphere.h"

void Sphere::setVertexData() {
    makeSphere();
}

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeSphere() {
    float subTheta = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currSubTheta = subTheta * i;
        float nextSubTheta = subTheta * (i + 1);

        makeWedge(currSubTheta, nextSubTheta);
    }
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float r = 0.5f;
    float subPhi = glm::radians(180.f / m_param1);

    for (int i = 0; i < m_param1; i++) {
        float currSubPhi = subPhi * i;
        float nextSubPhi = subPhi * (i + 1);

        glm::vec3 topLeft(r * glm::sin(currSubPhi) * glm::sin(currentTheta),
                          r * glm::cos(currSubPhi),
                          r * glm::sin(currSubPhi) * glm::cos(currentTheta));

        glm::vec3 topRight(r * glm::sin(currSubPhi) * glm::sin(nextTheta),
                           r * glm::cos(currSubPhi),
                           r * glm::sin(currSubPhi) * glm::cos(nextTheta));

        glm::vec3 bottomLeft(r * glm::sin(nextSubPhi) * glm::sin(currentTheta),
                             r * glm::cos(nextSubPhi),
                             r * glm::sin(nextSubPhi) * glm::cos(currentTheta));

        glm::vec3 bottomRight(r * glm::sin(nextSubPhi) * glm::sin(nextTheta),
                              r * glm::cos(nextSubPhi),
                              r * glm::sin(nextSubPhi) * glm::cos(nextTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // triangle 1
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));

    // triangle 2
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
}

void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
