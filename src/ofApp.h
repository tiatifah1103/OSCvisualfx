#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
//#include "VideoEcho.hpp"
#include "MotionBlur.hpp"
#include "StepPrint.hpp"
#include "Glitch.hpp"
#include "Static.hpp"

//#define OSC_PORT 9000

class ofApp : public ofBaseApp{

    public:
        void setup() override;
        void update() override;
        void draw() override;
        void exit() override;

        void keyPressed(int key) override;
        void keyReleased(int key) override;
        
    
    ofVideoPlayer video;
//    VideoEcho videoEcho;
    MotionBlur motionBlur;
    StepPrinting stepPrinting;
    GlitchEffect glitchEffect;
    StaticEffect staticEffect;
    
    ofxOscReceiver oscReceiver;

    //test commit
    //test commit 2
    
    bool isReverbActive = false;
};

