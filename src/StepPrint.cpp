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

void StepPrinting::update(ofVideoPlayer &video) {
    frameCounter++; //tracks when to capture frames based on the step interval

    // Check if the video is loaded and has valid pixels
    if (video.isLoaded() && video.getPixels().isAllocated()) {
        // Capture frame at the specified interval
        if (frameCounter % stepInterval == 0) {
            ofImage frame; // temp image object to hold the current frame
            frame.setFromPixels(video.getPixels()); //copies pixel data from the video to the image

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
    ofEnableBlendMode(OF_BLENDMODE_ADD); //additive blending


    // loops through all stored frames and draws each one in sequence
    for (const auto& frame : storedFrames) {
        frame.draw(x, y, width, height); // Draws the frame at the specified position and size.
    }
    
    ofDisableBlendMode();
    ofSetColor(255);
    
    // Draw each stored frame with decreasing opacity
      for (int i = 0; i < storedFrames.size(); ++i) {
          float opacity = 255 * (1.0f - float(i) / maxStoredFrames); // calculates opacity based on frame index.
          ofSetColor(255, 255, 255, opacity);  // sets the color with decreasing opacity.
          storedFrames[i].draw(x, y, width, height);   // draws the frame with the calculated opacity.
    
      }

      ofSetColor(255); // Reset to full opacity after drawing

}

void StepPrinting::clear() {
    storedFrames.clear();
}
