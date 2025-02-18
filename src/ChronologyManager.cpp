#include "ChronologyManager.hpp"

// Setup function
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
                clip.video.setLoopState(OF_LOOP_NORMAL); // Loop for footage
                topic.footage.push_back(clip);
            }

            topics.push_back(topic);
        }

        // Picks a random topic to start with
        selectRandomTopic();
    }
}

// Update function
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
        if (playingAnchor) {
            currentTopic->anchor.video.draw(0, 0, ofGetWidth(), ofGetHeight());
        } else {
            currentTopic->footage[currentFootageIndex].video.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
    }
}


void ChronologyManager::keyPressed(int key) {
    if (currentTopic && !playingAnchor) {
        if (key == OF_KEY_RIGHT) {
            currentFootageIndex = (currentFootageIndex + 1) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == OF_KEY_LEFT) {
            currentFootageIndex = (currentFootageIndex - 1 + currentTopic->footage.size()) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == 'n') {
            selectRandomTopic();
        } else if (key == 'l') {
            if (isLooping) {
                stopLooping();
            } else {
                startLooping();
            }
        }
    }
}

void ChronologyManager::selectRandomTopic() {
    // Stops all videos from the current topic before switching
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
