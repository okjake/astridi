#include "ofApp.h"

#define FREQ_MOD 0.01

void ofApp::setup(){
    for (int i = 0; i < 50; i++) {
        messageLog[i] = "";
    }
    
    ofBackground(0);
	ofSetWindowShape(640*2, 768);
	ofSetVerticalSync(true);

	bDrawPointCloud = false;
	bPointCloudUseColor = false;
	bUseRegistration = true;
	mesh.setMode(OF_PRIMITIVE_POINTS);

	astra.setup();
	astra.enableRegistration(bUseRegistration);
	//astra.initColorStream();
	astra.initDepthStream();
	astra.initPointStream();
    punchy = 0;
    midiOut.openPort(0);
}

void ofApp::update(){
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
	astra.update();

	if (astra.isFrameNew()) {
        zMin = 0;
        mesh.clear();

		float maxDepth = 1500.0;
        float minDepth =  443.0;

        // post-normalisation values
        int   thresh   = 100;
        int multThresh = 20;
        
		int w = width = astra.getDepthImage().getWidth();
		int h = height = astra.getDepthImage().getHeight();
        
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				ofVec3f p = astra.getWorldCoordinateAt(x, y);
				if (p.z == 0) continue;
				if (p.z > maxDepth) continue;
                
                if (zMin) {
                    if (p.z < zMin) zMin = p.z;
                }
                else {
                    zMin = p.z;
                }
                
				mesh.addVertex(p);

				if (bPointCloudUseColor) {
					mesh.addColor(astra.getColorImage().getColor(x, y));
				} else {
					mesh.addColor(ofColor::fromHsb(ofMap(p.z, 0, maxDepth, 0, 255), 255, 255));
				}
			}
		}
        
        // normalise
        zMin -= minDepth;
        zMin /= ((maxDepth - minDepth) / 127.0);
        zMin = 127 - zMin;
        
        // step
        step = zMin > thresh ? 127 : 0;
        
        // multistep
        int prevMultiStep = multiStep;
        for (int i = 0, on = 0; i < 127; i += multThresh, on = !on) {
            if (zMin < i) {
                if (on) multiStep = 127;
                else multiStep = 0;
                break;
            }
        }
        
        if (multiStep != prevMultiStep) {
            punchy = 128;
        }
    
        float phase = (zMin * FREQ_MOD) * ofGetElapsedTimef() * TWO_PI;
        sine = ofMap(sin(phase), -1, 1, 0, 127);
        square = sin(phase) > 0 ? 127 : 0;
        
    }
}

void ofApp::draw(){
    
    ofBackground(0,0,0);
    
	if (!bDrawPointCloud) {
		ofSetColor(ofColor::white);
		astra.draw(0, 0);
		astra.drawDepth(640, 0);
	} else {
		cam.begin();
		ofEnableDepthTest();
		ofRotateY(180);
		ofScale(1.5, 1.5);
		mesh.draw();
		ofDisableDepthTest();
		cam.end();
	}

	stringstream ss;
    ss << "depth image: " << width << 'x' << height << endl;
    ss << "[space] switch between images and point cloud" << endl;
    ss << "[+/-] change midi channel" << endl;
    ss << "[a-z/1-9] send midi on/off" << endl;
    ss << "[</>] send control messages" << endl;
    ss << "[{/}] change control" << endl;
    ss << "rotate the point cloud with the mouse" << endl << endl;
    ss << "--- noddy toss" << endl;
    ss << "raw: " << zMin << endl;
    ss << "step: " << step << endl;
    ss << "multistep: " << multiStep << endl;
    ss << "punchy: " << punchy << endl << endl;
    ss << "--- waves @ " << zMin * FREQ_MOD << "hz" << endl;
    ss << "sin: " << sine << endl;
    ss << "square: " << square << endl << endl;
    ss << "--- send nonsense to" << endl;
    ss << "channel: " << channel << endl;
    ss << "control: " << control << endl << endl;
    
    for (int i = 0; i < 50; i++) {
        ss << messageLog[i] << endl;
    }
    
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(ss.str(), 20, 20);
}

void ofApp::log(std::string message) {
    for (int i = 50; i > 0; i--) {
        messageLog[i] = messageLog[i - 1];
    }
    messageLog[0] = message;
}

void ofApp::keyPressed(int key){
	if (key == ' ')
		bDrawPointCloud ^= 1;
    if (key == '+' && channel < 16)
        channel++;
    if (key == '-' && channel > 1)
        channel--;
    if (key == '}' && control < 16)
        control++;
    if (key == '{' && control > 1)
        control--;
    if (key == '<' && controlVal > 1) {
        controlVal--;
        midiOut.sendControlChange(channel, control, controlVal);
        log("MIDI CONTROL CHANGE CH " + to_string(channel) + " CONTROL " + to_string(control) + " VAL " + to_string(controlVal));
    }
    if (key == '>' && controlVal < 128) {
        controlVal++;
        midiOut.sendControlChange(channel, control, controlVal);
        log("MIDI CONTROL CHANGE CH " + to_string(channel) + " CONTROL " + to_string(control) + " VAL " + to_string(controlVal));
    }
    
    if(isalnum((unsigned char) key)) {
        int note = ofMap(key, 48, 122, 0, 127);
        midiOut.sendNoteOn(channel, note);
        log("MIDI ON  CH " + to_string(channel) + " NOTE " + to_string(note));
    }
    
}

void ofApp::keyReleased(int key){
    if(isalnum(key)) {
        int note = ofMap(key, 48, 122, 0, 127);
        int velocity = 0;
        midiOut.sendNoteOff(channel, note, velocity);
        log("MIDI OFF CH " + to_string(channel) + " NOTE " + to_string(note));
    }
}
