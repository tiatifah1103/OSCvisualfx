#pragma once
#include "ofMain.h"

class FisheyeLens {
public:
    FisheyeLens();
    void setup(float _distortionStrength);
    void update(const ofTexture &videoTexture);
    void apply(float x, float y, float width, float height);
    
    void setDistortionStrength(float strength);
    float getDistortionStrength() const;
    
    // Enhanced bass response controls
    void setBassLevel(float level); // 0-1 range
    void setPulseFrequency(float freq); // How often pulses occur at max bass
    void setMaxDistortion(float max); // Maximum possible distortion
    
    void reset();
    
private:
    ofFbo distortedFrame;
    float distortionStrength; // Added missing member variable
    float baseDistortion;
    float currentDistortion;
    float distortionSmoothing;
    
    // Bass response system
    float bassLevel; // Current normalized bass level (0-1)
    float maxDistortion; // Maximum possible distortion
    float pulseFrequency; // How often pulses occur at max bass
    
    // Pulsing system
    float timeCounter;
    float nextPulseTime;
    float pulseDuration;
    float currentPulseStrength;
    
    // Movement system
    ofVec2f currentOffset;
    ofVec2f targetOffset;
    float movementSpeed;
    float movementAmount;
    
    // Vibration system
    float vibrationAmount;
    float vibrationSpeed;
    
    // Internal methods
    void updatePulsing(float deltaTime);
    void updateMovement(float deltaTime);
    float calculateFinalDistortion(); // Changed return type from void to float
};
