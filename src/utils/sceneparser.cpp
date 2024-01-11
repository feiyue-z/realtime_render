#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // clear container
    renderData = RenderData();

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();
    renderData.cameraData.pos[3] = 1;

    SceneNode* root = fileReader.getRootNode();
    glm::mat4 ctm = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 0, 1));
    constructFromTree(root, renderData, ctm);

    return true;
}

void SceneParser::constructFromTree(SceneNode* root, RenderData &renderData, glm::mat4 ctm) {
    if (root == nullptr) {
        return;
    }

    for (SceneTransformation* each : root->transformations) {
        switch (each->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = glm::translate(ctm, each->translate);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = glm::rotate(ctm, each->angle, each->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm = glm::scale(ctm, each->scale);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = each->matrix;
            break;
        }
    }

    for (ScenePrimitive* each : root->primitives) {
        renderData.shapes.push_back(
            RenderShapeData {
                *each,
                ctm,
                glm::transpose(glm::inverse(glm::mat3(ctm)))
            });
    }

    for (SceneLight* each : root->lights) {
        switch (each->type) {
        case LightType::LIGHT_POINT:
            renderData.lights.push_back(
                SceneLightData {
                    each->id,
                    each->type,
                    each->color,
                    each->function,
                    ctm * glm::vec4(0, 0, 0, 1),
                    each->dir,      // not applicable
                    each->penumbra, // not applicable
                    each->angle,    // not applicable
                    each->width,    // not supported
                    each->height    // not supported
                });
            break;
        case LightType::LIGHT_DIRECTIONAL:
            renderData.lights.push_back(
                SceneLightData {
                    each->id,
                    each->type,
                    each->color,
                    each->function,
                    glm::vec4(0, 0, 0, 1),    // not applicable
                    ctm * each->dir,
                    each->penumbra, // not applicable
                    each->angle,    // not applicable
                    each->width,    // not supported
                    each->height    // not supported
                });
            break;
        case LightType::LIGHT_SPOT:
            renderData.lights.push_back(
                SceneLightData {
                    each->id,
                    each->type,
                    each->color,
                    each->function,
                    ctm * glm::vec4(0, 0, 0, 1),
                    ctm * each->dir,
                    each->penumbra,
                    each->angle,
                    each->width,    // not supported
                    each->height    // not supported
                });
            break;
        }
    }

    for (SceneNode* each : root->children) {
        constructFromTree(each, renderData, ctm);
    }
}
