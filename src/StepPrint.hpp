//
//  StepPrintEffect.hpp
//  ccProject3
//
//  Created by Latifah Dickson on 17/11/2024.
//

#include "ofMain.h"
#pragma once

class StepPrinting{
public:
    StepPrinting();
    
    void setup(int _stepInterval);
    void update(const ofTexture &videoTexture);
    void apply(ofVideoPlayer &video, float x, float y, float width, float height);
    void clear();
    void setStepInterval(int interval);
    void setMaxStoredFrames(int maxFrames);
    
private:
    vector<ofImage> storedFrames;
    int stepInterval;
    int frameCounter;
    int maxStoredFrames;
    float feedbackFactor; // Dynamic factor influenced by feedback

};
