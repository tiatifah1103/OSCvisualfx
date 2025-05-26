#include "ChronologyManager.hpp"


void ChronologyManager::setup() {
    // Load JSON
    ofFile file("footage.json");
    if (file.exists()) {
        ofJson json = ofLoadJson(file);
        // Iterate through each topic defined in the JSON
        for (const auto& topicJson : json["topics"]) {
            Topic topic;
            topic.name = topicJson["topic_name"];

            // Load anchor point
            topic.anchor.videoPath = topicJson["anchor_points"][0]["video_path"];
            topic.anchor.description = topicJson["anchor_points"][0]["description"];
            topic.anchor.video.load(topic.anchor.videoPath);
            topic.anchor.video.setLoopState(OF_LOOP_NONE); // Play anchor once

            // Load all associated footage clips for the topic
            for (const auto& footageJson : topicJson["footage"]) {
                Clip clip;
                clip.videoPath = footageJson["video_path"];
                clip.description = footageJson["description"];
                clip.video.load(clip.videoPath);
                clip.video.setLoopState(OF_LOOP_NORMAL); // Loop footage videos indefinitely
                topic.footage.push_back(clip);
            }

            // Add fully constructed topic to the list
            topics.push_back(topic);
        }
        


        // Pick a random topic to start with
        selectRandomTopic();
        
      //  loadSplitScreenClips();

    }
    
    // Load split screen from external JSON
    ofJson splitJson = ofLoadJson("splitscreen.json");

        for (const auto& entry : splitJson["splitScreens"]) {
            SplitScreenClip clip;
            clip.id = entry["id"];
            clip.file = entry["file"];
            clip.hasAudio = entry["hasAudio"];
            
            // Load and configure split screen video clips
            clip.video.load("videos/" + clip.file);
            clip.video.setLoopState(OF_LOOP_NORMAL);
            clip.video.stop();  // Stop them initially
            clip.video.setVolume(0.0f); // Silent by default

            splitScreenClips.push_back(clip);
            ofLog() << "Loaded " << splitScreenClips.size() << " split screen clips.";

        }
    
    // Setup MIDI input
    midiIn.listInPorts();  // List available MIDI ports
    midiIn.openPort(0);    // Open the first available MIDI port (adjust as needed)
    midiIn.addListener(this);
    midiIn.setVerbose(true);
   }


void ChronologyManager::update() {
    if (currentTopic) {
        // If manual looping is enabled, manage loop playback timing
        if (isLooping) {
            // Get current playback time in seconds
            float currentTime = currentTopic->footage[currentFootageIndex].video.getPosition() *
                                currentTopic->footage[currentFootageIndex].video.getDuration();

            // Check if we've exceeded the loop end time
            if (currentTime > loopEndTime) {
                // Restart the video at the loop's start time
                float normalizedLoopStart = loopStartTime / currentTopic->footage[currentFootageIndex].video.getDuration();
                currentTopic->footage[currentFootageIndex].video.setPosition(normalizedLoopStart);
                currentTopic->footage[currentFootageIndex].video.play();
            }
        }

        if (playingAnchor) {
            currentTopic->anchor.video.update();
            if (currentTopic->anchor.video.getIsMovieDone()) {
                // Stop anchor playback
                currentTopic->anchor.video.stop();

                // Anchor completed; switch to footage
                playingAnchor = false;
                currentFootageIndex = 0;
                randomizeFootageOrder();
            }
        } else {
            // Update the current looping footage clip
            currentTopic->footage[currentFootageIndex].video.update();
        }
    }
}

void ChronologyManager::draw() {
    ofBackground(0);
    if (currentTopic) {
        if (splitScreenMode && !splitScreenClips.empty()) {
            drawSplitScreen();
        } else {
            // Fullscreen drawing of either anchor or regular footage
            if (playingAnchor) {
                currentTopic->anchor.video.draw(0, 0, ofGetWidth(), ofGetHeight());
            } else {
                currentTopic->footage[currentFootageIndex].video.draw(0, 0, ofGetWidth(), ofGetHeight());
            }
        }
    }
}


void ChronologyManager::drawSplitScreen() {
    float halfWidth = ofGetWidth() / 2.0f;
    float height = ofGetHeight();

    // Draw main footage - left side
    if (!playingAnchor) {
        // Ensures main video is playing
        if (!currentTopic->footage[currentFootageIndex].video.isPlaying()) {
            currentTopic->footage[currentFootageIndex].video.play();
        }
        currentTopic->footage[currentFootageIndex].video.draw(0, 0, halfWidth, height);
    } else {
        // If anchor active, draw instead
        currentTopic->anchor.video.draw(0, 0, halfWidth, height);
    }

    // Draw right side (split screen content)
    if (splitScreenMode && !splitScreenClips.empty()) {
        // Ensure split screen video is playing
        if (!splitScreenClips[currentSplitIndex].video.isPlaying()) {
            splitScreenClips[currentSplitIndex].video.play();
        }
        
        // Update the right side video
        splitScreenClips[currentSplitIndex].video.update();
        
        // Aspect-ratio correct dimensions
        float videoWidth = splitScreenClips[currentSplitIndex].video.getWidth();
        float videoHeight = splitScreenClips[currentSplitIndex].video.getHeight();
        float videoAspect = videoWidth / videoHeight;
        float screenAspect = halfWidth / height;
        
        float drawWidth, drawHeight;
        float drawX = halfWidth;
        float drawY = 0;
        
        if (videoAspect > screenAspect) {
            // Width-constrained scaling
            drawWidth = halfWidth;
            drawHeight = drawWidth / videoAspect;
            drawY = (height - drawHeight) / 2.0f;
        } else {
            // Height-constrained scaling
            drawHeight = height;
            drawWidth = drawHeight * videoAspect;
            drawX = halfWidth + (halfWidth - drawWidth) / 2.0f;
        }
        
        splitScreenClips[currentSplitIndex].video.draw(drawX, drawY, drawWidth, drawHeight);
    }
}

void ChronologyManager::keyPressed(int key) {
    if (currentTopic && !playingAnchor) {
        if (key == OF_KEY_RIGHT) {
            // Skip to the next footage
            currentFootageIndex = (currentFootageIndex + 1) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == OF_KEY_LEFT) {
            // Skip to the previous footage
            currentFootageIndex = (currentFootageIndex - 1 + currentTopic->footage.size()) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == 'n') {
            // Skip to a new random topic
            selectRandomTopic();
        }

        
        if (key == 'l') {
            if (isLooping) {
                stopLooping();
            } else {
                startLooping();
            }
        }
        
    }
}

void ChronologyManager::selectRandomTopic() {
    // Stop all videos from the current topic (if any) before switching
    if (currentTopic) {
        currentTopic->anchor.video.stop();
        for (auto& clip : currentTopic->footage) {
            clip.video.stop();
        }
    }

    int randomIndex;
    do {
        randomIndex = ofRandom(topics.size());
    } while (currentTopic && &topics[randomIndex] == currentTopic);

    currentTopic = &topics[randomIndex];
    playingAnchor = true;

    // Play the new topic's anchor video
    currentTopic->anchor.video.play();
    ofLog() << "Switched to topic: " << currentTopic->name;
}

void ChronologyManager::randomizeFootageOrder() {
    std::random_device rd;  // gets random seed from the hardware
    std::mt19937 g(rd());               // Seed the random number generator
    std::shuffle(currentTopic->footage.begin(), currentTopic->footage.end(), g); // Shuffles the footage vector
    playCurrentFootage(); // Plays the first video in the newly shuffled order
}

void ChronologyManager::playCurrentFootage() {
    // stops all other video clips except the one currently being played
    for (auto& clip : currentTopic->footage) {
        if (&clip != &currentTopic->footage[currentFootageIndex]) {
            clip.video.stop(); // stops non-current videos to avoid overlap
        }
    }
    
    // Start/restart the current video
    currentTopic->footage[currentFootageIndex].video.setLoopState(OF_LOOP_NORMAL); // loop video
    currentTopic->footage[currentFootageIndex].video.play();
    
    isLooping = false; // Reset manual looping
    ofLog() << "Playing footage (looped): " << currentTopic->footage[currentFootageIndex].videoPath; // Log current video
}

// Starts a short manual loop near the current playback position (for the right jogwheel)
void ChronologyManager::startLooping() {
    float currentTime = currentTopic->footage[currentFootageIndex].video.getPosition() *
                        currentTopic->footage[currentFootageIndex].video.getDuration(); // Get current time in seconds
    loopStartTime = std::max(0.0f, currentTime - loopDuration); // Define start of loop, clamped to  0
    loopEndTime = currentTime;  // Define end of loop at current position
    isLooping = true;                  // Enable manual looping
    ofLog() << "Started looping from " << loopStartTime << "s to " << loopEndTime << "s";
}

// Stops any active manual looping
void ChronologyManager::stopLooping() {
    isLooping = false;                 // Disable looping
    loopStartTime = 0;                 // Reset loop start
    loopEndTime = 0;                   // Reset loop end
    ofLog() << "Exited the loop.";     // Log loop exit
}

void ChronologyManager::newMidiMessage(ofxMidiMessage& message) {
    midiMessage = message; // Store the incoming message for debugging
    
    if (currentTopic && !playingAnchor) {
        // Handle jogwheel (Controller #25)
        if (message.status == MIDI_CONTROL_CHANGE && message.control == 25) {
            static bool jogwheelSpinning = false;
            static bool jogwheelReversed = false;
            static unsigned long lastMovementTime = 0;
            const int movementThresholdMin = 5;
            const int movementThresholdMax = 10;
            const int reverseMovementThresholdMin = 110;
            const int reverseMovementThresholdMax = 124;
            
            if (message.value >= movementThresholdMin && message.value <= movementThresholdMax) {
                jogwheelSpinning = true;
                jogwheelReversed = false;
                lastMovementTime = ofGetElapsedTimeMillis();
            } else if (message.value >= reverseMovementThresholdMin && message.value <= reverseMovementThresholdMax) {
                jogwheelReversed = true;
                jogwheelSpinning = false;
                lastMovementTime = ofGetElapsedTimeMillis();
            } else if (jogwheelSpinning && message.value == 1) {
                unsigned long now = ofGetElapsedTimeMillis();
                if (now - lastMovementTime > 100) {
                    jogwheelSpinning = false;
                    currentFootageIndex = (currentFootageIndex + 1) % currentTopic->footage.size();
                    playCurrentFootage();
                    ofLog() << "Jogwheel released (clockwise): Advancing to next clip";
                }
            } else if (jogwheelReversed && message.value == 127) {
                unsigned long now = ofGetElapsedTimeMillis();
                if (now - lastMovementTime > 100) {
                    jogwheelReversed = false;
                    currentFootageIndex = (currentFootageIndex - 1 + currentTopic->footage.size()) % currentTopic->footage.size();
                    playCurrentFootage();
                    ofLog() << "Jogwheel released (anti-clockwise): Going back to previous clip";
                }
            }
        }
        
        // Handle jogwheel
        if (message.status == MIDI_CONTROL_CHANGE && message.control == 24) {
            if (message.value >= 5 && message.value <= 10) {
                if (!isLooping) {
                    startLooping();
                }
                ofLog() << "Jogwheel turned clockwise: Looping enabled.";
            }
            
            if (message.value >= 110 && message.value <= 124) {
                if (isLooping) {
                    stopLooping();
                }
                ofLog() << "Jogwheel turned anti-clockwise: Looping disabled.";
            }
        }
        
        // Handles split screen control
        if (message.status == MIDI_CONTROL_CHANGE && message.control == 10) {
            bool enableSplitScreen = message.value >= 64;
            if (splitScreenMode != enableSplitScreen) {
                toggleSplitScreen(enableSplitScreen);
                ofLog() << "MIDI Controller #27: Split screen " << (enableSplitScreen ? "ON" : "OFF");
            }
        }
    
        
        // Handles split screen advancement
        if (message.status == MIDI_NOTE_ON && message.pitch == 66) {
            if (!note66Pressed && !note66HasAdvanced) {
                note66Pressed = true;
                note66HasAdvanced = true;
                
                if (splitScreenMode && !splitScreenClips.empty()) {
                    // Stops current video
                    splitScreenClips[currentSplitIndex].video.stop();
                    
                    // Checks if reached end
                    if (currentSplitIndex + 1 >= splitScreenClips.size()) {
                        // Reshuffle and start from beginning
                        randomizeSplitScreenOrder();
                    } else {
                        // Advance to next clip
                        currentSplitIndex++;
                    }
                    
                    // Start new video from beginning
                    splitScreenClips[currentSplitIndex].video.play();
                    
                    ofLog() << "MIDI Note 66: Advanced to split screen clip "
                    << currentSplitIndex << " - " << splitScreenClips[currentSplitIndex].file;
                }
            }
        }
        else if (message.status == MIDI_NOTE_OFF && message.pitch == 66) {
            note66Pressed = false;
            note66HasAdvanced = false;
        }
    }
    
    
    if (!currentTopic) return;
    
    // Always allow topic switching via MIDI notes 60/51
    if (message.status == MIDI_NOTE_ON && (message.pitch == 60 || message.pitch == 51)) {
        selectRandomTopic();
    }
}
    
    ofVideoPlayer* ChronologyManager::getCurrentVideo() {
        if (currentTopic) {
            if (playingAnchor) {
                return &currentTopic->anchor.video;
            } else {
                return &currentTopic->footage[currentFootageIndex].video;
            }
        }
        return nullptr; // Return null if no video is playing
    }
    
void ChronologyManager::toggleSplitScreen(bool enable) {
    if (playingAnchor) {
        enable = false;
        ofLog() << "Split screen disabled during anchor playback";
    }

    splitScreenMode = enable;
    isSplitScreenActive = enable;
    
    if (enable) {
        if (!splitScreenClips.empty()) {
            // Reshuffles when first activating split screen
            if (needReshuffleSplitScreen) {
                randomizeSplitScreenOrder();
            }
            
            // All split screen videos to loop
            for (auto& clip : splitScreenClips) {
                clip.video.setLoopState(OF_LOOP_NORMAL);
            }
            
            if (!splitScreenClips[currentSplitIndex].video.isPlaying()) {
                splitScreenClips[currentSplitIndex].video.play();
            }
            ofLog() << "Split screen activated with clip: " << splitScreenClips[currentSplitIndex].file;
            
            if (!playingAnchor && !currentTopic->footage[currentFootageIndex].video.isPlaying()) {
                playCurrentFootage();
            }
        } else {
            ofLogWarning() << "No split screen clips available!";
            splitScreenMode = false;
            isSplitScreenActive = false;
        }
    } else {
        for (auto& clip : splitScreenClips) {
            clip.video.setPaused(true);
        }
        // Reshuffle when split screen is activated
        needReshuffleSplitScreen = true;
        ofLog() << "Split screen deactivated (videos paused)";
    }
}

void ChronologyManager::randomizeSplitScreenOrder() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(splitScreenClips.begin(), splitScreenClips.end(), g);
    currentSplitIndex = 0;
    needReshuffleSplitScreen = false;
    ofLog() << "Randomized split screen clip order";
}
