#version 330 core

in vec3 worldSpacePos;
in vec3 worldSpaceNormal;

out vec4 fragColor;

// light
uniform int m_lType[8];
uniform vec3 m_lFunction[8];
uniform vec4 m_lColor[8];
uniform vec4 m_lDirection[8];
uniform vec4 m_lPosition[8];
uniform float m_lAngle[8];
uniform float m_lPenumbra[8];

// ambient term
uniform float m_ka;
uniform vec4 m_cAmbient;

// diffuse term
uniform float m_kd;
uniform vec4 m_cDiffuse;

// specular term
uniform float m_ks;
uniform float m_shininess;
uniform vec4 m_cSpecular;
uniform vec4 m_cameraPos;

vec4 getSpotLightColor(int i, vec4 pointToLight) {
    float outer = m_lAngle[i];
    float inner = m_lAngle[i] - m_lPenumbra[i];

    float x = acos(dot(-pointToLight, m_lDirection[i]) / (length(-pointToLight) * length(m_lDirection[i])));

    if (x < inner) {
        return m_lColor[i];
    }
    if (x > inner && x < outer) {
        float div = (x - inner) / (outer - inner);
        float falloff = -2 * pow(div, 3) + 3 * pow(div, 2);
        return m_lColor[i] * (1 - falloff);
    }
    return vec4(0.0);
}

void main() {
    // output initialization
    fragColor = vec4(0.0);

    vec3 normalizedWorldSpaceNormal = normalize(worldSpaceNormal);

    // ambient term
    fragColor += m_ka * m_cAmbient;

    for (int i = 0; i < 8; i++) {

        vec4 pointToLight; // ray from light to intersection
        vec4 color;

        // undefined
        if (m_lType[i] == 0) {
            return;
        }
        // directional
        else if (m_lType[i] == 1) {
            pointToLight = -m_lDirection[i];
            color = m_lColor[i];
        }
        // point
        else if (m_lType[i] == 2) {
            pointToLight = m_lPosition[i] - vec4(worldSpacePos, 1);
            color = m_lColor[i];
        }
        // spot
        else if (m_lType[i] == 3) {
            pointToLight = m_lPosition[i] - vec4(worldSpacePos, 1);
            color = getSpotLightColor(i, pointToLight);
        }

        float d = length(pointToLight); // distance from light to intersection
        float attenuation = min(1.0, 1.0 / (m_lFunction[i][0] + d * m_lFunction[i][1] + d * d * m_lFunction[i][2]));

        // diffuse term
        vec4 diffuse =
            m_kd *
            m_cDiffuse *
            max(0, min(1, dot(vec4(normalizedWorldSpaceNormal, 0.0), normalize(pointToLight))));

        fragColor += attenuation * color * diffuse;

        // specular term
        float dotProduct = dot(
            normalize(reflect(-pointToLight, vec4(normalizedWorldSpaceNormal, 0.0))),
            normalize(m_cameraPos - vec4(worldSpacePos, 1.0)));
        float clamppedDot = max(0, min(1, dotProduct));
        float power = 1;
        if (!(clamppedDot == 0 && m_shininess <= 0)) {
            power = pow(clamppedDot, m_shininess);
        }
        vec4 specular =
            m_ks *
            m_cSpecular *
            power;

        fragColor += attenuation * color * specular;
    }
}
