#include "ofApp.h"

//having issues with two effects playing at one time need to fix asap

//--------------------------------------------------------------
void ofApp::setup(){
    video.load("1980s_UK_Dance_Hall_Party_Black_British_Life_Home_Movies.mp4"); //test video
//    video.setLoopState(OF_LOOP_NORMAL);
    video.play();
    
//   videoEcho.setup(0.9f, 0.9f, 10); //300 ms delay // with some videos the echo effect is MUCH less obvious users will need to set some heavy parameters on this one to make it very obvious in some vids

 motionBlur.setup(1.0f, 0.6f);
//    staticEffect.setup();
    
//   stepPrinting.setup(40); // Capture every xth frame by default -- users can adjust to make the footage more choppy/stop motiony or not
    
//   glitchEffect.setup(1500, 1, 0.3);
    
    // Listen for OSC messages on port 9000
    oscReceiver.setup(9000);
    ofLog() << "Listening for OSC messages on port 9000...";
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
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
     }
    

    
    

    

    
    if (video.isFrameNew()) {
        motionBlur.update(video); // Always update motion blur
    }
    

    
    
    
//
//    if (video.isFrameNew()){
//        stepPrinting.update(video);
//    }
 
//    if (video.isFrameNew()) {
//          glitchEffect.update(video);  // Update the glitch effect
//      }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0, 0, 0);
//   videoEcho.apply(video, 0,0, ofGetWidth()/2, ofGetHeight()/2);
//    staticEffect.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    
    if (isReverbActive) {
        motionBlur.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    } else {
        motionBlur.setBlendFactor(1.0f);
        motionBlur.setStretchAmount(0.6f);
        motionBlur.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
    }
//
//    stepPrinting.apply(video, 0,0, ofGetWidth(), ofGetHeight());
    
//  glitchEffect.apply(video, 0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 's') {  // Press 's' to toggle the static effect
         staticEffect.toggleStatic(!staticEffect.isStaticActive);
     }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

