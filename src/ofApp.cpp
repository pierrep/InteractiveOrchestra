#include "ofApp.h"

void ofApp::exit() {
        vidRecorder.close();
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);
    ofSetVerticalSync(false);

    ofHideCursor();
    bDrawDebug = false;

    particleSystem.setup();

    mouseEmitter.velSpread = ofVec3f(25.0,25.0);
    mouseEmitter.life = 600;
    mouseEmitter.lifeSpread = 5.0;
    mouseEmitter.numPars = 10;
    mouseEmitter.color = ofColor(200,200,255);
    mouseEmitter.colorSpread = ofColor(20,20,0);
    mouseEmitter.size = 32;    

    leftEmitter.setPosition(ofVec3f(ofGetWidth()/2,ofGetHeight()/2));
    leftEmitter.setVelocity(ofVec3f(10.0,0.0));
    leftEmitter.posSpread = ofVec3f(10,10.0);
    leftEmitter.velSpread = ofVec3f(10.0,10);
    leftEmitter.life = 1000;
    leftEmitter.lifeSpread = 5.0;
    leftEmitter.numPars = 3;
    leftEmitter.color = ofColor(200,100,100);
    leftEmitter.colorSpread = ofColor(50,50,50);
    leftEmitter.size = 32;
    leftEmitter.maxPars = 3000;

//    rightEmitter = leftEmitter;
//    rightEmitter.setPosition(ofVec3f(ofGetWidth()-1,ofGetHeight()*2/3));
//    rightEmitter.setVelocity(ofVec3f(-10.0,0.0));
//    rightEmitter.color = ofColor(100,100,200);
//    rightEmitter.colorSpread = ofColor(50,50,50);

//    topEmitter = leftEmitter;
//    topEmitter.setPosition(ofVec3f(ofGetWidth()*2/3,0));
//    topEmitter.setVelocity(ofVec3f(0.0,10.0));
//    topEmitter.color = ofColor(100,200,100);
//    topEmitter.colorSpread = ofColor(50,50,50);

//    botEmitter = leftEmitter;
//    botEmitter.setPosition(ofVec3f(ofGetWidth()/3,ofGetHeight()-1));
//    botEmitter.setVelocity(ofVec3f(0.0,-10.0));
//    botEmitter.color = ofColor(200,200,0);
//    botEmitter.colorSpread = ofColor(50,50,0);

    vectorField.allocate(128, 128, 3);

    pTex.enableMipmap();
    ofLoadImage(pTex, "p.png");

    ofLoadImage(p1Tex, "p1.png");
    ofLoadImage(p2Tex, "p2.png");
    ofLoadImage(pMultiTex, "p_multi.png");

    rotAcc = 207;
    gravAcc = 185;
    drag = 0.96;
    fieldMult = 0.34;
    displayMode = 0;

    cam.setPosition(ofVec3f(0,0,-1000));
    cam.lookAt(ofVec3f(0,0,0));

    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<ContrastPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<MotionBlurPass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<RGBShiftPass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<ZoomBlurPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(false);
    post.createPass<BleachBypassPass>()->setEnabled(false);

    /* video recording */
    videofbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fileName = "movie";
    fileExt = ".mp4"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    vidRecorder.setVideoCodec("libx264");
    vidRecorder.setVideoBitrate("1500k");
    vidRecorder.setOutputPixelFormat("yuv420p");
    bRecording = false;

}

//--------------------------------------------------------------
void ofApp::update(){

    for(int y = 0; y < vectorField.getHeight(); y++)
        for(int x=0; x< vectorField.getWidth(); x++){
            int index = vectorField.getPixelIndex(x, y);
            float angle = ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05)*TWO_PI*2.0;
            ofVec2f dir(cos(angle), sin(angle));
            dir.normalize().scale(ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05+10.0));
            vectorField.setColor(x, y, ofColor_<float>(dir.x,dir.y, 0));
        }

    particleSystem.gravitateTo(ofPoint(ofGetWidth()/2,ofGetHeight()/2), gravAcc, 1, 10.0, false);
    particleSystem.rotateAround(ofPoint(ofGetWidth()/2,ofGetHeight()/2), rotAcc, 10.0, false);
    particleSystem.applyVectorField(vectorField.getData(), vectorField.getWidth(), vectorField.getHeight(), vectorField.getNumChannels(), ofGetWindowRect(), fieldMult);
    if(ofGetMousePressed(2)){
        particleSystem.gravitateTo(ofPoint(mouseX,mouseY), gravAcc, 1, 10.0, false);
    }

    float dt = (ofGetLastFrameTime() * 60);

    particleSystem.update(dt, drag);

    particleSystem.addParticles(leftEmitter);
//    particleSystem.addParticles(rightEmitter);
//    particleSystem.addParticles(topEmitter);
//    particleSystem.addParticles(botEmitter);

    ofVec2f mouseVel(mouseX-pmouseX,mouseY - pmouseY);
    mouseVel *= 2.0;
    if(ofGetMousePressed(0)){
        mouseEmitter.setPosition(ofVec3f(pmouseX,pmouseY),ofVec3f(mouseX,mouseY));
        mouseEmitter.posSpread = ofVec3f(10.0,10.0,0.0);
        mouseEmitter.setVelocity(pmouseVel, mouseVel);
        particleSystem.addParticles(mouseEmitter);
    }
    pmouseX = mouseX;
    pmouseY = mouseY;
    pmouseVel = mouseVel;

    if(bRecording){
        ofPixels pix;
        videofbo.readToPixels(pix);
        pix.setNumChannels(3);
        vidRecorder.addFrame(pix);
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    if(ofGetKeyPressed('v')){
        ofMesh fieldMesh;
        fieldMesh.disableIndices();
        fieldMesh.setMode(OF_PRIMITIVE_LINES);
        for(int y = 0; y < vectorField.getHeight(); y++) {
            for(int x=0; x< vectorField.getWidth(); x++) {
                ofColor_<float> c = vectorField.getColor(x, y);
                ofVec3f pos(x,y);
                ofVec3f dir(c.r, c.g);
                fieldMesh.addVertex(pos);
                fieldMesh.addColor(c);
                fieldMesh.addVertex(pos + dir);
                fieldMesh.addColor(c);
            }
        }

        ofSetLineWidth(1.0);
        ofSetColor(80, 80, 80);
        ofPushMatrix();
        ofScale(ofGetWidth()/(float)vectorField.getWidth(), ofGetHeight()/(float)vectorField.getHeight());
        fieldMesh.draw();
        ofPopMatrix();
    }

    if(bDrawDebug) {
        // Draw gravity and rotation radii
        ofPushStyle();
        ofNoFill();
        ofSetCircleResolution(180);
        ofSetColor(255, 0, 0, 50);
        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, gravAcc);
        ofSetColor(0, 0, 255, 50);
        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, rotAcc);
        ofPopStyle();
    }

    if(bRecording){                
        videofbo.begin();
        ofClear(0,0,0,255);
    }

    //cam.begin();
    post.begin(&cam);
    ofPushMatrix();
    ofScale(-1,-1.0f,1);
    ofTranslate(-ofGetWidth()/2,-ofGetHeight()/2);
    ofSetLineWidth(2.0);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    if (displayMode == 1) {
        particleSystem.draw(pTex);
    }
    else if(displayMode == 2) {
        particleSystem.draw(pMultiTex, p2Tex);
    }
    else {
        particleSystem.draw();
    }
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofPopMatrix();
    //cam.end();
    post.end();

    if(bRecording){
        videofbo.end();
        videofbo.draw(0,0);
    }

    if(bDrawDebug) {
        ofPushStyle();
        float dt = (ofGetLastFrameTime() * 60);
        ofSetColor(255, 255, 255);
        ofDrawBitmapString("Num particles: " + ofToString(particleSystem.getNumParticles()) + "\n" + "FPS: "+ofToString(ofGetFrameRate()) +
                           "\n(G/g) gravitation: " + ofToString(gravAcc) +
                           "\n(R/r) rotational acceleration: " + ofToString(rotAcc) +
                           "\n(F/f) vector field multiplier: " + ofToString(fieldMult) +
                           "\n(D/d) drag constant: " + ofToString(drag) +
                           "\n(v) show vector field" +
                           "\n(1-3) particle display modes" +
                           "\ntimestep: " + ofToString(dt) , 20,20);
        ofPopStyle();
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    unsigned idx = key - '0';
    if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());

    switch (key) {
        case 'r':
            if(rotAcc > 1.1)
                rotAcc /= 1.1;
            break;
        case 'R':
            rotAcc *= 1.1;
            break;

        case 'g':
            if(gravAcc > 1.1)
                gravAcc /= 1.1;
            break;
        case 'G':
            gravAcc *= 1.1;
            break;

        case 'd':
            if(drag > 0.01)
                drag /= 1.01;
            break;
        case 'D':
            drag *= 1.01;
            if(drag > 1.0) drag = 1.0;
            break;
        case 'f':
            if(fieldMult > 0.1)
                fieldMult /= 1.1;
            break;
        case 'F':
            fieldMult *= 1.1;
            break;
        case 'z':
            displayMode = 0;
            break;
        case 'x':
            displayMode = 1;
            break;
        case 'c':
            displayMode = 2;
            break;
        case 'b':
            bDrawDebug = !bDrawDebug;
            break;
        default:
            break;
    }

    if(key == 'm') {
        if(bRecording) {
            bRecording = false;
            vidRecorder.close();
        } else {
            bRecording = !bRecording;
            if(bRecording && !vidRecorder.isInitialized()) {
                ofLogNotice() << "Set up video recording";
                vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, ofGetWidth(), ofGetHeight(), 30, 0, 0);
                vidRecorder.start();
            }
        }

    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
//    leftEmitter.setPosition(ofVec3f(0,h/3));
//    rightEmitter.setPosition(ofVec3f(w-1,h*2/3));
//    topEmitter.setPosition(ofVec3f(w*2/3,0));
//    botEmitter.setPosition(ofVec3f(w/3,h-1));
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
