#pragma once
#include "ofMain.h"

class StaticEffect {
public:
    StaticEffect();
    void setup();
    void update();
    void apply(ofVideoPlayer& video, float x, float y, float width, float height);
    void toggleStatic(bool active);

    bool isStaticActive;
    
private:
    
    float videoXPos;       // To track the horizontal position of the video
    float speed;           // Speed of the horizontal movement
    ofFbo staticOverlay;   // FBO to store the static effect
    void generateStatic();
};
