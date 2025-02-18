#pragma once
#include "ofMain.h"
#include <random>

struct Clip {
    std::string videoPath;
    std::string description;
    ofVideoPlayer video;
};

struct Anchor {
    std::string videoPath;
    std::string description;
    ofVideoPlayer video;
};

struct Topic {
    std::string name;
    Anchor anchor;
    std::vector<Clip> footage;
};

class ChronologyManager : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);

    ofVideoPlayer* getCurrentVideo();
private:
    // Topics and clips
    std::vector<Topic> topics;
    Topic* currentTopic = nullptr;
    int currentFootageIndex = 0;
    bool playingAnchor = true;

    // Looping controls
    bool isLooping = false;
    float loopStartTime = 0;
    float loopEndTime = 0;
    float loopDuration = 10.0f;

    void selectRandomTopic();
    void randomizeFootageOrder();
    void playCurrentFootage();
    void startLooping();
    void stopLooping();
};
