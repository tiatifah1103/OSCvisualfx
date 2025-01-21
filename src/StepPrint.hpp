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
    void update(ofVideoPlayer &video);
    void apply(ofVideoPlayer &video, float x, float y, float width, float height);
    void clear();
    
private:
    vector<ofImage> storedFrames;
    int stepInterval;
    int frameCounter;
    int maxStoredFrames;
};
