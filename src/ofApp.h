//
//  ofApp.cpp
//  example
//
//  Created by Matt Felsen on 11/15/15.
//
//

#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxOrbbecAstra.h"

class ofApp : public ofBaseApp{

public:

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
    void keyReleased(int key);
    void log(std::string message);
    
	ofxOrbbecAstra astra;

	ofVboMesh mesh;
	ofEasyCam cam;
    ofxMidiOut midiOut;
    
	bool bDrawPointCloud;
	bool bPointCloudUseColor;
	bool bUseRegistration;
    
    int zMin;
    int step;
    int multiStep;
    int punchy;
    int width;
    int height;
    int sine;
    int square;
    
    int channel = 1;
    int control = 1;
    int controlVal = 1;
    int prevzMin = 1;
    
    std::string messageLog[50];

};
