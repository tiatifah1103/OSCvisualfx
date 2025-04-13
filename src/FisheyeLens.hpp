#pragma once
#include "ofMain.h"
#include "ofVideoPlayer.h"

class FisheyeLens {
public:
    FisheyeLens();
    void setup(float _distortionStrength = 0.5f);
    void update(const ofTexture &videoTexture);
    void apply(float x, float y, float width, float height);
    void setDistortionStrength(float strength);
    float getDistortionStrength() const;
    
private:
    float distortionStrength;
    ofFbo distortedFrame;
};
