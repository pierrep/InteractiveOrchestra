#pragma once

#include "ofMain.h"
#include "ofxParticles.h"
#include "ofxPostProcessing.h"
#include "ofxVideoRecorder.h"
#include "ofxTimeline.h"
#include "ofxOsc.h"
#include "ofxOpenCv.h"

#define matrixWidth 320
#define matrixHeight 240

class CrowdMatrix {
public:
    int x;
    int y;
    int value;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
        void cycleColours();
        void sceneChange();
        void setupTimeline();
        void receiveOsc();

        ofCamera cam;

        ofxParticleSystem particleSystem;
        int pmouseX, pmouseY;
        ofVec2f pmouseVel;

        ofxParticleEmitter mouseEmitter;
        ofxParticleEmitter emitter1;
        ofxParticleEmitter emitterMatrix[100];
        //CrowdMatrix        crowdMatrix[matrixWidth][matrixHeight];
        float rotAcc, gravAcc, fieldMult, drag;
        ofFloatPixels vectorField;

        ofTexture pTex, p1Tex, p2Tex, pMultiTex;
        int displayMode;

        ofxPostProcessing post;

        /* recording */
        ofxVideoRecorder vidRecorder;
        bool bRecording;
        string fileName;
        string fileExt;        
        ofFbo videofbo;

        /* Timeline */
        ofxTimeline timeline;
        void bangFired(ofxTLBangEventArgs& args);
        bool bShowTimeline;

        /* OSC */
        ofxOscReceiver receiver;
        string host;
        int port;
        int msg_string;
        int array_x;
        int array_y;
        int array_val;
        ofImage receivedImage;

        /* Colour changes */
        unsigned int colourScene;
        unsigned int maxColours;

        /* video input */
        ofVideoGrabber video;
        ofPixels videoPixels;
        ofTexture videoTexture;
        int camWidth;
        int camHeight;

        ofxCvColorImage		colorImg;
        ofxCvGrayscaleImage grayImage;
        ofxCvContourFinder 	contourFinder;
         int 				threshold;


    private:
        bool bDrawDebug;
};
