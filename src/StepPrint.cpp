
#include "StepPrint.hpp"

StepPrinting::StepPrinting() {
    stepInterval = 30;          // Capture a frame every 30 frames (adjustable)
    frameCounter = 0;          // Used to track how many frames have passed
    maxStoredFrames = 10;      // Limit of how many frames to keep in memory for blending
}

void StepPrinting::setup(int _stepInterval) { // sets up the intervals for capturing frames
    stepInterval = _stepInterval;
    storedFrames.clear(); // Clear any previously stored frames
}

void StepPrinting::update(const ofTexture &videoTexture) {
    frameCounter++; // Increment frame counter every update
    if (!videoTexture.isAllocated()) return;

    // Only capture a frame at every stepInterval frames
    if (frameCounter % stepInterval == 0) {
        ofFbo frame; // Create a new frame buffer object to store this frame
        frame.allocate(videoTexture.getWidth(), videoTexture.getHeight());
        frame.begin();
        videoTexture.draw(0, 0); // Draw current video texture into the FBO
        frame.end();
        
        storedFrames.push_back(frame);
        
        // Limits number of stored frames to avoid memory buildup
        while (storedFrames.size() > maxStoredFrames) {
            storedFrames.erase(storedFrames.begin());
        }
    }
}


void StepPrinting::apply(ofFbo& fbo) {
    if (!isActive() || storedFrames.empty()) return;
    
    fbo.begin();
    //additive blending to create ghosting/motion trail effect
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    // Loop through all stored frames and draw them with decreasing alpha
    for (size_t i = 0; i < storedFrames.size(); ++i) {
        // Fade strength is based on how old the frame is
        float alpha = 255 * powf(0.5f, i / (float)maxStoredFrames * fadeStrength);
        ofSetColor(255, alpha);
        storedFrames[i].getTexture().draw(0, 0, fbo.getWidth(), fbo.getHeight());
    }
    ofDisableBlendMode(); // Reset blend mode to default
    fbo.end();
}
bool StepPrinting::isActive() const {
    // Effect is active if more than one frame is stored and fading is enabled
    return (maxStoredFrames > 1 && fadeStrength > 0.0f);
}



void StepPrinting::clear() {
    storedFrames.clear();
}

void StepPrinting::setStepInterval(int interval) {
    // Sets step interval but clamp it to valid range to prevent bugs
    stepInterval = ofClamp(interval, 1, 100); // allows external control of the step interval
}

void StepPrinting::setMaxStoredFrames(int maxFrames, bool forceClear) {
    // Sets the maximum number of frames to keep and clear
    maxStoredFrames = ofClamp(maxFrames, 1, 100);
    // Clears if forced or new limit is lower than current stored frame count
    if (forceClear || maxStoredFrames < storedFrames.size()) {
        storedFrames.clear();
        frameCounter = 0;
    }
}

void StepPrinting::setFadeStrength(float strength) {
    // Adjusts how quickly older frames fade out
    fadeStrength = ofClamp(strength, 0.0f, 3.0f);
}

void StepPrinting::clearFrames() {
    // Manual frame buffer clear
    storedFrames.clear();
}

void StepPrinting::resetAllParameters() {
    stepInterval = 1;  // Reset to minimum value
    maxStoredFrames = 1;
    fadeStrength = 0.0f;
    storedFrames.clear();
    frameCounter = 0;
}


