#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cone
{
public:
    void updateParams(int param1, int param2);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();

    void makeCone();
    void makeCap(float currTheta, float nextTheta, float r, float h, bool yIsPositive);
    void makeSide(float currTheta, float nextTheta, float r, float h);

    void makePlaneNormalFace(glm::vec3 top,
                                   glm::vec3 left,
                             glm::vec3 right);
    void makePlaneNormalTriangle(glm::vec3 top,
                                       glm::vec3 left,
                                       glm::vec3 right);

    void makeRadialNormalFace(glm::vec3 top,
                              glm::vec3 left,
                              glm::vec3 right,
                              glm::vec3 steppedTop);
    void makeTipTriangle(glm::vec3 top,
                         glm::vec3 left,
                         glm::vec3 right,
                         glm::vec3 steppedTop);
    void makeRadialNormalTriangle(glm::vec3 top,
                                  glm::vec3 left,
                                  glm::vec3 right);

    glm::vec3 getRadialNormal(glm::vec3 point);
    glm::vec3 interpolate(glm::vec3 x, glm::vec3 y, float a);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
