//want to refine this to make it seem less choppy/laggy and more somthing else???
//needs a motion blur/smoothing

#include "StepPrint.hpp"

StepPrinting::StepPrinting() {
    stepInterval = 20;         // Interval to capture every 10th frame
    frameCounter = 0;
    maxStoredFrames = 1;       // Adjust to see more frames in sequence
}

void StepPrinting::setup(int _stepInterval) { // sets up the intervals for capturing frames
    stepInterval = _stepInterval;
    storedFrames.clear(); //Clears the frame buffer, preparing the effect for fresh use.
}

void StepPrinting::update(const ofTexture &videoTexture) {
    frameCounter++; // tracks when to capture frames based on the step interval

    // Check if the texture is valid
    if (videoTexture.isAllocated()) {
        // Capture frame at the specified interval
        if (frameCounter % stepInterval == 0) {
            ofImage frame; // temp image object to hold the current frame
            frame.allocate(videoTexture.getWidth(), videoTexture.getHeight(), OF_IMAGE_COLOR_ALPHA);
            videoTexture.readToPixels(frame.getPixels()); // copies pixel data from the texture to the image

            // Store the new frame and ensure the vector size is managed
            storedFrames.push_back(frame); // Adds the captured frame to the `storedFrames` vector

            // Ensures storedFrames doesn't exceed maxStoredFrames
            if (storedFrames.size() > maxStoredFrames) {
                storedFrames.erase(storedFrames.begin());
            }

            // Debugging output
            ofLog() << "Captured frame. Total stored frames: " << storedFrames.size();
        }
    }
}

    void StepPrinting::apply(ofVideoPlayer &video, float x, float y, float width, float height) {
        ofSetColor(255);
        ofEnableBlendMode(OF_BLENDMODE_ADD); // Additive blending for smoother transitions

        // Blend frames smoothly
        for (size_t i = 0; i < storedFrames.size(); ++i) {
            float alpha = 255 * (1.0f - float(i) / maxStoredFrames); // Gradually fade frames
            ofSetColor(255, 255, 255, alpha);
            storedFrames[i].draw(x, y, width, height);
        }

        ofDisableBlendMode();
        ofSetColor(255); // Resets to full opacity
    }

void StepPrinting::clear() {
    storedFrames.clear();
}

void StepPrinting::setStepInterval(int interval) {
    stepInterval = interval;  // allows external control of the step interval
}

void StepPrinting::setMaxStoredFrames(int maxFrames) {
    maxStoredFrames = maxFrames;  // allows external control of stored frames
}
