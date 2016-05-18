#include "ofApp.h"

void ofApp::setup(){
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
        
        if (punchy) { punchy -= 3; }
        
        float phase = zMin * 10 * ofGetElapsedTimef() * M_TWO_PI;
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
    ss << "r: toggle registration between color & depth images (";
	ss << (bUseRegistration ? "on)" : "off)") << endl;
	ss << "p: switch between images and point cloud" << endl;
	ss << "c: toggle point cloud using color image or gradient (";
	ss << (bPointCloudUseColor ? "color image)" : "gradient)") << endl;
    ss << "rotate the point cloud with the mouse" << endl;
    ss << "---" << endl;
    ss << "noddy toss" << endl;
    ss << "raw: " << zMin << endl;
    ss << "step: " << step << endl;
    ss << "multistep: " << multiStep << endl;
    ss << "punchy: " << punchy << endl;
    ss << "---";
    ss << "waves @ " << zMin * 10 << "hz" << endl;
    ss << "sin: " << sine << endl;
    ss << "square: " << square << endl;
    ss << endl;
    
    
	ofSetColor(ofColor::white);
	ofDrawBitmapStringHighlight(ss.str(), 20, 500);
}

void ofApp::keyPressed(int key){
	if (key == 'p')
		bDrawPointCloud ^= 1;
	if (key == 'c')
		bPointCloudUseColor ^= 1;
	if (key == 'r') {
		bUseRegistration ^= 1;
		astra.enableRegistration(bUseRegistration);
	}
}
