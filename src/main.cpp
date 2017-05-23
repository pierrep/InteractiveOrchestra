#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    //ofSetLogLevel(OF_LOG_VERBOSE);

    ofGLFWWindowSettings settings;
    settings.setGLVersion(2, 1);
    settings.multiMonitorFullScreen = true;
    settings.width = 1920;
    settings.height = 1080;
    settings.windowMode = OF_GAME_MODE;

//    settings.width = 1280;
//    settings.height = 720;
//    settings.windowMode = OF_WINDOW;

    ofCreateWindow(settings);


	ofRunApp(new ofApp());
}
