// GlitchEffect.cpp
#include "Glitch.hpp"

void GlitchEffect::setup() {
    glitchAmount = 0.5f;
    glitchCounter = 0;
    magnifierSize = 100.0f; // Using size for square dimensions
    magnifierStrength = 1.5f;
    lastGlitchTime = 0;
    glitchInterval = 100; // milliseconds between major glitches
}

void GlitchEffect::update(const ofTexture& tex) {
    // Allocate if needed
    if (!fbo.isAllocated() || fbo.getWidth() != tex.getWidth() || fbo.getHeight() != tex.getHeight()) {
        fbo.allocate(tex.getWidth(), tex.getHeight());
        buffer.allocate(tex.getWidth(), tex.getHeight(), OF_IMAGE_COLOR);
    }
    
    // Copy texture to FBO
    fbo.begin();
    ofClear(0, 0, 0, 255);
    tex.draw(0, 0);
    fbo.end();
    
    // Read from FBO to buffer
    fbo.readToPixels(buffer.getPixels());
    buffer.update();
    
    // Apply effects to FBO
    fbo.begin();
    ofClear(0, 0, 0, 255);
    
    // apply some subtle glitching
    applyGlitchEffect(buffer, 0.3f * glitchAmount); // Subtle continuous glitch
    
    //stronger random glitches periodically
    if (ofGetElapsedTimeMillis() - lastGlitchTime > glitchInterval) {
        lastGlitchTime = ofGetElapsedTimeMillis();
        glitchInterval = ofRandom(50, 500); // Random interval for next glitch
        
        // Strong glitch
        applyGlitchEffect(buffer, glitchAmount);
        
        // Random square magnifiers (1-3 at a time)
        int numMagnifiers = ofRandom(1, 4);
        for (int i = 0; i < numMagnifiers; i++) {
            applySquareMagnifierEffect(buffer);
        }
    }
    
    buffer.draw(0, 0);
    fbo.end();
}

void GlitchEffect::applyGlitchEffect(ofImage& img, float strength) {
    ofPixels& pixels = img.getPixels();
    int width = img.getWidth();
    int height = img.getHeight();
    
    // Modifies the channel shifting to use colorShiftAmount
    int shiftR = ofRandom(-15, 15) * strength * colorShiftAmount;
    int shiftG = ofRandom(-15, 15) * strength * colorShiftAmount;
    int shiftB = ofRandom(-15, 15) * strength * colorShiftAmount;
    
    
    // Random scanline jitter
    int jitterAreaHeight = ofRandom(10, 100) * strength;
    int jitterY = ofRandom(height - jitterAreaHeight);
    
    // Loop through each pixel
    for (int y = 0; y < height; y++) {
        
        for (int x = 0; x < width; x++) {
            // Calculates new positions for each color channel with clamping
            int srcXR = ofClamp(x + shiftR, 0, width-1);
            int srcXG = ofClamp(x + shiftG, 0, width-1);
            int srcXB = ofClamp(x + shiftB, 0, width-1);
            
            ofColor c = pixels.getColor(x, y);
            if (y > jitterY && y < jitterY + jitterAreaHeight) {
                // More intense glitch in jitter area
                c.r = pixels.getColor(srcXR, y).r;
                c.g = pixels.getColor(ofClamp(x + shiftG*2, 0, width-1), y).g;
                c.b = pixels.getColor(srcXB, y).b;
            } else {
                // Subtler glitch outside
                if (ofRandomf() < 0.3f * strength) {
                    c.r = pixels.getColor(srcXR, y).r;
                }
                if (ofRandomf() < 0.3f * strength) {
                    c.g = pixels.getColor(srcXG, y).g;
                }
            }
            pixels.setColor(x, y, c);
        }
    }
    
    // Random block copies to create digital tearing
    for (int i = 0; i < 5 * strength; i++) {
        int blockW = ofRandom(10, 100);
        int blockH = ofRandom(5, 30);
        int srcX = ofRandom(width - blockW);
        int srcY = ofRandom(height - blockH);
        int destX = ofRandom(width - blockW);
        int destY = ofRandom(height - blockH);
        
        for (int y = 0; y < blockH; y++) {
            for (int x = 0; x < blockW; x++) {
                // Copy pixel from source to destination
                pixels.setColor(destX + x, destY + y, pixels.getColor(srcX + x, srcY + y));
            }
        }
    }
    
    img.update();
}

void GlitchEffect::applySquareMagnifierEffect(ofImage& img) {
    ofPixels& pixels = img.getPixels();
    int width = img.getWidth();
    int height = img.getHeight();
    
    // Random square properties
    float size = ofRandom(50, 200);                    // Size of square effect
    ofVec2f center(ofRandom(width), ofRandom(height)); // Random center of square
    float strength = ofRandom(1.2f, 2.5f);             // Strength of zoom
    bool zoomIn = ofRandomf() > 0.5f;                  // Random zoom direction
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Check if the current pixel is inside the distortion square
            if (abs(x - center.x) < size/2 && abs(y - center.y) < size/2) {
                // Calculates distance from center
                float dx = (x - center.x) / (size/2);
                float dy = (y - center.y) / (size/2);
                
                // Square distortion based on max axis distance
                float distortion = max(abs(dx), abs(dy));
                float factor = zoomIn ?
                    (1.0 - strength * distortion) : // Zoom in
                    (1.0 + strength * (1.0 - distortion)); // Zoom out
                
                // Calculates new sample coordinates
                int srcX = center.x + (x - center.x) * factor;
                int srcY = center.y + (y - center.y) * factor;
                
                // Clamp coordinates to image bounds
                     srcX = ofClamp(srcX, 0, width - 1);
                     srcY = ofClamp(srcY, 0, height - 1);
                     
                     // Set pixel to distorted sample
                     pixels.setColor(x, y, pixels.getColor(srcX, srcY));
            }
        }
    }
    
    img.update();
}

void GlitchEffect::draw(float x, float y, float w, float h) {
    fbo.draw(x, y, w, h);
}

void GlitchEffect::reset() {
    magnifierStrength = 1.0f;
    glitchAmount = 0.0f;
    
    midRangeAmount = 0.0f;
    highRangeAmount = 0.0f;
    colorShiftAmount = 0.0f;
}

void GlitchEffect::setGlitchAmount(float amount) {
    glitchAmount = ofClamp(amount, 0.0f, 1.0f);
}

void GlitchEffect::setMagnifierSize(float size) {
    magnifierSize = ofClamp(size, 10.0f, 500.0f); // Constrain to reasonable values
}

void GlitchEffect::setMagnifierStrength(float strength) {
    magnifierStrength = ofClamp(strength, 0.1f, 5.0f); // Constrain to reasonable values
}

void GlitchEffect::setMidRangeAmount(float amount) {
    midRangeAmount = ofClamp(amount, 0.0f, 1.0f);
    // Map mids to magnifier strength and distortion
    magnifierStrength = ofMap(midRangeAmount, 0.0f, 1.0f, 1.0f, 3.0f);
    setMagnifierSize(ofMap(midRangeAmount, 0.0f, 1.0f, 50.0f, 200.0f));
}

void GlitchEffect::setHighRangeAmount(float amount) {
    highRangeAmount = ofClamp(amount, 0.0f, 1.0f);
    // Map highs to color glitch intensity
    colorShiftAmount = ofMap(highRangeAmount, 0.0f, 1.0f, 0.0f, 2.0f);
}
