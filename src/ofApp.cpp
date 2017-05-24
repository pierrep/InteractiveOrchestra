#include "ofApp.h"

void ofApp::exit() {
        vidRecorder.close();
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);

    ofHideCursor();
    bDrawDebug = false;

    particleSystem.setup();

    mouseEmitter.velSpread = ofVec3f(25.0,25.0);
    mouseEmitter.life = 600;
    mouseEmitter.lifeSpread = 5.0;
    mouseEmitter.numPars = 1;
    mouseEmitter.color = ofColor(200,200,255);
    mouseEmitter.colorSpread = ofColor(20,20,0);
    mouseEmitter.size = 32;

    for(int i = 0; i < 100;i++){
        emitterMatrix[i].velSpread = ofVec3f(5.0,5.0);
        emitterMatrix[i].life = 300;
        emitterMatrix[i].lifeSpread = 5.0;
        emitterMatrix[i].numPars = 2;
        emitterMatrix[i].color = ofColor(100,100,155);
        emitterMatrix[i].colorSpread = ofColor(50,50,50);
        emitterMatrix[i].size = 32;
    }

    emitter1.setPosition(ofVec3f(ofGetWidth()/2,ofGetHeight()/2));
    emitter1.setVelocity(ofVec3f(10.0,0.0));
    emitter1.posSpread = ofVec3f(10,10.0);
    emitter1.velSpread = ofVec3f(10.0,10);
    emitter1.life = 1000;
    emitter1.lifeSpread = 5.0;
    emitter1.numPars = 3;
    emitter1.color = ofColor(25,25,102);
    emitter1.colorSpread = ofColor(50,50,50);
    emitter1.size = 32;
    emitter1.maxPars = 3000;

    particleState = ParticleState::Circle;

    vectorField.allocate(128, 128, 3);

    pTex.enableMipmap();
    ofLoadImage(pTex, "p.png");
    ofLoadImage(p1Tex, "p1.png");
    ofLoadImage(p2Tex, "p2.png");
    ofLoadImage(pMultiTex, "p_multi.png");

    colourScene = 0;
    maxColours = 6;

    rotAcc = 207;
    gravAcc = 185;
    drag = 0.96;
    fieldMult = 0.34;
    gravity = 50;
    displayMode = 2;

    cam.setPosition(ofVec3f(0,0,-1000));
    cam.lookAt(ofVec3f(0,0,0));

    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<ContrastPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(true);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<MotionBlurPass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<RGBShiftPass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<ZoomBlurPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(true);
    post.createPass<BleachBypassPass>()->setEnabled(false);

    /* video recording */
    videofbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,4);
    fileName = "movie";
    fileExt = ".mp4"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats
    vidRecorder.setVideoCodec("libx264");
    vidRecorder.setVideoBitrate("1500k");
    vidRecorder.setOutputPixelFormat("yuv420p");
    bRecording = false;

    setupTimeline();

    //host = "localhost";//"10.49.85.91";
    //port = 2346;
    //receiver.setup(port);
    //ofLogNotice() << "listening for osc messages from " << host  << " on port " << port;

    video.setDeviceID(0);
    video.setDesiredFrameRate(30);
    video.initGrabber(320, 180);
    threshold = 180;
    bIgnoreBlobs = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    /* interactive camera */
    video.update();
    if(video.isFrameNew())
    {
        colorImg.setFromPixels(video.getPixels());

        grayImage = colorImg;
        grayImage.mirror(false,true);
        grayImage.threshold(threshold);
        grayImage.dilate();
        grayImage.dilate();

        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayImage, 2, (320*240)/3, 10, true);	// find holes
    }

    //receiveOsc();

    /* vector field */
    for(unsigned int y = 0; y < vectorField.getHeight(); y++) {
        for(unsigned int x=0; x < vectorField.getWidth(); x++) {
            int index = vectorField.getPixelIndex(x, y);
            float angle = ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0f,ofGetElapsedTimef()*0.05f)*TWO_PI*2.0f;
            ofVec2f dir(cos(angle), sin(angle));
            dir.normalize().scale(ofNoise(x/(float)vectorField.getWidth()*4.0f, y/(float)vectorField.getHeight()*4.0f,ofGetElapsedTimef()*0.05f+10.0f));
            vectorField.setColor(x, y, ofColor_<float>(dir.x,dir.y, 0));
        }
    }

    /* main particles */
    float dt = (ofGetLastFrameTime() * 60);    
    if(particleState == ParticleState::Circle)
    {
        particleSystem.gravitateTo(ofPoint(ofGetWidth()/2,ofGetHeight()/2), gravAcc, 1, 10.0, false);
        particleSystem.rotateAround(ofPoint(ofGetWidth()/2,ofGetHeight()/2), rotAcc, 10.0, 1, false);
        particleSystem.addParticles(emitter1);
    }
    else if(particleState == ParticleState::FigureEight)
    {
        particleSystem.gravitateTo(ofPoint(ofGetWidth()/4,ofGetHeight()/3), gravAcc, 1, 10.0, false);
        particleSystem.gravitateTo(ofPoint(ofGetWidth()/4*3,ofGetHeight()/3), gravAcc, 1, 10.0, false);
        particleSystem.rotateAround(ofPoint(ofGetWidth()/4,ofGetHeight()/3), rotAcc, 10.0, 1, false);
        particleSystem.rotateAround(ofPoint(ofGetWidth()/4*3,ofGetHeight()/3), rotAcc, 10.0, -1,false);
        particleSystem.addParticles(emitter1);
    }
    else if(particleState == ParticleState::Bubbles)
    {
        particleSystem.gravitateTo(ofPoint(ofGetWidth()/2,ofGetHeight()/2), gravAcc, 1, 10.0, false);
        particleSystem.rotateAround(ofPoint(ofGetWidth()/2,ofGetHeight()/2), rotAcc, 10.0, 1, false);
        particleSystem.addParticles(emitter1);
        mouseEmitter.setPosition(ofVec3f(ofGetWidth()/2,ofGetHeight()),ofVec3f(ofGetWidth()/2,ofGetHeight()));
        mouseEmitter.posSpread = ofVec3f(10.0,10.0,0.0);
        particleSystem.addParticles(mouseEmitter);
    }
    particleSystem.applyVectorField(vectorField.getData(), vectorField.getWidth(), vectorField.getHeight(), vectorField.getNumChannels(), ofGetWindowRect(), fieldMult);
    particleSystem.update(dt, drag);

    /* matrix */
    int maxBlobs = ofClamp(contourFinder.nBlobs,0,50);
    if(bIgnoreBlobs) maxBlobs = 0;
    for (int i = 0; i < maxBlobs; i++)
    {
        float posx = contourFinder.blobs[i].centroid.x * 6.0f;
        float posy = contourFinder.blobs[i].centroid.y * 6.0f;

        emitterMatrix[i].setPosition(ofVec3f(posx,posy),ofVec3f(posx+10,posy+10));
        emitterMatrix[i].posSpread = ofVec3f(10.0,10.0,0.0);
        //emitterMatrix[i].setVelocity(posx, posy);
        particleSystem.addParticles(emitterMatrix[i]);
        particleSystem.gravitateTo(ofPoint(posx,posy), gravity, 1, 10.0, false);
    }

    /* mouse emitter */
    if(ofGetMousePressed(2))
    {
        particleSystem.gravitateTo(ofPoint(mouseX,mouseY), gravAcc, 1, 10.0, false);
    }

    ofVec2f mouseVel(mouseX-pmouseX,mouseY - pmouseY);
    mouseVel *= 2.0;
    if(ofGetMousePressed(0)){
        mouseEmitter.setPosition(ofVec3f(pmouseX,pmouseY),ofVec3f(mouseX,mouseY));
        mouseEmitter.posSpread = ofVec3f(10.0,10.0,0.0);
        //mouseEmitter.setVelocity(pmouseVel, mouseVel);
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

    if(bShowTimeline) {
        timeline.draw();
    }


    if(bDrawDebug) {
        ofPushStyle();
        //video.draw(0,0);
        ofNoFill();
        ofDrawRectangle(0,ofGetHeight()-180,320,180);
        grayImage.draw(0,ofGetHeight()-180);

        for (int i = 0; i < contourFinder.nBlobs; i++){
            contourFinder.blobs[i].draw(0,ofGetHeight()-180);
        }
        ofPopStyle();
        //cout << "contourFinder.nBlobs: " << contourFinder.nBlobs << endl;
    }

}

//--------------------------------------------------------------
void ofApp::receiveOsc()
{
        while(receiver.hasWaitingMessages()){
            ofxOscMessage m;
            receiver.getNextMessage(m);

            if(m.getAddress() == "/value") {

            }
            else {
                // unrecognized message: display on the bottom of the screen
                string msg_string;
                msg_string = m.getAddress();
                msg_string += ": ";
                for(int i = 0; i < m.getNumArgs(); i++){
                    // get the argument type
                    msg_string += m.getArgTypeName(i);
                    msg_string += ":";
                    // display the argument - make sure we get the right type
                    if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                        msg_string += ofToString(m.getArgAsInt32(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                        msg_string += ofToString(m.getArgAsFloat(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                        msg_string += m.getArgAsString(i);
                    }
                    else{
                        msg_string += "unknown";
                    }
                }
                ofLogNotice() << msg_string;

            }
        }
}

//--------------------------------------------------------------
void ofApp::sceneChange()
{

}

//--------------------------------------------------------------
void ofApp::cycleColours()
{
    colourScene++;
    if(colourScene >= maxColours) colourScene = 0;
}

//--------------------------------------------------------------
void ofApp::setColour(int colourId)
{
    ofColor newColour;

    switch(colourId) {
        case 1:
            //dark blue
           newColour = ofColor(25,25,102);
        break;
        case 2:
            // lavender
            newColour = ofColor(110,110,150);
        break;
        case 3:
            // red
            newColour = ofColor(200,100,100);
        break;
        case 4:
            // cyan
            newColour = ofColor(25,145,175);
        break;
        case 5:
            // orange
            newColour = ofColor(165,103,25);
        break;
        case 6:
            // magenta
            newColour = ofColor(175,25,145);
        break;
        default:
        break;
    }

    particleSystem.changeColour(newColour);
    emitter1.color = mouseEmitter.color = newColour;
    for(int i = 0; i < 50;i++){
        emitterMatrix[i].color = ofColor(100,100,155);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    unsigned idx = key - '0';
    //if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());
    if (idx < 7) setColour(idx);

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
        case 'i':
            bIgnoreBlobs = !bIgnoreBlobs;
            break;
        case 'n':
            cycleColours();
            break;
        case 'u':
            threshold++;
            if(threshold >= 255) threshold = 255;
            cout << "threshold="<< threshold << endl;
        break;
        case 'U':
            threshold--;
            if(threshold < 0) threshold = 0;
            cout << "threshold="<< threshold << endl;
        break;
        case 'j':
            particleState = ParticleState::Bubbles;
            rotAcc = 128;
            gravAcc = 126;
            drag = 0.91;
            fieldMult = 0.34;
        break;
        case 'k':
            particleState = ParticleState::FigureEight;
            rotAcc = 227;
            gravAcc = 270;
            drag = 0.9696;
            fieldMult = 0.0984;
        break;
        case 'l':
            particleState = ParticleState::Circle;
            rotAcc = 207;
            gravAcc = 185;
            drag = 0.96;
            fieldMult = 0.34;
        break;
        case 'p':
            particleSystem.setImmortal(false);
        break;
//        case 'P':
//            particleSystem.setImmortal(true);
//        break;

        case 'q':
            post[4]->setEnabled(!post[4]->getEnabled());
        break;
        case 'w':
            post[7]->setEnabled(!post[7]->getEnabled());
        break;
        default:
            break;
    }


    if(key == 'm') {
        if(bRecording) {
            bRecording = false;
            vidRecorder.close();
        } else {
            /* uncomment the below to record */
            //bRecording = !bRecording;
            if(bRecording && !vidRecorder.isInitialized()) {
                ofLogNotice() << "Set up video recording";
                vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, ofGetWidth(), ofGetHeight(), 30, 0, 0);
                vidRecorder.start();
            }
        }
    }

    if(key == 't') {
        bShowTimeline  = !bShowTimeline;
        if(bShowTimeline) timeline.show();
        else timeline.hide();
    }

}


void ofApp::setupTimeline()
{
    bShowTimeline = false;
    timeline.setup();

    timeline.addCurves("curves", ofRange(0, 255));
    timeline.addBangs("bangs");
    timeline.addFlags("flags");
    timeline.addColors("colors");
    timeline.addLFO("lfo");
    timeline.addSwitches("switches");

    timeline.setPageName("Page 1");
    timeline.addPage("Page 2");
    timeline.addPage("Page 3");
    timeline.addPage("Page 4");
    timeline.setCurrentPage(0);

    timeline.enableSnapToOtherKeyframes(false);
    timeline.setLoopType(OF_LOOP_NORMAL);

    ofAddListener(timeline.events().bangFired, this, &ofApp::bangFired);
}

//--------------------------------------------------------------
void ofApp::bangFired(ofxTLBangEventArgs& args){
    ofLogVerbose() << "bang fired!" << args.flag;
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
//    emitter1.setPosition(ofVec3f(0,h/3));
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
