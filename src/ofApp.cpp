#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofEnableAlphaBlending(); //for FBO
    ofSetFrameRate(30);
    glPointSize(4);
    glEnable(GL_POINT_SMOOTH);
    ofBackground(0);
    ofSetCircleResolution(128);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofHideCursor();
    
    //setup particles___________________________
    p.assign(NUMPART, particle());
    for(int i=0; i<p.size(); i++){
        p[i].setup();
    }
    
    
    //setup live video input___________________________
    //setup kinect
    kinect.init();
    kinect.open();
    kinect.setCameraTiltAngle(0);
    width = kinect.getWidth();
    height = kinect.getHeight();
    bLED = false;
    
    
    
    //setup openCv images___________________________
    bShowCv = false;
    colorImg.allocate(width, height);
    grayImg.allocate(width, height);
    grayBg.allocate(width, height);
    grayDiff.allocate(width, height);
    bLearnBackground = true;
    
    
    
    //setup fbo___________________________
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB);
    fbo.begin();
    ofClear(255, 255, 255, 0);
    fbo.end();
    
    
    //setup mesh___________________________
    // OF_PRIMITIVE_TRIANGLES, OF_PRIMITIVE_TRIANGLE_STRIP, OF_PRIMITIVE_TRIANGLE_FAN, OF_PRIMITIVE_LINES, OF_PRIMITIVE_LINE_STRIP, OF_PRIMITIVE_LINE_LOOP, OF_PRIMITIVE_POINTS
    mesh.setMode(OF_PRIMITIVE_LINES);
    mesh.enableColors();
    mesh.enableIndices();
    
    
    //setup easyCam___________________________
    easyCam.setVFlip(true);
    easyCam.setNearClip(1.0);
    easyCam.setPosition(0, 0, 665.107);
    
    //setup gui___________________________
    bGui = false;
    gui.setup();
    gui.add(bDrawParticle.setup("pDraw", true));
    gui.add(overallSpeed.setup("pSpeed", 0.8, 0, 3));
    gui.add(noiseAmount.setup("pNoise", 0.1, 0, 3));
    gui.add(bRepel.setup("pRepel", false));
    gui.add(trail.setup("pTrail", 12.0, 0, 20));
    gui.add(sizeDot.setup("pSizeDot", 0.5, 0.1, 1.0));
    gui.add(color[0].setup("pColor1", ofColor(188, 70, 40), ofColor(0, 0, 0), ofColor(255, 255, 255)));
    gui.add(color[1].setup("pColor2", ofColor(115, 104, 27), ofColor(0, 0, 0), ofColor(255, 255, 255)));
    gui.add(color[2].setup("pColor3", ofColor(2, 22, 132), ofColor(0, 0, 0), ofColor(255, 255, 255)));
    gui.add(color[3].setup("pColor4", ofColor(104, 28, 191), ofColor(0, 0, 0), ofColor(255, 255, 255)));
    gui.add(pBrightness.setup("pBrightness", 255, 128, 255));
    
    gui.add(threshold.setup("cvThreshold", 40, 0, 255));
    gui.add(blur.setup("cvBlur", 11, 0, 21));
    gui.add(bErode.setup("cvErode", false));
    gui.add(bDilate.setup("cvDilate", false));
    gui.add(maxBlobNum.setup("cvMaxBlobNum", 10, 1, 20));
    gui.add(maxArea.setup("cvMaxArea", 50000, 10000, (width * height)*0.5));
    gui.add(minArea.setup("cvMinArea", 3000, 20, 9999));
    gui.add(bLearnBackground.setup("cvAaptureBackground", true));
    
    gui.add(lightnessMin.setup("mLightnessMin", 40, 0, 255));
    gui.add(lightnessMax.setup("mLightnessMax", 80, 0, 255));
    gui.add(connectionDistance.setup("mDistanceThresh", 50, 20, 100));
    gui.add(mSaturation.setup("mSaturation", 80, 0, 255));
    gui.add(lineWidth.setup("mLineWidth", 3, 1, 10));
    gui.add(resizeScale.setup("mResize", 6, 2, 16));
    gui.add(displacementScale.setup("mDisplacement", 10, 5, 15.0));
    resizeScale.addListener(this, &ofApp::change_resizeScale);
    
    gui.add(videoVol.setup("vVol", 0.3, 0.0, 1.0));
    gui.add(ghostScale.setup("vGhost",8, 5, 20));
    gui.add(bDistMod.setup("vDistMod", true));
    
    gui.add(breathVol.setup("breath", 0.5, 0.0, 1.0));
    

    
    //setup background video___________________________
    video.load("murphy_quad.mov");
    
    video.setVolume(videoVol);
    video.play();
    video.setLoopState(OF_LOOP_NORMAL);
    ow = video.getWidth(); oh = video.getHeight();
    //    rw = ow/resizeScale; rh = oh/resizeScale;  //use video for mesh
    
    rw = width/resizeScale; rh = height/resizeScale; //use kinect for mesh
    zDev = 10;

    //setup sound
    breath.load("/Applications/of_v0.12.0/apps/myApps/prototype_1/sc code/foa_voice/16_in_ex_a_hoa5.wav");
    breath.setVolume(breathVol);
    breath.setLoop(true);
    breath.play();
    
    
    //unique values for each vertex to jitter
    for(int y=0; y<rh; y++){
        for(int x =0 ; x<rw; x++){
            offsets.push_back(ofVec3f(ofRandom(0, 100000), ofRandom(0, 100000), ofRandom(0, 100000)));
        }
    }
    
    //setup effects
    effects.init();
    effects.createPass<BloomPass>();
    effects.createPass<RGBShiftPass>();
    effects.createPass<ZoomBlurPass>();  //BloomPass, ZoomBlurPass, EdgePass, KaleidoscopePass, NoiseWarpPass, PixelatePass, RGBShiftPass, ToonPass
    
    //setup OSC
    sender.setup(HOST, SENDPORT); //"localhost", 12345
    receiver.setup(LISPORT); //12344
    oscFreq = 2;
    
    //setup video delays
    nDelayFrames = 300;
    buffer.setup(nDelayFrames);
    tex.allocate(width, height, GL_LUMINANCE);
}

//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    breath.setVolume(breathVol);
    
    //update live video input___________________________
    
    //    webcam.update();
    kinect.update();
    
    bool bFrameNew;
    //    bFrameNew = webcam.isFrameNew();
    bFrameNew = kinect.isFrameNew();
    
    
    //update openCV and draw mesh___________________________
    if(bFrameNew){
        
        //OpenCv___________________________
        
        grayImg = kinect.getDepthPixels(); //using depth image - better for low light
        grayImg.mirror(false, true);
        
        if(bLearnBackground){
            grayBg = grayImg;
            bLearnBackground=false;
        }
        
        grayDiff.absDiff(grayBg, grayImg); //background subtraction
        grayDiff.blurGaussian(blur);//gaussianBluf
        grayDiff.threshold(threshold); //thresholding
        
        //erode and dilate
        if(bErode)grayDiff.erode();
        if(bDilate)grayDiff.dilate();
        
        contourFinder.findContours(grayDiff, minArea, maxArea, maxBlobNum, true);
        
        //draw mesh___________________________
        updateMesh();
        
        //push frame into buffer
        buffer.pushPixels(kinect.getDepthPixels());
    }
    
    //update particle___________________________
    
    //calculate silhouette for repel
    vector<ofVec2f>blobPts;
    for(int i=0; i<contourFinder.nBlobs; i++){
        for(int j=0; j<contourFinder.blobs[i].pts.size(); j++){
            blobPts.push_back(contourFinder.blobs[i].pts[j] * ofGetWidth()/width);
        }
    }
    
    //particles update
    //    ofVec2f avgRepelFrc;
    float avgRepelFrc, avgVel;
    for(int i=0; i<p.size(); i++){
        p[i].repel(blobPts);
        p[i].update(overallSpeed, noiseAmount, bRepel);
        avgRepelFrc += p[i].repelFrc.length();
        avgVel += p[i].vel.length();
    }
    avgRepelFrc = avgRepelFrc/p.size()/0.0005;
    avgVel = avgVel /p.size();
    
    if(kinect.isFrameNew()){

        //distance interaction
        float avgDist=650.0f, smoothDist = 650.0f, distPct;
        float nearInDist = 1050, farInDist = 1300, distWeight;
        float nearOutDist = 800, farOutDist = 1;
        
    
        for( int y = 0; y < height; y++) {
            for( int x = 0; x < width; x++) {
                avgDist += kinect.getDistanceAt(x,y);
            }
        }
        
        avgDist /= (height * width);
        distWeight = avgDist/kinect.getFarClipping();
        smoothDist = smoothDist * distWeight + avgDist * (1-distWeight); //interpolation
        distPct = ofMap(smoothDist, nearInDist, farInDist, 1, 0, true); //cap smoothDist range, linear pct
        distPct = powf(distPct, 1.2);
        distPct = roundf(distPct * 100)/100;

        
        //transformation calculation
        float time = ofGetElapsedTimef();


        if(bDistMod){
            if(distPct<=0.35){
                zDev *= 1.05;
                zDev = ofClamp(zDev, farOutDist, nearOutDist);
            } else {
                zDev *= 0.97;
                zDev = ofClamp(zDev, farOutDist, nearOutDist );
            }
        } else {
            zDev = ofMap(ofNoise(time/9.0), 0, 1, -20, 100); //non interactive
        }

        float xDev = ofMap(ofNoise(time/10.0), 0, 1, -100, 100) *  (1 - zDev/nearOutDist);
        float yDev = ofMap(ofNoise(time/4.0), 0, 1, -60, 60) * (1 - zDev/nearOutDist);
        float rotateDeg = ofMap(ofNoise(time/2.0), -1, 1, -20, 20)  * (1 - zDev/nearOutDist);
        
        //update video and draw video into effects___________________________
        video.update();
        video.setVolume(videoVol);


        ofEnableDepthTest();
        effects.begin(easyCam);
        ofPushMatrix();
        ofRotateDeg(rotateDeg, 0, 1, 0); //
        ofTranslate(-ofGetWidth()/2 + xDev, -ofGetHeight()/2+ yDev, zDev);

        ofSetColor(255, 255, 255, 255);
        video.draw(0, 0, ofGetWidth(), ofGetHeight());
        effects.end(false);
        ofPopMatrix();
        ofDisableDepthTest();
                
        
        //sending message to SC
        if(ofGetFrameNum()% oscFreq ==0){
            ofxOscMessage sendM;
            sendM.setAddress("/ofParams");
            sendM.addFloatArg(mesh.getNumVertices());
            sendM.addFloatArg(mesh.getCentroid().x);
            sendM.addFloatArg(mesh.getCentroid().y);
            sendM.addIntArg(contourFinder.nBlobs);
            sendM.addFloatArg(blobPts.size());
            sendM.addFloatArg(avgRepelFrc);
            sendM.addFloatArg(avgVel);
            sendM.addFloatArg(smoothDist);
            sendM.addFloatArg(zDev);
            sender.sendMessage(sendM, false);
            
            
            while(receiver.hasWaitingMessages()){
                ofxOscMessage recvM;
                receiver.getNextMessage(recvM);
                
                if(recvM.getAddress() == "/scParams"){ //0 is the first argument sent, not the path
                    amp = recvM.getArgAsFloat(0);
                    freq = recvM.getArgAsFloat(1);
                    entropy = recvM.getArgAsFloat(2);
                }
                
            }
        }
        
        smoothDist  = avgDist; //update smoothDev
    }
    
    if(bLED){
        kinect.setLed(ofxKinect::LED_GREEN);
    } else {
        kinect.setLed(ofxKinect::LED_OFF);
    }
    
    //setup delayed video
    int ind = ofMap(amp, 0, 1, nDelayFrames-1, 15);
    tex.loadData(buffer.getDelayedPixels(ind));
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //draw effect outside of fbo
    effects.draw(0, 0, ofGetWidth(), ofGetHeight());
    

    
    //drawing into fbo (for trailing)___________________________
    fbo.begin();
    
    ofSetColor(255, 255, 255, trail*ghostScale);
    kinect.drawDepth(0, 0, ofGetWidth(), ofGetHeight());
    
    if(ofGetFrameNum()>=nDelayFrames-1){
        tex.draw(0, 0, ofGetWidth(), ofGetHeight());
    }


    if(bDrawParticle){
        //draw particles outside of effects (vs inside)
        ofSetColor(255, 255, 255, 255);
        for(int i=0; i<p.size(); i++){
            ofColor thisColor =color[p[i].uniqueCol];
            thisColor.setBrightness(pBrightness);
            ofSetColor(thisColor);
            p[i].draw(sizeDot);
        }
    }
    
    
    //draw mesh inside fbo
    ofPushMatrix();
    ofSetLineWidth(lineWidth);
    mesh.draw();
    ofNoFill();
    
    //listening from SC
    if(amp>0.04){
        float radius = ofMap(amp, 0.1, 0.3, 100, ofGetWidth()/2,true);
        ofDrawCircle(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), radius);
    }
    ofFill();
    ofPopMatrix();

    
    fbo.end();
    
    //draw fbo within easyCam___________________________
    ofEnableDepthTest();
    easyCam.begin();
    ofPushMatrix();
    ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2, 0);
    fbo.draw(0, 0);
    ofPopMatrix();
    easyCam.end();
    ofDisableDepthTest();
    
    
    //listening from SC___________________________________
    ofNoFill();
    if(amp>0.03){
        float radius = ofMap(amp, 0.1, 0.3, 10, 500,true);
        ofDrawCircle(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), radius);
    }
    //draw openCv___________________________
    ofFill();
    if(bShowCv){
        
        ofSetHexColor(0xffffff);
        grayImg.draw(20, 20);
        grayBg.draw(40+width, 20);
        grayDiff.draw(20, 40 + height);
        
        ofFill();
        ofSetHexColor(0x333333);
        ofDrawRectangle(40+width, 40+height, width, height);
        ofSetHexColor(0xffffff);
        contourFinder.draw(40+width, 40+height);
        
        kinect.drawDepth(60 + width*2,20, -width, height);

    }
    
    //draw gui___________________________
    if(bGui){
        ofSetColor(255, 255, 255, 255);
        gui.draw();
        ofDrawBitmapString("frame rate: "+ofToString(ofGetFrameRate(), 2), 20, ofGetHeight() -20);
        ofShowCursor();
    } else {
        ofHideCursor();
    }
    
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.setCameraTiltAngle(0);
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key=='f'){
        ofToggleFullscreen();
        easyCam.reset();
        resetParticle();
        ofHideCursor();
    }
    
    if(key == ' '){
        bLearnBackground = !bLearnBackground;
    }
    
    if(key =='z'){
        kinect.setCameraTiltAngle(0);
    }
    
    if (key== OF_KEY_UP){
        kinectAngle++;
        if(kinectAngle>30) kinectAngle=30;
        kinect.setCameraTiltAngle(kinectAngle);
    }
    
    if(key==OF_KEY_DOWN){
        kinectAngle--;
        if(kinectAngle<-30) kinectAngle=-30;
        kinect.setCameraTiltAngle(kinectAngle);
    }
    
    if(key == 'g'){
        bGui = !bGui;
    }
    
    if(key == 'c'){
        bShowCv = ! bShowCv;
    }
    
    if(key == 'r'){
        resetParticle();
    }
    
    if(key == '-'){
        oscFreq-- ;
        if(oscFreq<1)oscFreq = 1;
        cout<<"oscFerq is: "<<oscFreq<<endl;
    }
    
    if(key == '+'){
        oscFreq++ ;
        if(oscFreq>20)oscFreq = 20;
        cout<<"oscFerq is: "<<oscFreq<<endl;
    }
    
    if(key == '0'){
        bLED = !bLED;
    }
}

//--------------------------------------------------------------
void ofApp::resetParticle(){
    for(int i=0; i<NUMPART; i++){
        p[i].setup();
    }
}

//--------------------------------------------------------------
void ofApp::updateMesh(){
    //these three for jitter
    float time = ofGetElapsedTimef();
    float timeScale = 5.0;
    //    float displacementScale= 1.5;
    
    
    
    //generate mesh
    mesh.clear();
    
    ofPixels pixels = kinect.getDepthPixels();
    pixels.mirror(false, true);
    pixels.resize(rw, rh);
        
    //filter out and update the vertices_faster
    for(auto line: pixels.getLines()){
        auto x = 0;
        for(auto pixel : line.getPixels()){
            ofColor c = pixel.getColor();

            float lightness = c.getLightness();
            //                if(lightness>lightnessMin){
            if(lightness >= lightnessMin && lightness <=lightnessMax){
                float saturation = c.getSaturation();
                float z = ofMap(saturation, 0, 255, 400, -400);
                ofVec3f pos(x * resizeScale * ofGetWidth()/width, line.getLineNum() * resizeScale * ofGetHeight()/height, z);

                c.setSaturation(mSaturation);
                mesh.addVertex(pos);
                mesh.addColor(c);
            }
            x++;
        }
    }
    
    //draw lines if they are within certain distance
    
    int numVerts = mesh.getNumVertices();
    for (int a=0; a<numVerts; a++) {
        ofVec3f verta = mesh.getVertex(a);
        
        //draw lines if they are within certain distance
        for (int b=a+1; b<a+8; b++) {
            ofVec3f vertb = mesh.getVertex(b);
            //                float distance = verta.squareDistance(vertb);
            float distance = verta.distance(vertb);
            if (distance <= connectionDistance) {
                mesh.addIndex(a);
                mesh.addIndex(b);
            }
        }
        
        //jitter
        ofVec3f offset = offsets[a];
        verta.x += (ofSignedNoise(time * timeScale + offset.x)) * displacementScale;
        verta.y += (ofSignedNoise(time * timeScale + offset.y)) * displacementScale;
        verta.z += (ofSignedNoise(time * timeScale + offset.z)) * displacementScale;
        mesh.setVertex(a, verta);
    }
    
    //    cout<<mesh.getNumVertices()<<endl;
}
//--------------------------------------------------------------
void ofApp::change_resizeScale(int & scale){
    //    rw = ow/scale; rh = oh/scale; //video
    rw = width/scale; rh = height/scale;
    offsets.resize(rw * rh);
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
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    fbo.allocate(w, h, GL_RGBA32F_ARB);
    ofHideCursor();

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
