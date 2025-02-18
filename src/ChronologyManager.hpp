

#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class ChronologyManager : public ofBaseApp {
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

    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    ofVideoPlayer* getCurrentVideo();

private:
    // Helper functions
    void selectRandomTopic();
    void randomizeFootageOrder();
    void playCurrentFootage();
    
    bool isLooping = false;
    float loopStartTime = 0;
    float loopDuration = 6.0f;       // 6 seconds for the loop
    bool isVideoLooping = false;
};
