#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxPostProcessing.h"
#include "ofxOsc.h"

#include "particle.h"

#define NUMPART 3500
#define HOST "localhost"
#define SENDPORT 12345
#define LISPORT 12344

class circularPixelBuffer{
public:
    circularPixelBuffer(){
        currentIndex = 0;
    }
    void setup(int numFrames){
        frames.resize(numFrames);
        currentIndex = numFrames -1;
    }
    void pushPixels(ofPixels& pix){
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = frames.size() -1;
        }
        frames[currentIndex] = pix;
    }
    
    ofPixels& getDelayedPixels(size_t delay){
        if(delay < frames.size()){
            return frames[ofWrap(delay + currentIndex, 0, frames.size())];
        }
        return frames[0];
    }
    
protected:
    int currentIndex;
    vector<ofPixels> frames;
};


class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;

    //particles
    vector<particle> p;
    void resetParticle();
    ofxIntSlider pBrightness;
    ofxToggle bRepel;
    ofxToggle bDrawParticle;

    
    //guis
    bool bGui;
    ofxPanel gui;
    ofxFloatSlider overallSpeed;
    ofxFloatSlider noiseAmount;
    ofxFloatSlider trail;
    ofxFloatSlider sizeDot;
    ofxColorSlider color[4];

    
    //background video player
    ofVideoPlayer video;
    ofxIntSlider ghostScale; 
//    ofVideoGrabber webcam;
    ofxToggle bDistMod;
    float zDev;
    
    //background sound
    ofSoundPlayer breath;
    ofxFloatSlider breathVol;
    
    //kinect input
    ofxKinect kinect;
    int width, height, kinectAngle;
    ofxFloatSlider videoVol;
    bool bLED; 
    
    
    
    //openCv
    bool bShowCv;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImg;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    ofxCvContourFinder contourFinder;
    ofxIntSlider blur;
    
    //more gui for opencCv
    ofxIntSlider threshold;
    ofxIntSlider maxBlobNum;
    ofxIntSlider maxArea;
    ofxIntSlider minArea;
    ofxToggle bLearnBackground;
    ofxToggle bDilate;
    ofxToggle bErode;
    
    
    //fbo
    ofFbo fbo;
    
    //mesh
    ofMesh mesh;
    int ow, oh, rw, rh;
    ofxIntSlider resizeScale;
    vector<ofVec3f> offsets; //for jittering
    void updateMesh();
    ofxIntSlider lineWidth;
    
    //listen to resizeScale
    void change_resizeScale(int & scale);
    
    //more gui for mesh
    ofxIntSlider lightnessMin;
    ofxIntSlider lightnessMax;
    ofxIntSlider connectionDistance;
    ofxFloatSlider displacementScale;
    ofxIntSlider mSaturation;
    
    //easy cam
    ofEasyCam easyCam;
    
    //effects
    ofxPostProcessing effects;
    
    //osc communication
    ofxOscSender sender;
    ofxOscReceiver receiver; 
    int oscFreq;
    //~mlOnsets, ~mlAmp, ~mlFreq, ~mlEntropy
    float amp;
    float freq;
    float entropy;
    int onsets;
    
    //video delay
    int nDelayFrames;
    circularPixelBuffer buffer;
    ofTexture tex;
};
