#include "MotionBlur.hpp"
#include <cmath>  // For sqrt and pow functions - which is used to calcultae euclidean distance between colours

MotionBlur::MotionBlur(){
    blendFactor = 0.9f;  // determines how much of the current frame blends into the accumulation buffer
    stretchAmount = 0.2f;  // threshold to decide when to apply stretching effects based on motion intensity.
}

void MotionBlur::setup(float _blendFactor, float _stretchAmount){
    blendFactor = _blendFactor;
    stretchAmount = _stretchAmount;
    
    // Allocate an accumulation buffer
    accumulationBuffer.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    accumulationBuffer.begin();
    ofClear(0, 0, 0, 0); // Clear the buffer
    accumulationBuffer.end();
}

float MotionBlur::colorDistance(const ofColor &color1, const ofColor &color2) {
    // Calculate the Euclidean distance between two colors
    float rDiff = color1.r - color2.r;
    float gDiff = color1.g - color2.g;
    float bDiff = color1.b - color2.b;
    
    return sqrt(rDiff * rDiff + gDiff * gDiff + bDiff * bDiff);
    
    //this function is used to calculate the euclidean distance between colours in the RGB space - measureing how different 2 colours are which helps detect the motion between frames.
}

void MotionBlur::update(ofVideoPlayer &video) {
    // Get the current frame's pixels
    ofPixels currentPixels = video.getPixels();
    int downsampleFactor = 4; // Process fewer pixels for performance

    // Allocate distorted frame buffer
    distortedFrame.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    distortedFrame.begin();
    ofClear(0, 0, 0, 0); // Clear the buffer before drawing distorted pixels

    if (previousFramePixels.getWidth() != 0) {
        int width = currentPixels.getWidth();
        int height = currentPixels.getHeight();

        // Iterate through pixels with downsampling
        for (int y = 0; y < height; y += downsampleFactor) {
            for (int x = 0; x < width; x += downsampleFactor) {
                // Get the current and previous frame colors
                ofColor currentColor = currentPixels.getColor(x, y);
                ofColor previousColor = previousFramePixels.getColor(x, y);

                // Calculate the difference in color intensity
                float difference = colorDistance(currentColor, previousColor);

                // Stretch or de-stretch pixels based on motion magnitude
                float stretch = ofMap(difference, 0, 255, 0, stretchAmount); // Map motion to stretch

                if (stretch > 0) {
                    // Stretch effect: Draw a stretched rectangle in the motion direction
                    int stretchOffset = stretch; // Amount to stretch in each direction
                    int leftX = ofClamp(x - stretchOffset, 0, width - 1);
                    int rightX = ofClamp(x + stretchOffset, 0, width - 1);

                    // Blend color for stretched pixels
                    ofColor blendColor = currentColor.getLerped(previousColor, 0.5f);
                    ofSetColor(blendColor);

                    // Draw stretched areas on both sides
                    ofDrawRectangle(leftX, y, downsampleFactor, downsampleFactor);
                    ofDrawRectangle(rightX, y, downsampleFactor, downsampleFactor);
                } else {
                    // De-stretch: Draw the original pixel with no stretching
                    ofSetColor(currentColor);
                    ofDrawRectangle(x, y, downsampleFactor, downsampleFactor);
                }
            }
        }
    }

    distortedFrame.end();

    // Accumulate the distorted frame
    accumulationBuffer.begin();
    ofSetColor(255, 255, 255, blendFactor * 255);
    video.draw(0, 0, ofGetWidth(), ofGetHeight()); // Draw the original video
    distortedFrame.draw(0, 0, ofGetWidth(), ofGetHeight()); // Overlay the distorted frame
    accumulationBuffer.end();

    // Store the current frame for the next update
    previousFramePixels = currentPixels;
}

void MotionBlur::apply(ofVideoPlayer &video, float x, float y, float width, float height){
    // Draw the accumulated motion blur from the buffer
    ofSetColor(255);
    accumulationBuffer.draw(x, y, width, height);
}

void MotionBlur::clear(){
    // Clear the accumulation buffer
    accumulationBuffer.begin();
    ofClear(0, 0, 0, 0);
    accumulationBuffer.end();
}

// Set the blend factor
void MotionBlur::setBlendFactor(float factor) {
    blendFactor = factor;
}

// Get the blend factor
float MotionBlur::getBlendFactor() const {
    return blendFactor;
}

// Set the stretch amount
void MotionBlur::setStretchAmount(float amount) {
    stretchAmount = amount;
}

// Get the stretch amount
float MotionBlur::getStretchAmount() const {
    return stretchAmount;
}
