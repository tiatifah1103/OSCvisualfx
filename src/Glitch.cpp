#include "Glitch.hpp"

GlitchEffect::GlitchEffect() {
    glitchInterval = 20;  // triggers the glitches every 20 frames
    frameCounter = 0;
    glitchStrength = 10;  // increases strength/ displacement of the glitch
    glitchProbability = 0.5f;  // % probability of glitching
}

void GlitchEffect::setup(int _glitchInterval, float _glitchStrength, float _glitchProbability) {
    glitchInterval = _glitchInterval;
    glitchStrength = _glitchStrength;
    glitchProbability = _glitchProbability;
}

void GlitchEffect::update(ofVideoPlayer &video) {
    frameCounter++; // increments the frame counter each time update is called to keep track

    // Handle freeze timer when glitches are frozen
    if (isFrozen) {
        freezeTimer++; // Increments freeze timer to track how long the glitch is frozen
        if (freezeTimer > freezeDuration) {
            isFrozen = false;        // End freeze state
            freezeTimer = 0; // resets timer
            frozenGlitches.clear();  // Clear stored glitches
        }
    } else {
        // Randomly trigger frozen state
        if (ofRandom(1.0f) < 0.8f) { // % chance per update - users should be able to adjust this parameter to decide how MUCH glitcht they want - the higher it is the less flashing glitch the lower it is the less frozen glitch.
            isFrozen = true;
        }
    }

    // Generate new glitches if not frozen
    if (!isFrozen && frameCounter % glitchInterval == 0) {
        if (ofRandom(1.0f) < glitchProbability) {
            applyGlitch(video, 0, 0, video.getWidth(), video.getHeight());
        }
    }
}

void GlitchEffect::apply(ofVideoPlayer &video, float x, float y, float width, float height) {
    
    ofSetColor(255);
     video.draw(x, y, width, height); // Draw the video normally

     if (isFrozen) {
         //if in frozen state, draw stored frozen glitches.

         for (const auto &glitch : frozenGlitches) {
             ofSetColor(255, 255, 255, 200); // slight transparency
             video.getTexture().drawSubsection(
                 x + glitch.offsetX + glitch.displacement, y + glitch.offsetY,
                 glitch.width, glitch.height,
                 glitch.offsetX, glitch.offsetY
             );
         }
     } else {
         // Apply dynamic glitches
         applyGlitch(video, x, y, width, height);
     }
 }

void GlitchEffect::applyGlitch(ofVideoPlayer &video, float x, float y, float width, float height) {
    int numGlitches = 10; // Number of glitch strips

    for (int i = 0; i < numGlitches; i++) {
        // random starting position for the glitch rectangle
        float offsetX = ofRandom(0, width);  // random horizontal position.
        float offsetY = ofRandom(0, height); // random vertical position.

        // random width and height
        float glitchWidth = ofRandom(width * 0.3, width * 0.8);  // random width (10%-80% of video width)
        float glitchHeight = ofRandom(height * 0.1, height * 0.3); // random height (10%-30% of video height)

        // ensure the glitch rectangle stays within the bounds of the video
        offsetX = ofClamp(offsetX, 0, width - glitchWidth);
        offsetY = ofClamp(offsetY, 0, height - glitchHeight);

        // Random horizontal displacement for the glitch
        float displacement = ofRandom(-glitchStrength, glitchStrength);

        // Create a rectangle from the video to displace and draw it
        ofPixels pixels = video.getPixels();
        ofImage glitchImage;
        glitchImage.setFromPixels(pixels);

        // Apply a color channel effect for added distortion
        bool applyColorEffect = (ofRandom(1.0f) > 0.5f); // 50% chance to apply
       
        // distorts colour channels (swap and boost color components)
        if (applyColorEffect) {
            for (int y = 0; y < glitchImage.getHeight(); y++) {
                for (int x = 0; x < glitchImage.getWidth(); x++) {
                    ofColor color = glitchImage.getColor(x, y);

                    // Swap and modify color channels
                    std::swap(color.r, color.g); // Swap red and green
                    color.b = ofClamp(color.b * 1.2, 0, 255); // Boost blue
                    glitchImage.setColor(x, y, color);
                }
            }
            glitchImage.update(); // apply the color changes
        }

        //bind the texture for drawing
        glitchImage.getTexture().bind();
        ofSetColor(255, 255, 255, 200); // slight transparency for blending

        // draw the displaced glitch rectangle
        glitchImage.getTexture().drawSubsection(
            x + offsetX + displacement, y + offsetY, // draw at displaced position
            glitchWidth, glitchHeight,               // size of the glitch
            offsetX, offsetY                         // source position in the video
        );

        //unbind the texture
        glitchImage.getTexture().unbind();
    }
}
