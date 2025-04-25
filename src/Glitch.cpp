#include "Glitch.hpp"

GlitchEffect::GlitchEffect() {
    glitchInterval = 20;
    frameCounter = 0;
    glitchStrength = 10;
    glitchProbability = 0.5f;
    isFrozen = false;
    freezeTimer = 0;
    freezeDuration = 10;
}

void GlitchEffect::setup(int _glitchInterval, float _glitchStrength, float _glitchProbability) {
    fboWidth = ofGetWidth();
    fboHeight = ofGetHeight();
    glitchInterval = _glitchInterval;
    glitchStrength = _glitchStrength;
    glitchProbability = _glitchProbability;

    if (!glitchFbo.isAllocated() || glitchFbo.getWidth() != fboWidth || glitchFbo.getHeight() != fboHeight) {
        glitchFbo.allocate(fboWidth, fboHeight, GL_RGBA);
        ofLogNotice("GlitchEffect") << "Allocated glitchFbo with screen dimensions: " << fboWidth << "x" << fboHeight;
    }

    glitchFbo.begin();
    ofClear(0, 0, 0, 255);
    glitchFbo.end();
}



void GlitchEffect::update(ofTexture &videoTexture) {
    frameCounter++;

    glitchFbo.begin();
    ofClear(0, 0, 0, 255);

    // Draw base texture
    videoTexture.draw(0, 0, fboWidth, fboHeight);

    if (isFrozen) {
        freezeTimer++;
        if (freezeTimer > freezeDuration) {
            isFrozen = false;
            freezeTimer = 0;
            frozenGlitches.clear();
        }
    } else {
        if (frameCounter % glitchInterval == 0 && ofRandom(1.0f) < glitchProbability) {
            generateGlitches(fboWidth, fboHeight);
        }

        if (ofRandom(1.0f) < 0.05f) {
            isFrozen = true;
        }
    }

    drawGlitches(videoTexture, 0, 0, fboWidth, fboHeight);
    glitchFbo.end();
}

void GlitchEffect::draw(float x, float y, float width, float height) {
    ofSetColor(255);
    glitchFbo.draw(x, y, width, height);
}

void GlitchEffect::generateGlitches(float width, float height) {
    frozenGlitches.clear();
    int numGlitches = 10;

    for (int i = 0; i < numGlitches; i++) {
        GlitchRect rect;
        rect.width = ofRandom(width * 0.2, width * 0.6);
        rect.height = ofRandom(height * 0.05, height * 0.2);
        rect.offsetX = ofRandom(0, width - rect.width);
        rect.offsetY = ofRandom(0, height - rect.height);
        rect.displacement = ofRandom(-glitchStrength, glitchStrength);
        frozenGlitches.push_back(rect);
    }
}

void GlitchEffect::drawGlitches(ofTexture &tex, float x, float y, float w, float h) {
    ofSetColor(255, 255, 255, 200); // slight transparency

    for (auto &glitch : frozenGlitches) {
        tex.drawSubsection(
            glitch.offsetX + glitch.displacement, glitch.offsetY,
            glitch.width, glitch.height,
            glitch.offsetX, glitch.offsetY
        );
    }
}
