#pragma once
#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxMidi.h"
//#include "ofApp.h"


class ChronologyManager : public ofBaseApp, public ofxMidiListener {
public:
    // Struct for video clips
    struct Clip {
        std::string videoPath;
        std::string description;
        ofVideoPlayer video;
    };

    // Struct for topics
    struct Topic {
        std::string name;
        Clip anchor;
        std::vector<Clip> footage;
    };

    // Variables
    std::vector<Topic> topics;
    Topic* currentTopic = nullptr;

    int currentFootageIndex = 0;
    bool playingAnchor = true;

    // Core openFrameworks functions
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    
    // MIDI methods
    void newMidiMessage(ofxMidiMessage& message);
    
    ofVideoPlayer* getCurrentVideo();

private:

    void selectRandomTopic();
    void randomizeFootageOrder();
    void playCurrentFootage();
    void startLooping();
    void stopLooping();
    
    bool isLooping = false;           // To track whether the loop is active
    float loopStartTime = 0;
   float loopEndTime = 0; // Time when the loop starts
    float loopDuration = 6.0f;       // 10 seconds for the loop
    bool isVideoLooping = false;
    
    // MIDI objects
    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;

};
