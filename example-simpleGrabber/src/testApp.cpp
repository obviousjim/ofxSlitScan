#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    ofImage distortionMap;
    distortionMap.loadImage("distortion.png");
    
    //set up a slit scan with a maximum capacity of frames
    // in the distortion buffer
    slitScan.setup(640,480,30,OF_IMAGE_COLOR);
    
    slitScan.setDelayMap(distortionMap);
    //blending means the edges between the scans are feathered
    slitScan.setBlending(true);
    //time delay is the deepest in history the delay can go
    //and width is the number of frames the distortion will encompass
    //note that the delay cannot be more than the total capacity
    slitScan.setTimeDelayAndWidth(30, 30);
    
    //set up the grabber
    grabber.initGrabber(640, 480);
}

//--------------------------------------------------------------
void testApp::update(){
    grabber.update();
    if(grabber.isFrameNew()){
        slitScan.addImage(grabber);
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    slitScan.getOutputImage().draw(0, 0);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}