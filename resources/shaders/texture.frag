#version 330 core

in vec2 uvPos;

uniform bool applyGrayscale;
uniform bool applyBlur;
uniform sampler2D sampleImage;
uniform vec2 offset;
uniform float kernel[25];

out vec4 fragColor;

vec4 getGrayscaleColor(vec4 color) {
    float g = color[0] * 0.299 + color[1] * 0.587 + color[2] * 0.114;
    color[0] = g;
    color[1] = g;
    color[2] = g;
    return color;
}

vec4 getBlurredColor(bool gray) {
    vec4 result = vec4(0);
    int count = 0;

    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            vec2 currOffset = vec2(j * offset[0], i * offset[1]);
            vec4 currColor = texture(sampleImage, uvPos + currOffset);
            if (gray) {
                currColor = getGrayscaleColor(currColor);
            }
            result += currColor * kernel[count++];
        }
    }

    return clamp(result, 0.f, 1.f);
}

void main()
{
    fragColor = vec4(0);
    fragColor = texture(sampleImage, uvPos);

    if (applyGrayscale && applyBlur) {
        fragColor = getBlurredColor(true);
    } else if (applyBlur) {
        fragColor = getBlurredColor(false);
    } else if (applyGrayscale) {
        fragColor = getGrayscaleColor(fragColor);
    }
}
