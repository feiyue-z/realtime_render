#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/sceneparser.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void addCubeTessellation();
    void addSphereTessellation();
    void addCylinderTessellation();
    void addConeTessellation();

    void passVertUniforms();
    void passFragUniforms();

    glm::mat4 generateViewMatrix();
    glm::mat4 generateProjectionMatrix();
    glm::mat3 generateRotateMatrix(float theta, glm::vec3 u);

    void initializePhong();
    void initializeTexture();

    void paintShapes();
    void populateLightData();
    void populateShapeData(const RenderShapeData& shape);

    void makeFbo();
    void paintTexture();

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;
    
    GLuint m_phong_shader;
    GLuint m_texture_shader;

    GLuint m_vbo;
    GLuint m_vao;

    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_default_fbo = 2;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    bool renderDataIsParsed = false;
    bool settingIsChanged = false;

    int m_fbo_width;
    int m_fbo_height;

    RenderData m_renderData;
    std::vector<float> m_vboData;

    glm::mat4 m_model  = glm::mat4(1);
    glm::mat3 m_normal = glm::mat4(1);
    glm::mat4 m_view   = glm::mat4(1);
    glm::mat4 m_proj   = glm::mat4(1);

    glm::vec4 m_worldSpaceCameraPos;

    float m_ka;
    float m_kd;
    float m_ks;
    float m_shininess;

    glm::vec4 m_cAmbient;
    glm::vec4 m_cDiffuse;
    glm::vec4 m_cSpecular;

    int m_lType[8];
    glm::vec3 m_lFunction[8];
    glm::vec4 m_lColor[8];
    glm::vec4 m_lDirection[8];
    glm::vec4 m_lPosition[8];
    float m_lAngle[8];
    float m_lPenumbra[8];
};
