#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cylinder
{
public:
    void updateParams(int param1, int param2);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();

    void makeCylinder();
    void makeSide(float currTheta, float nextTheta,
                  float r, float h);
    void makeCap(float currTheta,
                 float nextTheta,
                 float r, float h,
                 bool yIsPositive);

    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeTileFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);

    void makeTriangleFace(glm::vec3 center,
                          glm::vec3 left,
                          glm::vec3 right);
    void makeTriangle(glm::vec3 center,
                      glm::vec3 left,
                      glm::vec3 right);

    glm::vec3 getRadialNormal(glm::vec3 point);
    glm::vec3 interpolate(glm::vec3 x, glm::vec3 y, float a);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
