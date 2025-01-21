//
//  GlitchEffect.hpp
//  ccProject3
//
//  Created by Latifah Dickson on 17/11/2024.
//

#pragma once
#include "ofMain.h"

class GlitchEffect{
public:
    
    GlitchEffect();
    void setup (int _glitchInterval, float _glitchStrength, float _glitchProbability);
    void update(ofVideoPlayer &video);
    void apply(ofVideoPlayer &video, float x, float y, float width, float height);
    void applyGlitch(ofVideoPlayer &video, float x, float y, float width, float height);
    

private:
    struct Glitch {
            float offsetX, offsetY;
            float width, height;
            float displacement;
        };

        int glitchInterval;
        int frameCounter;
        int glitchStrength;
        float glitchProbability;

        bool isFrozen;                 // Whether glitches are frozen
        int freezeDuration;            // How long to hold the frozen state
        int freezeTimer;               // Timer for managing freeze state

        std::vector<Glitch> frozenGlitches; // Store frozen glitches
};
