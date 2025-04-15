#include "ChronologyManager.hpp"


void ChronologyManager::setup() {
    // Load JSON
    ofFile file("footage.json");
    if (file.exists()) {
        ofJson json = ofLoadJson(file);
        for (const auto& topicJson : json["topics"]) {
            Topic topic;
            topic.name = topicJson["topic_name"];

            // Load anchor point
            topic.anchor.videoPath = topicJson["anchor_points"][0]["video_path"];
            topic.anchor.description = topicJson["anchor_points"][0]["description"];
            topic.anchor.video.load(topic.anchor.videoPath);
            topic.anchor.video.setLoopState(OF_LOOP_NONE); // Play anchor once

            // Load footage
            for (const auto& footageJson : topicJson["footage"]) {
                Clip clip;
                clip.videoPath = footageJson["video_path"];
                clip.description = footageJson["description"];
                clip.video.load(clip.videoPath);
                clip.video.setLoopState(OF_LOOP_NONE); // Don't loop by default
                topic.footage.push_back(clip);
            }

            topics.push_back(topic);
        }
        


        // Pick a random topic to start with
        selectRandomTopic();
        
      //  loadSplitScreenClips();

    }
    
    
    ofJson splitJson = ofLoadJson("splitscreen.json");

        for (const auto& entry : splitJson["splitScreens"]) {
            SplitScreenClip clip;
            clip.id = entry["id"];
            clip.file = entry["file"];
            clip.hasAudio = entry["hasAudio"];

            clip.video.load("videos/" + clip.file);
            clip.video.setLoopState(OF_LOOP_NORMAL);
            clip.video.stop();  // Stop them initially
           // clip.video.setVolume(0.0f); // ✅ Mute the clip

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

    if (!playingAnchor) {
        currentTopic->footage[currentFootageIndex].video.draw(0, 0, halfWidth, height);
    } else {
        currentTopic->anchor.video.draw(0, 0, halfWidth, height);
    }

    if (splitScreenMode && !splitScreenClips.empty()) {
        splitScreenClips[currentSplitIndex].video.update();
    }

    splitScreenClips[currentSplitIndex].video.draw(halfWidth, 0, halfWidth, height);
    
    ofLog() << "Drawing split screen: Left = " << (playingAnchor ? "anchor" : "footage")
            << ", Right = " << splitScreenClips[currentSplitIndex].file;

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
        
        else if (key == 's') {
            if (!playingAnchor) {
                splitScreenMode = !splitScreenMode;
                ofLog() << "Split screen toggled. Now: " << (splitScreenMode ? "ON" : "OFF");

                if (splitScreenMode && !splitScreenClips.empty()) {
                    currentSplitIndex = 0; // or random index if you want
                    splitScreenClips[currentSplitIndex].video.play();
                    playCurrentFootage(); // ensure main footage is also playing
                } else {
                    for (auto& clip : splitScreenClips) {
                        clip.video.stop();
                    }
                }
            } else {
                ofLog() << "Can't toggle split screen during anchor playback.";
            }
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
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(currentTopic->footage.begin(), currentTopic->footage.end(), g);
    playCurrentFootage();
}

void ChronologyManager::playCurrentFootage() {
    for (auto& clip : currentTopic->footage) {
        clip.video.stop();
    }
    currentTopic->footage[currentFootageIndex].video.play();
    isLooping = false; // Reset looping
    ofLog() << "Playing footage: " << currentTopic->footage[currentFootageIndex].videoPath;
}

void ChronologyManager::startLooping() {
    float currentTime = currentTopic->footage[currentFootageIndex].video.getPosition() *
                        currentTopic->footage[currentFootageIndex].video.getDuration();
    loopStartTime = std::max(0.0f, currentTime - loopDuration); // Ensure we don't go below 0
    loopEndTime = currentTime;
    isLooping = true;
    ofLog() << "Started looping from " << loopStartTime << "s to " << loopEndTime << "s";
}

void ChronologyManager::stopLooping() {
    isLooping = false;
    loopStartTime = 0;
    loopEndTime = 0;
    ofLog() << "Exited the loop.";
}

void ChronologyManager::newMidiMessage(ofxMidiMessage& message) {
    midiMessage = message; // Store the incoming message for debugging
    
    if (currentTopic && !playingAnchor) {
        // Handle jogwheel (Controller #25)
        if (message.status == MIDI_CONTROL_CHANGE && message.control == 25) {
            static bool jogwheelSpinning = false; // Track if jogwheel is actively spinning
            static bool jogwheelReversed = false; // Track anti-clockwise spin state
            static unsigned long lastMovementTime = 0; // Track the last jogwheel activity time
            const int movementThresholdMin = 5;  // Minimum value for jogwheel movement
            const int movementThresholdMax = 10; // Maximum value for jogwheel movement
            const int reverseMovementThresholdMin = 110;  // Updated threshold for anti-clockwise movement
            const int reverseMovementThresholdMax = 124;  // Maximum value for anti-clockwise movement
            
            if (message.value >= movementThresholdMin && message.value <= movementThresholdMax) {
                // Jogwheel is actively spinning clockwise
                jogwheelSpinning = true;
                jogwheelReversed = false; // Reset reverse flag when clockwise spinning
                lastMovementTime = ofGetElapsedTimeMillis(); // Update last activity time
            } else if (message.value >= reverseMovementThresholdMin && message.value <= reverseMovementThresholdMax) {
                // Jogwheel is spinning anti-clockwise
                jogwheelReversed = true;
                jogwheelSpinning = false; // Reset clockwise spinning flag
                lastMovementTime = ofGetElapsedTimeMillis(); // Update last activity time
            } else if (jogwheelSpinning && message.value == 1) {
                // Jogwheel has been released
                unsigned long now = ofGetElapsedTimeMillis();
                if (now - lastMovementTime > 100) { // 100ms debounce
                    // Jogwheel released after clockwise spin
                    jogwheelSpinning = false;
                    // Advance to the next clip
                    currentFootageIndex = (currentFootageIndex + 1) % currentTopic->footage.size();
                    playCurrentFootage();
                    ofLog() << "Jogwheel released (clockwise): Advancing to next clip";
                }
            } else if (jogwheelReversed && message.value == 127) {
                // Jogwheel has been released after anti-clockwise spin
                unsigned long now = ofGetElapsedTimeMillis();
                if (now - lastMovementTime > 100) { // 100ms debounce
                    // Jogwheel released after anti-clockwise spin
                    jogwheelReversed = false;
                    // Go back to the previous clip
                    currentFootageIndex = (currentFootageIndex - 1 + currentTopic->footage.size()) % currentTopic->footage.size();
                    playCurrentFootage();
                    ofLog() << "Jogwheel released (anti-clockwise): Going back to previous clip";
                }
            }
        }
        
        if (currentTopic && !playingAnchor) {
            // Handle jogwheel (Controller #25)
            if (message.status == MIDI_CONTROL_CHANGE && message.control == 24) {
                // Clockwise jogwheel values (e.g., low range: 2–10)
                if (message.value >= 5 && message.value <= 10) {
                    if (!isLooping) {
                        startLooping(); // Start looping
                    }
                    ofLog() << "Jogwheel turned clockwise: Looping enabled.";
                }
                
                // Anti-clockwise jogwheel values (e.g., high range: 110–124)
                if (message.value >= 110 && message.value <= 124) {
                    if (isLooping) {
                        stopLooping(); // Stop looping
                    }
                    ofLog() << "Jogwheel turned anti-clockwise: Looping disabled.";
                }
            }
        }
        // Handle topic navigation (Controller #26)
        if (message.status == MIDI_CONTROL_CHANGE && message.control == 26) {
            if (message.value == 1) {
                // Advance to the next topic
                selectRandomTopic();
                ofLog() << "Controller #26: Advanced to the next topic.";
            } else if (message.value == 127) {
                // Go back to the previous topic
                static int previousTopicIndex = -1;
                if (previousTopicIndex >= 0) {
                    currentTopic = &topics[previousTopicIndex];
                    currentTopic->anchor.video.play();
                    playingAnchor = true;
                    ofLog() << "Controller #26: Returned to the previous topic.";
                } else {
                    ofLog() << "Controller #26: No previous topic to return to.";
                }
            }
        }
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
