// FisheyeLens.cpp
#include "FisheyeLens.hpp"


FisheyeLens::FisheyeLens()
: distortionStrength(0.5f), // Base strength of fisheye distortion
  baseDistortion(0.0f),     // User-defined baseline distortion
  currentDistortion(0.0f),  // Smoothly interpolated distortion value
  distortionSmoothing(0.1f), // Smoothing factor for distortion changes
  bassLevel(0.0f),     // Real-time bass level input
  maxDistortion(2.5f),  // Max distortion level
  pulseFrequency(2.0f),   // How often pulses happen
  timeCounter(0.0f),    // Global time counter for pulse timing
  nextPulseTime(0.0f), // Time of the next expected pulse
  pulseDuration(0.0f),  // Duration of each pulse
  currentPulseStrength(0.0f), // Strength of the active pulse
  movementSpeed(1.0f), // How quickly the visual offset moves
  movementAmount(0.0f),   // Intensity of movement (based on bass)
  vibrationAmount(0.0f),  // Shaking amount for jitter effect
  vibrationSpeed(1.0f)    // Speed of vibration oscillation
{
}

void FisheyeLens::setup(float _distortionStrength) {
    baseDistortion = _distortionStrength;
    currentDistortion = 0.0f;
    distortionSmoothing = 0.15f;
    
    // allocates FBO the same size as screen for rendering final distorted image
    distortedFrame.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
}


void FisheyeLens::setBassLevel(float level) {
    bassLevel = ofClamp(level, 0.0f, 1.0f);
    
    // Dynamic adjustments based on bass level
    movementAmount = bassLevel * 50.0f; // More movement at higher bass
    vibrationAmount = bassLevel * 0.2f;
    vibrationSpeed = ofMap(bassLevel, 0.0f, 1.0f, 1.0f, 5.0f);
    
    // More pulsing at higher bass
    pulseFrequency = ofMap(bassLevel, 0.0f, 1.0f, 0.2f, 5.0f);
}

void FisheyeLens::setPulseFrequency(float freq) {
    pulseFrequency = freq;
}

void FisheyeLens::setMaxDistortion(float max) {
    maxDistortion = max;
}

void FisheyeLens::update(const ofTexture &videoTexture) {
    float deltaTime = ofGetLastFrameTime();
    timeCounter += deltaTime;
    
    // Smooth distortion interpolation toward target distortion strength
    currentDistortion += (distortionStrength - currentDistortion) * distortionSmoothing;
    
    // updates all effect systems
    updatePulsing(deltaTime);
    updateMovement(deltaTime);
    
    // Calculate combined distortion with all effects
    float finalDistortion = calculateFinalDistortion();
    
    int width = videoTexture.getWidth();
    int height = videoTexture.getHeight();
    float maxDim = std::max(width, height);
    float scaleX = (float)width / maxDim;
    float scaleY = (float)height / maxDim;

    distortedFrame.begin();
    ofClear(0, 0, 0, 255);

    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);

    int step = 10;
    // Mesh grid with distortion to each point
    for (int y = 0; y < height - step; y += step) {
        for (int x = 0; x < width - step; x += step) {
            for (int dy = 0; dy <= step; dy += step) {
                for (int dx = 0; dx <= step; dx += step) {
                    float srcX = x + dx + currentOffset.x;
                    float srcY = y + dy + currentOffset.y;
                    
                    // Normalise coordinates to [-1, 1] with aspect ratio preserved
                    float nx = ((srcX / width) * 2.0f - 1.0f) / scaleX;
                    float ny = ((srcY / height) * 2.0f - 1.0f) / scaleY;
                    
                    // Calculate radial distance from center
                    float r = sqrt(nx * nx + ny * ny);
                    // Sinusoidal vibration for visual shaking
                    float vibration = vibrationAmount * sin(timeCounter * vibrationSpeed * 10.0f);
                    
                    // Extreme fisheye distortion at high bass
                    float theta = atan(r);
                    float distortedR = (r > 0.0f) ? theta / r : 1.0f;
                    
                    // Apply all distortion effects
                    distortedR = 1.0 + finalDistortion * (distortedR - 1.0) * (1.0 + vibration);
                    
                    // Adds pulse distortion
                    if (currentPulseStrength > 0.0f) {
                        float pulseDistort = currentPulseStrength * 0.5f * sin(r * PI * 2.0f);
                        distortedR += pulseDistort;
                    }
                    
                    // Converts distorted radial coordinates back to screen space
                    float distortedX = nx * distortedR;
                    float distortedY = ny * distortedR;
                    
                    // Bring back to 0 width, 0 height space
                    float u = ((distortedX * scaleX) + 1.0f) * 0.5f * width;
                    float v = ((distortedY * scaleY) + 1.0f) * 0.5f * height;
                    
                    mesh.addVertex(glm::vec3(srcX, srcY, 0));
                    mesh.addTexCoord(glm::vec2(u, v));
                }
            }
            
            // Creates triangles from vertices (2 per grid cell)
            int i = mesh.getNumVertices();
            mesh.addIndex(i - 4);
            mesh.addIndex(i - 3);
            mesh.addIndex(i - 2);
            
            mesh.addIndex(i - 4);
            mesh.addIndex(i - 2);
            mesh.addIndex(i - 1);
        }
    }
    
    videoTexture.bind();
    mesh.draw();
    videoTexture.unbind();
    
    distortedFrame.end();
}

void FisheyeLens::updatePulsing(float deltaTime) {
    // Update current pulse
    if (currentPulseStrength > 0.0f) {
        currentPulseStrength -= deltaTime * 2.0f; // Faster decay
        if (currentPulseStrength < 0.0f) {
            currentPulseStrength = 0.0f;
        }
    }
    
    // Trigger new pulses based on bass level
    if (timeCounter > nextPulseTime && bassLevel > 0.3f) {
        // More frequent and stronger pulses at higher bass
        float pulseIntensity = ofRandom(0.7f, 1.0f) * bassLevel;
        currentPulseStrength = pulseIntensity;
        
        // Schedule next pulse w/ randomness
        float nextPulseDelay = ofRandom(0.5f, 2.0f) / (pulseFrequency * bassLevel);
        nextPulseTime = timeCounter + nextPulseDelay;
        
        // Random pulse duration
        pulseDuration = ofRandom(0.1f, 0.3f) * (1.0f - (bassLevel * 0.5f));
    }
}

void FisheyeLens::updateMovement(float deltaTime) {
    // Move toward target offset
    currentOffset += (targetOffset - currentOffset) * movementSpeed * deltaTime;
    
    // Set new random target when close to current target
    if (currentOffset.distance(targetOffset) < 5.0f && bassLevel > 0.1f) {
        float moveAmount = movementAmount * bassLevel;
        targetOffset.set(
            ofRandom(-moveAmount, moveAmount),
            ofRandom(-moveAmount, moveAmount)
        );
        
        // Faster movement at higher bass
        movementSpeed = ofMap(bassLevel, 0.0f, 1.0f, 0.5f, 3.0f);
    }
}

// Combine all factors to calculate the distortion amount
float FisheyeLens::calculateFinalDistortion() {
    // Base distortion affected by bass level (exponential response)
    float bassDistortion = pow(bassLevel, 3.0f) * maxDistortion;
    
    // Combine with current distortion
    float combined = currentDistortion + bassDistortion;
    
    // Add pulse effect
    combined *= (1.0f + currentPulseStrength * 0.5f);
    
    return combined;
}

void FisheyeLens::apply(float x, float y, float width, float height) {
    ofSetColor(255);
    distortedFrame.draw(x, y, width, height);
}

void FisheyeLens::setDistortionStrength(float strength) {
    distortionStrength = ofClamp(strength, 0.0f, maxDistortion);
}

float FisheyeLens::getDistortionStrength() const {
    return distortionStrength;
}

// Reset distortion effect
void FisheyeLens::reset() {
    setDistortionStrength(0.0f);
    bassLevel = 0.0f;
    currentPulseStrength = 0.0f;
    currentOffset.set(0, 0);
    targetOffset.set(0, 0);
}
