#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ChronologyManager.hpp"
//#include "VideoEcho.hpp"
#include "MotionBlur.hpp"
#include "StepPrint.hpp"
#include "Glitch.hpp"
#include "Static.hpp"
#include "FisheyeLens.hpp"

//#define OSC_PORT 9000

class ofApp : public ofBaseApp{

    public:
        void setup() override;
        void update() override;
        void draw() override;
        void exit() override;

        void keyPressed(int key) override;
      //  void keyReleased(int key) override;
        
    
    ofVideoPlayer video;
//    VideoEcho videoEcho;
    MotionBlur motionBlur;
    StepPrinting stepPrinting;
    GlitchEffect glitchEffect;
    StaticEffect staticEffect;
    
    ofxOscReceiver oscReceiver;
    
    
    ofFbo videoFbo;

    int standardWidth = ofGetWidth();
    int standardHeight = ofGetHeight();


    //test commit
    //test commit 2
    
    bool isReverbActive = false;
    bool isDelayActive = false;
    
    // Declare a vector to store the video objects
    vector<ofVideoPlayer> videos;
    int currentVideoIndex = 0;  // Track the current video index
    
    ChronologyManager chronologyManager;
    FisheyeLens fisheye;
    bool isFisheyeActive = false;

};

