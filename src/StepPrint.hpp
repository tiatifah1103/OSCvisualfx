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
   // void apply(ofVideoPlayer &video, float x, float y, float width, float height);
    void clear();
    void setStepInterval(int interval);
    void setMaxStoredFrames(int maxFrames, bool forceClear = false);

    void setFadeStrength(float strength);
  //  void clearFrames();
    bool isActive() const;
    
    void resetAllParameters();
    void clearFrames();
    void apply(ofFbo& fbo);
    
private:
    std::vector<ofFbo> storedFrames;
    int stepInterval;
    int frameCounter;
    int maxStoredFrames;
    float feedbackFactor; // Dynamic factor influenced by feedback
    float fadeStrength = 1.5f; // controls how fast frames fade (lower = smoother trail)

};
