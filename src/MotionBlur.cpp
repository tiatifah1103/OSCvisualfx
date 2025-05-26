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

void MotionBlur::update(const ofTexture &videoTexture) {
    
    // Skip processing if texture isn't ready
     if (!videoTexture.isAllocated()) return;
    
    int downsampleFactor = 4;

    // Converts texture to pixels
    ofPixels currentPixels;
    ofImage tempImage;
    tempImage.allocate(videoTexture.getWidth(), videoTexture.getHeight(), OF_IMAGE_COLOR_ALPHA);
    videoTexture.readToPixels(tempImage.getPixels());
    currentPixels = tempImage.getPixels();

    // Allocates distorted frame buffer
    if (!distortedFrame.isAllocated()) {
        distortedFrame.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    }
    distortedFrame.begin();
    ofClear(0, 0, 0, 0);
    
    // If there's a previous frame to compare

    if (previousFramePixels.getWidth() != 0) {
        int width = currentPixels.getWidth();
        int height = currentPixels.getHeight();
        
        // Loop through the pixels with a step size of downsampleFactor for efficiency
        for (int y = 0; y < height; y += downsampleFactor) {
            for (int x = 0; x < width; x += downsampleFactor) {
                // Get colours from the current and previous frame at the same pixel
                ofColor currentColor = currentPixels.getColor(x, y);
                ofColor previousColor = previousFramePixels.getColor(x, y);
                
                // Calculates how different the current pixel is from the previous
                float difference = colorDistance(currentColor, previousColor);
                
                // Maps the color difference to a stretch amount (larger difference = more blur/stretch)
                float stretch = ofMap(difference, 0, 255, 0, stretchAmount);

                // If the pixels have moved / changed enough stretch them
                if (stretch > 0) {
                    int stretchOffset = stretch;
                    // Calculate left and right stretch positions - clamped to image bounds
                    int leftX = ofClamp(x - stretchOffset, 0, width - 1);
                    int rightX = ofClamp(x + stretchOffset, 0, width - 1);

                    // Blend the current and previous colors for a smooth visual transition
                    ofColor blendColor = currentColor.getLerped(previousColor, 0.5f);
                    ofSetColor(blendColor);

                    // Draws rectangles at left and right positions to simulate motion blur
                    ofDrawRectangle(leftX, y, downsampleFactor, downsampleFactor);
                    ofDrawRectangle(rightX, y, downsampleFactor, downsampleFactor);
                } else {
                    // If there's no difference draw current pixel normally
                    ofSetColor(currentColor);
                    ofDrawRectangle(x, y, downsampleFactor, downsampleFactor);
                }
            }
        }
    }

    // Finish drawing to the framebuffer
    distortedFrame.end();

    // Accumulate
    accumulationBuffer.begin();
    ofSetColor(255, 255, 255, blendFactor * 255);
    distortedFrame.draw(0, 0, ofGetWidth(), ofGetHeight());
    accumulationBuffer.end();

    // Stores the current frame pixels as previous for next update
    previousFramePixels = currentPixels;
}


void MotionBlur::apply(ofFbo& fbo) {
    // Create temporary FBO
    ofFbo tempFbo;
    tempFbo.allocate(fbo.getWidth(), fbo.getHeight());
    
    // Apply motion blur to the input FBO
    tempFbo.begin();
    ofClear(0, 0, 0, 255);
    accumulationBuffer.draw(0, 0);
    tempFbo.end();
    
    // Draw result back to original FBO
    fbo.begin();
    ofClear(0, 0, 0, 255);
    tempFbo.draw(0, 0);
    fbo.end();
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

void MotionBlur::resetAllParameters() {
    blendFactor = 0.0f;
    stretchAmount = 0.0f;
    clear();
}

