//
//  StaticEffect.cpp
//  ccProject3
//
//  Created by Latifah Dickson on 17/11/2024.
//

#include "Static.hpp"

StaticEffect::StaticEffect() {
    isStaticActive = false;
    videoXPos = 0.0f;  // Horizontal position of the video
    speed = 2.0f;  // Speed of movement (adjust as needed)
}

void StaticEffect::setup() {
//    staticOverlay.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);  // FBO for static noise overlay
}

void StaticEffect::update() {
    if (isStaticActive) {
        // Update horizontal position for the carousel effect
        videoXPos += speed;  // Move the video horizontally
        if (videoXPos > ofGetWidth()) {
            videoXPos = 0;  // Reset to create the endless loop effect
        }

//        // Generate static noise overlay
//        generateStatic();
    }
}

void StaticEffect::apply(ofVideoPlayer& video, float x, float y, float width, float height) {
    if (isStaticActive) {
        // Apply horizontal shift to simulate carousel effect
        ofPushMatrix();
        ofTranslate(-videoXPos, 0);  // Move the video horizontally
        video.draw(x, y, width, height);  // Draw the video frame
        ofPopMatrix();

//        // Draw the static overlay on top
//        ofSetColor(255);
//        staticOverlay.draw(0, 0, ofGetWidth(), ofGetHeight());
//    } else {
        // If static effect is not active, just draw the video normally
        video.draw(x, y, width, height);
    }
}

void StaticEffect::toggleStatic(bool active) {
    isStaticActive = active;  // Toggle static effect
}

void StaticEffect::generateStatic() {
//    staticOverlay.begin();
//    ofClear(0, 0, 0, 255);  // Clear the FBO
//
//    // Generate random noise for static effect
//    for (int y = 0; y < ofGetHeight(); y++) {
//        for (int x = 0; x < ofGetWidth(); x++) {
//            if (ofRandom(1.0) > 0.5) {
//                ofSetColor(255);  // White pixel for noise
//            } else {
//                ofSetColor(0);    // Black pixel for noise
//            }
//            ofDrawRectangle(x, y, 1, 1);  // Draw each pixel for the noise
//        }
//    }
//
//    staticOverlay.end();
}
