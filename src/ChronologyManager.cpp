
#include "ChronologyManager.hpp"

bool isLooping = false;
float loopStartTime = 0;          // Time when the loop starts
float loopDuration = 6.0f;
bool isVideoLooping = false;      //flag to make sure only one video is looping at a time
float loopStartPosition = 0;      // The starting position of the loop (secs)

void ChronologyManager::setup() {
    // Load JSON (same as before)
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

void ChronologyManager::update() {
    if (currentTopic) {
        // Handles loop timing
        if (isLooping) {
            float elapsedTime = ofGetElapsedTimef() - loopStartTime;
            if (elapsedTime >= loopDuration) {
                // Stops looping after 6 seconds
                isLooping = false;
                currentTopic->footage[currentFootageIndex].video.setFrame(loopStartPosition * currentTopic->footage[currentFootageIndex].video.getTotalNumFrames() / currentTopic->footage[currentFootageIndex].video.getDuration());
                currentTopic->footage[currentFootageIndex].video.play();
            }
        }

        if (playingAnchor) {
            currentTopic->anchor.video.update();
            if (currentTopic->anchor.video.getIsMovieDone()) {
                // Stop anchor playback
                currentTopic->anchor.video.stop();

                //When  anchor completed it switches to footage
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

// Key presses handling for clips
void ChronologyManager::keyPressed(int key) {
    if (currentTopic && !playingAnchor) {
        if (key == OF_KEY_RIGHT) {
            // Skips to next footage
            currentFootageIndex = (currentFootageIndex + 1) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == OF_KEY_LEFT) {
            // Skips to previous footage
            currentFootageIndex = (currentFootageIndex - 1 + currentTopic->footage.size()) % currentTopic->footage.size();
            playCurrentFootage();
        } else if (key == 'n') {
            // Skips to a new random topic
            selectRandomTopic();
        } else if (key == 'l') {
            // Triggers looping on the current footage for 6 seconds
            isLooping = true;
            loopStartTime = ofGetElapsedTimef();
            loopStartPosition = currentTopic->footage[currentFootageIndex].video.getPosition();  // Gets current video position
            currentTopic->footage[currentFootageIndex].video.setFrame(loopStartPosition * currentTopic->footage[currentFootageIndex].video.getTotalNumFrames() / currentTopic->footage[currentFootageIndex].video.getDuration());
            currentTopic->footage[currentFootageIndex].video.play();
        }
    }
}

void ChronologyManager::selectRandomTopic() {
    // Stops all videos from the current topic before switching
    if (currentTopic) {
        // Stops the anchor video
        currentTopic->anchor.video.stop();

        // Stops all footage videos
        for (auto& clip : currentTopic->footage) {
            clip.video.stop();
        }
    }

    // Selects a new random topic that is different from the current one
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
    // Use a random number generator
    std::random_device rd; // Creates a random device to get a seed
    std::mt19937 g(rd()); // Mersenne Twister engine creates more random numbers based on the seed.


    std::shuffle(currentTopic->footage.begin(), currentTopic->footage.end(), g); // Mixes up the order of all the videos stored in the current topic.
    playCurrentFootage();
}

void ChronologyManager::playCurrentFootage() {
    // Stops the anchor video
    currentTopic->anchor.video.stop();

    // Stops all other footage to prevent overlapping audio
    for (auto& clip : currentTopic->footage) {
        clip.video.stop();
    }

    // Plays the current footage
    currentTopic->footage[currentFootageIndex].video.play();
    ofLog() << "Playing footage: " << currentTopic->footage[currentFootageIndex].videoPath;
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

