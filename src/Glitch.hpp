#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

class GlitchEffect {
public:
    struct GlitchElement {
        ofVec2f position;
        float size;
        float strength;
        float lifetime;
        float maxLifetime;
        bool isMagnifier;
    };

    void setup();
    void update(const ofTexture& tex);
    void draw(float x, float y, float w, float h);
    void reset();
    
    // Parameter setters
    void setGlitchAmount(float amount);
    void setMagnifierSize(float size);
    void setMagnifierStrength(float strength);
    void setGlitchInterval(int intervalMs);
    
    // Parameter getters
    float getGlitchAmount() const;
    float getMagnifierSize() const;
    float getMagnifierStrength() const;
    int getGlitchInterval() const;
    
    // Add these new methods
    void setMidRangeAmount(float amount); // Controls magnifiers and distortion
    void setHighRangeAmount(float amount); // Controls colour glitches
    void applyPersistentMagnifier(const GlitchElement& glitch, float strength);

private:
    void applyGlitchEffect(ofImage& img, float strength);
    void applySquareMagnifierEffect(ofImage& img);
    
    ofFbo fbo;                      // Framebuffer for processing
    ofImage buffer;                 // Image buffer for pixel manipulation
    
    // Effect parameters
    float glitchAmount;             // Overall intensity of glitch effects
    float magnifierSize;            // Base size of square magnifier effects
    float magnifierStrength;        // Intensity of magnification effects
    
    // Timing control
    int lastGlitchTime;             // Last time a major glitch occurred
    int glitchInterval;             // Base interval between glitches
    int glitchCounter;              // Counter for glitch variations
    
    float midRangeAmount;
    float highRangeAmount;
    float colorShiftAmount;
    
    // Add these for the border
    ofColor magnifierBorderColor;
    int magnifierBorderSize;
    
    vector<GlitchElement> activeGlitches;
    float baseGlitchDuration;
    float magnifierDuration;
};
