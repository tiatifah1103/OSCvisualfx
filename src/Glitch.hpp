//
//  GlitchEffect.hpp
//  ccProject3
//
//  Created by Latifah Dickson on 17/11/2024.
//

#pragma once
#include "ofMain.h"

class GlitchEffect {
public:
    GlitchEffect();

    void setup(int _glitchInterval, float _glitchStrength, float _glitchProbability);
    void update(ofTexture &videoTexture);
    void draw(float x, float y, float width, float height);

private:
    struct GlitchRect {
        float offsetX, offsetY, width, height, displacement;
    };

    void generateGlitches(float width, float height);
    void drawGlitches(ofTexture &tex, float x, float y, float w, float h);

    ofFbo glitchFbo;
    ofShader glitchShader;

    vector<GlitchRect> frozenGlitches;

    int glitchInterval;
    int frameCounter;
    float glitchStrength;
    float glitchProbability;

    bool isFrozen;
    int freezeTimer;
    int freezeDuration;

    int fboWidth, fboHeight;
};
