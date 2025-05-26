#pragma once
#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxMidi.h"
#include "ofSoundStream.h"

// Forward declare ofApp to break circular dependency
class ofApp;

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

    struct SplitScreenClip {
        std::string id;
        std::string file;
        bool hasAudio;
        ofVideoPlayer video;
    };
    
    // Variables
    std::vector<Topic> topics;
    Topic* currentTopic = nullptr;
    vector<SplitScreenClip> splitScreenClips;

    int currentFootageIndex = 0;
    bool playingAnchor = true;

    // Core openFrameworks functions
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    
    // MIDI methods
    void newMidiMessage(ofxMidiMessage& message);
    void drawSplitScreen();

    ofVideoPlayer* getCurrentVideo();
    
    bool isSplitScreenActive = false;  // Flag to control split screen state
       void toggleSplitScreen(bool enable);  // Method to toggle split screen state
    bool isPlayingAnchor() const { return playingAnchor; }
    
    bool splitScreenMode = false;
    int currentSplitIndex = 0;


private:

    void selectRandomTopic();
    void randomizeFootageOrder();
    void playCurrentFootage();
    void startLooping();
    void stopLooping();
    void randomizeSplitScreenOrder();
    
    bool isLooping = false;           // To track whether the loop is active
    float loopStartTime = 0;
   float loopEndTime = 0; // Time when the loop starts
    float loopDuration = 6.0f;       // 10 seconds for the loop
    bool isVideoLooping = false;
    
    // MIDI objects
    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;
    
    bool note66Pressed = false;
    bool note66HasAdvanced = false;
    bool needReshuffleSplitScreen = true;
    

    




};
