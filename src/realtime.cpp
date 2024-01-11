#include "realtime.h"
#include "settings.h"
#include "utils/shaderloader.h"
#include "utils/sceneparser.h"
#include "shapes/cube.h"
#include "shapes/sphere.h"
#include "shapes/cylinder.h"
#include "shapes/cone.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_texture_shader);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;

    m_phong_shader   = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    initializePhong();
    initializeTexture();
}

void Realtime::initializePhong() {
    // generate and bind vbo
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // generate and bind vao
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0));

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // clean up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::initializeTexture() {
    glUseProgram(m_texture_shader);

    GLint sampleImageLocation = glGetUniformLocation(m_texture_shader, "sampleImage");
    if (sampleImageLocation != -1) {
        glUniform1i(sampleImageLocation, 0);
    }

    float kernel[25];
    for (int i = 0; i < 25; i++) {
//        kernel[i] = 1.f / 25;
        kernel[i] = 0.04f;

        std::string tag = "kernel[" + std::to_string(i) + "]";
        GLint kernelLocation = glGetUniformLocation(m_texture_shader, tag.c_str());
        if (kernelLocation != -1) {
            glUniform1f(kernelLocation, kernel[i]);
        }
    }

    glm::vec2 offset = glm::vec2(
        1.f / m_fbo_width,
        1.f / m_fbo_height
    );
    GLint offsetLocation = glGetUniformLocation(m_texture_shader, "offset");
    if (offsetLocation != -1) {
        glUniform2fv(offsetLocation, 1, &offset[0]);
    }

    glUseProgram(0);

    std::vector<GLfloat> fullscreen_quad_data = {
        // vertex 1
        -1.f, 1.f, 0.f,
        // uv coor 1
        0.f, 1.f,

        // vertex 2
        -1.f, -1.f, 0.f,
        // uv coor 2
        0.f,  0.f,

        // vertex 3
        1.f, -1.f, 0.f,
        // uv coor 3
        1.f,  0.f,

        // vertex 4
        1.f, 1.f, 0.f,
        // uv coor 4
        1.f, 1.f,

        // vertex 5
        -1.f, 1.f, 0.f,
        // uv coor 5
        0.f,  1.f,

        // vertex 6
        1.f, -1.f, 0.f,
        // uv coor 6
        1.f,  0.f
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size() * sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFbo();
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    if (!renderDataIsParsed) {
        return;
    }

    // draw to offscreen framebuffer -- m_fbo
    if (settings.perPixelFilter || settings.kernelBasedFilter) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_fbo_width, m_fbo_height);
    }

    // clear screen color and depth
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paintShapes();

    // unbind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);

    if (settings.perPixelFilter || settings.kernelBasedFilter) {
        glUseProgram(m_texture_shader);

        GLint applyGrayscaleLocation = glGetUniformLocation(m_texture_shader, "applyGrayscale");
        if (applyGrayscaleLocation != -1) {
            glUniform1i(applyGrayscaleLocation, settings.perPixelFilter);
        }

        GLint applyBlurLocation = glGetUniformLocation(m_texture_shader, "applyBlur");
        if (applyBlurLocation != -1) {
            glUniform1i(applyBlurLocation, settings.kernelBasedFilter);
        }

        glUseProgram(0);

        paintTexture();
    }
}

void Realtime::passVertUniforms() {
    // model matrix
    GLint modelMatrixLocation = glGetUniformLocation(m_phong_shader, "modelMatrix");
    if (modelMatrixLocation != -1) {
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &m_model[0][0]);
    }

    // normal matrix
    GLint normalMatrixLocation = glGetUniformLocation(m_phong_shader, "normalMatrix");
    if (normalMatrixLocation != -1) {
        glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, &m_normal[0][0]);
    }

    // view matrix
    GLint viewMatrixLocation = glGetUniformLocation(m_phong_shader, "viewMatrix");
    if (viewMatrixLocation != -1) {
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &m_view[0][0]);
    }

    // projection matrix
    GLint projectionMatrixLocation = glGetUniformLocation(m_phong_shader, "projectionMatrix");
    if (projectionMatrixLocation != -1) {
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &m_proj[0][0]);
    }
}

void Realtime::passFragUniforms() {
    /// LIGHT ///

    std::string tag;

    for (int i = 0; i < 8; i++) {

        std::string index = std::to_string(i);

        // light type
        tag = "m_lType[" + index + "]";
        GLint typeLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (typeLocation != -1) {
            glUniform1i(typeLocation, m_lType[i]);
        }

        // light function
        tag = "m_lFunction[" + index + "]";
        GLint functionLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (functionLocation != -1) {
            glUniform3fv(functionLocation, 1, &m_lFunction[i][0]);
        }

        // light color
        tag = "m_lColor[" + index + "]";
        GLint colorPosition = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (colorPosition != -1) {
            glUniform4fv(colorPosition, 1, &m_lColor[i][0]);
        }

        // light direction
        tag = "m_lDirection[" + index + "]";
        GLint directionLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (directionLocation != -1) {
            glUniform4fv(directionLocation, 1, &m_lDirection[i][0]);
        }

        // light position
        tag = "m_lPosition[" + index + "]";
        GLint positionLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (positionLocation != -1) {
            glUniform4fv(positionLocation, 1, &m_lPosition[i][0]);
        }

        // light angle
        tag = "m_lAngle[" + index + "]";
        GLint angleLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (angleLocation != -1) {
            glUniform1f(angleLocation, m_lAngle[i]);
        }

        // light penumbra
        tag = "m_lPenumbra[" + index + "]";
        GLint penumbraLocation = glGetUniformLocation(m_phong_shader, tag.c_str());
        if (penumbraLocation != -1) {
            glUniform1f(penumbraLocation, m_lPenumbra[i]);
        }
    }

    /// AMBIENT TERM ///

    // m_ka
    GLint mkaLocation = glGetUniformLocation(m_phong_shader, "m_ka");
    if (mkaLocation != -1) {
        glUniform1f(mkaLocation, m_ka);
    }

    // ambient color
    GLint cAmbientLocation = glGetUniformLocation(m_phong_shader, "m_cAmbient");
    if (cAmbientLocation != -1) {
        glUniform4fv(cAmbientLocation, 1, &m_cAmbient[0]);
    }

    /// DIFFUSE TERM ///

    // m_kd
    GLint mkdLocation = glGetUniformLocation(m_phong_shader, "m_kd");
    if (mkdLocation != -1) {
        glUniform1f(mkdLocation, m_kd);
    }

    // diffuse color
    GLint cDiffuseLocation = glGetUniformLocation(m_phong_shader, "m_cDiffuse");
    if (cDiffuseLocation != -1) {
        glUniform4fv(cDiffuseLocation, 1, &m_cDiffuse[0]);
    }

    /// SPECULAR TERM ///

    // m_ks
    GLint mksLocation = glGetUniformLocation(m_phong_shader, "m_ks");
    if (mksLocation != -1) {
        glUniform1f(mksLocation, m_ks);
    }

    // specular color
    GLint cSpecularLocation = glGetUniformLocation(m_phong_shader, "m_cSpecular");
    if (cSpecularLocation != -1) {
        glUniform4fv(cSpecularLocation, 1, &m_cSpecular[0]);
    }

    // shininess
    GLint shininessLocation = glGetUniformLocation(m_phong_shader, "m_shininess");
    if (shininessLocation != -1) {
        glUniform1f(shininessLocation, m_shininess);
    }

    // camera position
    GLint worldSpaceCameraPosition = glGetUniformLocation(m_phong_shader, "m_cameraPos");
    if (worldSpaceCameraPosition != -1) {
        glUniform4fv(worldSpaceCameraPosition, 1, &m_worldSpaceCameraPos[0]);
    }
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;

    populateLightData();
}

void Realtime::sceneChanged() {
    // read render data from file
    SceneParser::parse(settings.sceneFilePath, m_renderData);
    populateLightData();

    renderDataIsParsed = true;

    update(); // asks for a PaintGL() call to occur
}

void Realtime::populateLightData() {
    m_view = generateViewMatrix();
    m_proj = generateProjectionMatrix();

    m_worldSpaceCameraPos = inverse(m_view) * glm::vec4(0, 0, 0, 1);

    m_ka = m_renderData.globalData.ka;
    m_kd = m_renderData.globalData.kd;
    m_ks = m_renderData.globalData.ks;

    for (int i = 0; i < 8; i++) {
        if (i >= m_renderData.lights.size()) {
            m_lType[i] = 0;
//            m_color[i] = glm::vec4(0);
//            m_direction[i] = glm::vec4(0);
//            m_lightPos[i] = glm::vec4(0);
//            m_function[i] = glm::vec3(0);

            continue;
        }

        const SceneLightData& light = m_renderData.lights[i];

        switch(light.type) {
        case LightType::LIGHT_DIRECTIONAL:
            m_lType[i] = 1;
            m_lDirection[i] = light.dir;
            break;
        case LightType::LIGHT_POINT:
            m_lType[i] = 2;
            m_lPosition[i] = light.pos;
            break;
        case LightType::LIGHT_SPOT:
            m_lType[i] = 3;
            m_lDirection[i] = light.dir;
            m_lPosition[i] = light.pos;
            m_lAngle[i] = light.angle;
            m_lPenumbra[i] = light.penumbra;
            break;
        }

        m_lFunction[i] = light.function;
        m_lColor[i] = light.color;
    }
}

void Realtime::paintShapes() {
    // activate phong shader
    glUseProgram(m_phong_shader);

    // bind vao
    glBindVertexArray(m_vao);

    // populate render data and draw
    for (const RenderShapeData& shape : m_renderData.shapes) {
        m_vboData.clear();

        populateShapeData(shape);

        // send data to vbo
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vboData.size() * sizeof(GLfloat), m_vboData.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // pass uniform data
        passFragUniforms();
        passVertUniforms();

        // draw
        glDrawArrays(GL_TRIANGLES, 0, m_vboData.size() / 6);
    }

    // unbind vao
    glBindVertexArray(0);

    // deactivate phong shader
    glUseProgram(0);
}

void Realtime::paintTexture() {
    // activate texture shader
    glUseProgram(m_texture_shader);

    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Realtime::populateShapeData(const RenderShapeData& shape) {
    switch (shape.primitive.type) {
    case PrimitiveType::PRIMITIVE_CUBE:
        addCubeTessellation();
        break;
    case PrimitiveType::PRIMITIVE_SPHERE:
        addSphereTessellation();
        break;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        addCylinderTessellation();
        break;
    case PrimitiveType::PRIMITIVE_CONE:
        addConeTessellation();
        break;
    case PrimitiveType::PRIMITIVE_MESH:
        // todo
        break;
    }

    m_model = shape.ctm;
    m_normal = shape.normalMatrix;

    SceneMaterial material = shape.primitive.material;
    m_cAmbient = material.cAmbient;
    m_cDiffuse = material.cDiffuse;
    m_cSpecular = material.cSpecular;
    m_shininess = material.shininess;
}

void Realtime::settingsChanged() {
    populateLightData();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::makeFbo() {
    // texture
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width,  m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // render buffer
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // frame buffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // attach texture and render buffer to fbo
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // unbind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
}

void Realtime::addCubeTessellation() {
    Cube cube;
    cube.updateParams(settings.shapeParameter1);
    std::vector<float> cubeData = cube.generateShape();

    // append to the end of m_vboData
    m_vboData.insert(m_vboData.end(),
                     std::make_move_iterator(cubeData.begin()),
                     std::make_move_iterator(cubeData.end()));
}

void Realtime::addSphereTessellation() {
    Sphere sphere;
    sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> sphereData = sphere.generateShape();

    // append to the end of m_vboData
    m_vboData.insert(m_vboData.end(),
                     std::make_move_iterator(sphereData.begin()),
                     std::make_move_iterator(sphereData.end()));
}

void Realtime::addCylinderTessellation() {
    Cylinder cylinder;
    cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> cylinderData = cylinder.generateShape();

    // append to the end of m_vboData
    m_vboData.insert(m_vboData.end(),
                     std::make_move_iterator(cylinderData.begin()),
                     std::make_move_iterator(cylinderData.end()));
}

void Realtime::addConeTessellation() {
    Cone cone;
    cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> coneData = cone.generateShape();

    // append to the end of m_vboData
    m_vboData.insert(m_vboData.end(),
                     std::make_move_iterator(coneData.begin()),
                     std::make_move_iterator(coneData.end()));
}

glm::mat4 Realtime::generateViewMatrix() {
    glm::vec4 w = -glm::normalize(m_renderData.cameraData.look);
    glm::vec4 v = glm::normalize(m_renderData.cameraData.up - glm::dot(m_renderData.cameraData.up, w) * w);
    glm::vec3 cross = glm::cross(
        glm::vec3(v),
        glm::vec3(w));
    glm::vec4 u = glm::vec4(cross[0], cross[1], cross[2], 0);

    glm::mat4 rotate = glm::mat4(
        glm::vec4(u[0], v[0], w[0], 0),
        glm::vec4(u[1], v[1], w[1], 0),
        glm::vec4(u[2], v[2], w[2], 0),
        glm::vec4(0, 0, 0, 1));

    glm::vec4 pos = m_renderData.cameraData.pos;
    glm::mat4 translate = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(-pos[0], -pos[1], -pos[2], 1));

    return rotate * translate;
}

glm::mat4 Realtime::generateProjectionMatrix() {
    float aspect = static_cast<float>(size().width()) / static_cast<float>(size().height());

    float thetaH = m_renderData.cameraData.heightAngle;
    float thetaW = 2 * atan(aspect * tan(thetaH / 2));

    float near = settings.nearPlane;
    float far = settings.farPlane;

    glm::mat4 scale = glm::mat4(
        glm::vec4(1 / (far * tan(thetaW / 2)), 0, 0, 0),
        glm::vec4(0, 1 / (far * tan(thetaH / 2)), 0, 0),
        glm::vec4(0, 0, 1 / far, 0),
        glm::vec4(0, 0, 0, 1));

    float c = -near / far;

    // perspective to parallel
    glm::mat4 parallel = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1 / (1 + c), -1),
        glm::vec4(0, 0, -c / (1 + c), 0));

    // map [0, 1] to [-1, 1]
    glm::mat4 range = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, -2, 0),
        glm::vec4(0, 0, -1, 1));

    return range * parallel * scale;
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        bool cameraChanged = false;

        if (deltaX != 0) {
            glm::mat3 rotate = generateRotateMatrix(glm::radians(deltaX / 100.f), glm::vec3(0, 1, 0));
            m_renderData.cameraData.look = glm::vec4(rotate * glm::vec3(m_renderData.cameraData.look), 0);
            m_renderData.cameraData.up = glm::vec4(rotate * glm::vec3(m_renderData.cameraData.up), 0);
            cameraChanged = true;
        }
        if (deltaY != 0) {
            glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(m_renderData.cameraData.look), glm::vec3(m_renderData.cameraData.up)));
            glm::mat3 rotate = generateRotateMatrix(glm::radians(deltaY / 100.f), axis);
            m_renderData.cameraData.look = glm::vec4(rotate * glm::vec3(m_renderData.cameraData.look), 0);
            m_renderData.cameraData.up = glm::vec4(rotate * glm::vec3(m_renderData.cameraData.up), 0);
            cameraChanged = true;
        }

        if (cameraChanged) {
            // update view matrix
            m_view = generateViewMatrix();
//            // update world space camera pos
//            m_worldSpaceCameraPos = inverse(m_view) * m_renderData.cameraData.pos;
        }

        update(); // asks for a PaintGL() call to occur
    }
}

glm::mat3 Realtime::generateRotateMatrix(float theta, glm::vec3 u) {
    glm::mat3 rotate = glm::mat3(
        glm::vec3(
            cos(theta) + u.x * u.x * (1 - cos(theta)),
            u.x * u.y * (1 - cos(theta)) + u.z * sin(theta),
            u.x * u.z * (1 - cos(theta)) - u.y * sin(theta)),
        glm::vec3(
            u.x * u.y * (1 - cos(theta)) - u.z * sin(theta),
            cos(theta) + u.y * u.y * (1 - cos(theta)),
            u.y * u.z * (1 - cos(theta)) + u.x * sin(theta)
        ),
        glm::vec3(
            u.x * u.z * (1 - cos(theta)) + u.y * sin(theta),
            u.y * u.z * (1 - cos(theta)) - u.x * sin(theta),
            cos(theta) + u.z * u.z * (1 - cos(theta))
        )
    );
    return rotate;
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    bool cameraMoved = false;
    float distanceToMove = deltaTime * 5;

    if (m_keyMap[Qt::Key_W]) {
        m_renderData.cameraData.pos += m_renderData.cameraData.look * distanceToMove;
        cameraMoved = true;
    }
    if (m_keyMap[Qt::Key_S]) {
        m_renderData.cameraData.pos += -m_renderData.cameraData.look * distanceToMove;
        cameraMoved = true;
    }
    if (m_keyMap[Qt::Key_A]) {
        glm::vec3 left = glm::cross(glm::vec3(m_renderData.cameraData.up), glm::vec3(m_renderData.cameraData.look));
        m_renderData.cameraData.pos += glm::vec4(left, 0) * distanceToMove;
        cameraMoved = true;
    }
    if (m_keyMap[Qt::Key_D]) {
        glm::vec3 right = -glm::cross(glm::vec3(m_renderData.cameraData.up), glm::vec3(m_renderData.cameraData.look));
        m_renderData.cameraData.pos += glm::vec4(right, 0) * distanceToMove;
        cameraMoved = true;
    }
    if (m_keyMap[Qt::Key_Space]) {
        m_renderData.cameraData.pos += glm::vec4(0, 1, 0, 0) * distanceToMove;
        cameraMoved = true;
    }
    if (m_keyMap[Qt::Key_Control]) {
        m_renderData.cameraData.pos += glm::vec4(0, -1, 0, 0) * distanceToMove;
        cameraMoved = true;
    }

    if (cameraMoved) {
        // update view matrix
        m_view = generateViewMatrix();
        // update world space camera pos
        m_worldSpaceCameraPos = inverse(m_view) * m_renderData.cameraData.pos;
    }

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
