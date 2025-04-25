#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    // Set to full screen mode
     //ofSetFullscreen(true);
    
    chronologyManager.setup();
    
 motionBlur.setup(1.0f, 0.6f);
//    staticEffect.setup();
    
   stepPrinting.setup(30); // Capture every xth frame by default -- users can adjust to make the footage more choppy/stop motiony or not
    
    fisheye.setup(1.5f);
    
    // In setup
    glitchEffect.setup(20, 10.0f, 0.5f);


    
//   glitchEffect.setup(1500, 1, 0.3);
    
    // Listen for OSC messages on port 9000
    oscReceiver.setup(9000);
    ofLog() << "Listening for OSC messages on port 9000...";


    videoFbo.allocate(standardWidth, standardHeight, GL_RGBA);
    videoFbo.begin();
    ofClear(0, 0, 0, 255);
    videoFbo.end();
    
    

}

//--------------------------------------------------------------
void ofApp::update() {
    // Update the Chronology Manager
    chronologyManager.update();
    
    // Only update effects for the main video
    ofVideoPlayer* currentVideo = chronologyManager.getCurrentVideo();
    
    if (currentVideo && currentVideo->isFrameNew()) {
        videoFbo.begin();
        ofClear(0, 0, 0, 255);
        currentVideo->draw(0, 0, standardWidth, standardHeight);
        videoFbo.end();
        
        if (isReverbActive) {
            motionBlur.update(videoFbo.getTexture());
        } else if (isStepActive) {
            stepPrinting.update(videoFbo.getTexture());
        } else if (isFisheyeActive) {
            fisheye.update(videoFbo.getTexture());
        } else if (isGlitchActive) {
            glitchEffect.update(videoFbo.getTexture());
        }
    }
    
    // Update split screen video if active (but no effects needed)
    if (chronologyManager.isSplitScreenActive && !chronologyManager.splitScreenClips.empty()) {
        chronologyManager.splitScreenClips[chronologyManager.currentSplitIndex].video.update();
    }
    
    
    if (isMotionBlurActive) {
        motionBlur.update(videoFbo.getTexture());
    } else if (isStepActive) {
        stepPrinting.update(videoFbo.getTexture());
    } else if (isFisheyeActive) {
        fisheye.update(videoFbo.getTexture());
    } else if (isGlitchActive) {
        glitchEffect.update(videoFbo.getTexture());
    }
    
    
    
    //       // Check and update effects, including Motion Blur and Step Printing
    //       if (video.isFrameNew() && isReverbActive) {
    //           motionBlur.update(video);
    //       }
    //
    //       if (video.isFrameNew() && isDelayActive) {
    //           stepPrinting.update(video);
    //       }
    
    //   videoEcho.update(video);
    
    //    staticEffect.update();
    
    //    while (oscReceiver.hasWaitingMessages()) {
    //         ofxOscMessage msg;
    //         oscReceiver.getNextMessage(msg);
    //
    //         if (msg.getAddress() == "/reverb/roomSize") {
    //             float roomSize = msg.getArgAsFloat(0);
    //             motionBlur.setRoomSizeInfluence(roomSize); // Example function
    //         } else if (msg.getAddress() == "/reverb/wetLevel") {
    //             float wetLevel = msg.getArgAsFloat(0);
    //             motionBlur.setWetLevelInfluence(wetLevel); // Example function
    //         }
    //     }
    //
    
    //receive and check for incoming OSC messages
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage m; // Create an OSC message object
        oscReceiver.getNextMessage(m); // Retrieve the next OSC message
        
        // Log the OSC message for debugging
        ofLog() << "Received OSC message: " << m.getAddress();
        
        // Check if the message relates to the reverb effect
        if (m.getAddress() == "/reverb/roomSize" || m.getAddress() == "/reverb/wetLevel") {
            float value = m.getArgAsFloat(0); // get  value from the OSC message
            isReverbActive = value > 0.0f;   // Determine if reverb should be active based on the value
            
            // Adjust  motion blur  based on the OSC message parameters
            if (m.getAddress() == "/reverb/roomSize") {
                // Map  room size value to blend factor range and apply it
                motionBlur.setBlendFactor(ofMap(value, 0.0f, 1.0f, 0.1f, 2.0f));
            } else if (m.getAddress() == "/reverb/wetLevel") {
                // Map  wet level value to stretch amount range and apply it
                motionBlur.setStretchAmount(ofMap(value, 0.0f, 1.0f, 0.1f, 1.5f));
            }
        }
        
        // Check if the message relates to the delay effect
        if (m.getAddress() == "/delay/delayTime" || m.getAddress() == "/delay/feedback") {
            float delayValue = m.getArgAsFloat(0); // get  value from the OSC message
            
            // Handle delay-related messages
            if (m.getAddress() == "/delay/delayTime") {
                motionBlur.setBlendFactor(ofMap(delayValue, 0.0f, 2000.0f, 0.1f, 3.0f)); //map delay time to blend factor
                
            } else if (m.getAddress() == "/delay/feedback") {
                motionBlur.setStretchAmount(ofMap(delayValue, 0.0f, 1.0f, 0.1f, 2.0f)); // map feedback to stretch
                
            }
            
            // Check for delay mix
            else if (m.getAddress() == "/delay/mix") {
                float mix = m.getArgAsFloat(0); // Get the mix value
                stepPrinting.setStepInterval(ofMap(mix, 0.0f, 1.0f, 1, 30)); // Map mix to max stored frames
                stepPrinting.setMaxStoredFrames(ofMap(mix, 0.0f, 1.0f, 1, 40));
                // ofLog() << "Updated Max Stored Frames (via Mix): " << stepPrinting.getMaxStoredFrames();
            }
            
        }
        
        
        // effects activation logic
        
        
        // Check for Reverb Activation
        if (m.getAddress() == "/effect/reverb/activate") {
            isReverbActive = m.getArgAsInt(0) == 1; // Activate reverb
            if (isReverbActive) isDelayActive = false; // Deactivate conflicts
        }
        
        // Check for Delay Activation
        if (m.getAddress() == "/effect/delay/activate") {
            isDelayActive = m.getArgAsInt(0) == 1; // Activate delay
            if (isDelayActive) isReverbActive = false; // Deactivate conflicts
        }
        
        // Check for video advancement
        if (m.getAddress() == "/video/advance" && m.getArgAsInt(0) == 1) {
            currentVideoIndex = (currentVideoIndex + 1) % videos.size(); // Advance to the next video
            ofLog() << "Video advanced to index: " << currentVideoIndex;
            
            // Stop the previous video and play the next one
            for (int i = 0; i < videos.size(); ++i) {
                if (i == currentVideoIndex) {
                    videos[i].play();
                } else {
                    videos[i].stop();
                }
            }
            
            
            //        // Handle Reverb Parameters
            //        if (isReverbActive && m.getAddress() == "/reverb/roomSize") {
            //            float roomSize = m.getArgAsFloat(0);
            //            motionBlur.setBlendFactor(ofMap(roomSize, 0.0f, 1.0f, 0.1f, 2.0f));
            //        }
            //
            //        // Handle Delay Parameters
            //        if (isDelayActive && m.getAddress() == "/delay/delayTime") {
            //            float delayTime = m.getArgAsFloat(0);
            //            motionBlur.setBlendFactor(ofMap(delayTime, 0.0f, 2000.0f, 0.1f, 3.0f));
            //        }
            //    }
            
            //    while (oscReceiver.hasWaitingMessages()) {
            //        ofxOscMessage m;
            //        oscReceiver.getNextMessage(m);
            //        // Log the received message
            //        ofLog() << "Received OSC message: " << m.getAddress();
            //
            //        // Check if the message relates to the delay effect
            //        if (m.getAddress() == "/delay/delayTime" || m.getAddress() == "/delay/feedback") {
            //            float delayValue = m.getArgAsFloat(0); // get  value from the OSC message
            //
            //            // Handle delay-related messages
            //            if (m.getAddress() == "/delay/delayTime") {
            //                motionBlur.setBlendFactor(ofMap(delayValue, 0.0f, 2000.0f, 0.1f, 3.0f)); //map delay time to blend factor
            //
            //            } else if (m.getAddress() == "/delay/feedback") {
            //                motionBlur.setStretchAmount(ofMap(delayValue, 0.0f, 1.0f, 0.1f, 2.0f)); // map feedback to stretch
            //
            //            }
            //        }
            
            
            //
            //            if (video.isFrameNew() && isReverbActive) {
            //                motionBlur.update(video); // Always update motion blur if reverb is active
            //            }
            //
            //    // Update step printing if the video frame is new
            //    if (video.isFrameNew() && isDelayActive) {
            //        stepPrinting.update(video);
            //    }
            //
        }
        
        //    if (video.isFrameNew()) {
        //          glitchEffect.update(video);  // Update the glitch effect
        //      }
        //}
    }
}
    
    //--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(0, 0, 0);

    if (chronologyManager.isSplitScreenActive && !chronologyManager.isPlayingAnchor()) {
        // Split screen mode - effects only on main video, not split screen
        float halfWidth = ofGetWidth() / 2.0f;
        float height = ofGetHeight();
        
        // Draw left side (main content) with effects
        ofVideoPlayer* mainVideo = chronologyManager.getCurrentVideo();
        if (mainVideo) {
            if (isMotionBlurActive) {
                motionBlur.apply(*mainVideo, 0, 0, halfWidth, height);
            } else if (isStepActive) {
                stepPrinting.apply(*mainVideo, 0, 0, halfWidth, height);
            } else if (isFisheyeActive) {
                fisheye.apply(0, 0, halfWidth, height);
            } else if (isGlitchActive) {
                glitchEffect.draw(0, 0, halfWidth, height);
            } else {
                mainVideo->draw(0, 0, halfWidth, height);
            }
        }
        
        // Draw split screen content WITHOUT effects
        if (!chronologyManager.splitScreenClips.empty()) {
            ofVideoPlayer& splitVideo = chronologyManager.splitScreenClips[chronologyManager.currentSplitIndex].video;
            
            // Calculate aspect-ratio correct dimensions
            float videoWidth = splitVideo.getWidth();
            float videoHeight = splitVideo.getHeight();
            float videoAspect = videoWidth / videoHeight;
            float screenAspect = halfWidth / height;
            
            float drawWidth, drawHeight;
            float drawX = halfWidth;
            float drawY = 0;
            
            if (videoAspect > screenAspect) {
                drawWidth = halfWidth;
                drawHeight = drawWidth / videoAspect;
                drawY = (height - drawHeight) / 2.0f;
            } else {
                drawHeight = height;
                drawWidth = drawHeight * videoAspect;
                drawX = halfWidth + (halfWidth - drawWidth) / 2.0f;
            }
            
            // Draw split screen video WITHOUT effects
            splitVideo.draw(drawX, drawY, drawWidth, drawHeight);
        }
    }
    else {
        // Normal drawing without split screen
        ofVideoPlayer* currentVideo = chronologyManager.getCurrentVideo();
        if (currentVideo) {
            if (!chronologyManager.isPlayingAnchor()) {
                if (isReverbActive) {
                    motionBlur.apply(*currentVideo, 0, 0, ofGetWidth(), ofGetHeight());
                } else if (isStepActive) {
                    stepPrinting.apply(*currentVideo, 0, 0, ofGetWidth(), ofGetHeight());
                } else if (isFisheyeActive) {
                    fisheye.apply(0, 0, ofGetWidth(), ofGetHeight());
                } else if (isGlitchActive) {
                    glitchEffect.draw(0, 0, ofGetWidth(), ofGetHeight());
                } else {
                    currentVideo->draw(0, 0, ofGetWidth(), ofGetHeight());
                }
            } else {
                // Anchor point - no effects
                currentVideo->draw(0, 0, ofGetWidth(), ofGetHeight());
            }
        }
    }

    
    
    //    // Check if there are videos and draw the current one with effects
    //    if (!videos.empty()) {
    //        if (isReverbActive) {
    //            motionBlur.apply(videos[currentVideoIndex], 0, 0, ofGetWidth(), ofGetHeight());
    //        } else if (isDelayActive) {
    //            stepPrinting.apply(videos[currentVideoIndex], 0, 0, ofGetWidth(), ofGetHeight());
    //        } else {
    //            videos[currentVideoIndex].draw(0, 0, ofGetWidth(), ofGetHeight());
    //        }
    //    }
    
    //   videoEcho.apply(video, 0,0, ofGetWidth()/2, ofGetHeight()/2);
    //    staticEffect.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    
    //    if (isReverbActive) {
    //    motionBlur.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    //    } else {
    //        motionBlur.setBlendFactor(1.0f);
    //        motionBlur.setStretchAmount(0.6f);
    //        motionBlur.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    //    }
    //
    //stepPrinting.apply(video, 0,0, ofGetWidth(), ofGetHeight());
    
    //  glitchEffect.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    
}
        
    
    //--------------------------------------------------------------
    void ofApp::exit(){
        
    }
    
    //--------------------------------------------------------------
    void ofApp::keyPressed(int key){
        //    if (key == 's') {  // Press 's' to toggle the static effect
        //         staticEffect.toggleStatic(!staticEffect.isStaticActive);
        //     }
        //
        //    if (key == 'n') {  // Press 'n' to go to the next video
        //        currentVideoIndex = (currentVideoIndex + 1) % videos.size();
        //        videos[currentVideoIndex].play();
        //    }
        
//            if (key == 's') {
//                // Only allow split screen when not showing anchor
//                if (!chronologyManager.isPlayingAnchor()) {
//                    chronologyManager.toggleSplitScreen(!chronologyManager.isSplitScreenActive);
//                } else {
//                    ofLog() << "Cannot toggle split screen during anchor playback";
//                }
//            }
//            
//            if (key == 'f') {
//                isMotionBlurActive = !isMotionBlurActive;
//            }
        }
    
    //--------------------------------------------------------------

