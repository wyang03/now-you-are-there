//
//  particle.cpp
//  anotherParticle
//
//  Created by Wei Yang on 9/21/23.
//
//

#include "particle.h"

particle:: particle(){
//    image.load("hand1.png");
}

void particle::setup(){
    pos.x  = ofRandomWidth();
    pos.y = ofRandomHeight();
    
    vel.x = ofRandom(-4, 4);
    vel.y = ofRandom(-4, 4);
    
    frc = ofVec2f(0);
    
    uniqueVal.x = ofRandom(-1000, 1000);
    uniqueVal.y = ofRandom(-1000, 1000);
    
    uniqueCol = ofRandom(0, 4);
    
    size = ofRandom(3, 5);
//    size = ofRandom(50, 25);
    
    drag = ofRandom(0.97, 0.99);
}

void particle::update(float speed, float noise, bool repel){
    frc.x = ofSignedNoise(uniqueVal.x, ofGetElapsedTimeMillis());
    frc.y = ofSignedNoise(uniqueVal.y, ofGetElapsedTimeMillis());
    
    frc *= noise;  //noise control added to frc
    
    vel *= drag; //smooth decrease
    
    if(repel){
        vel -= repelFrc; //take into consideration of repel, change minus to add for attraction, try!!
    } else {
        vel += repelFrc;
    }
    
    vel += frc;
    
    pos += vel * speed ; //speed control added to vel
    
    //reset at border
    if(pos.x > ofGetWidth()){
        pos.x= 0;
    } else if (pos.x < 0){
        pos.x = ofGetWidth();
    }
    
    if(pos.y > ofGetHeight()){
        pos.y = 0;
    } else if (pos.y < 0){
        pos.y = ofGetHeight();
    }
}

void particle::draw(float sizeDot){
    ofDrawCircle(pos.x, pos.y, size * sizeDot);
}

void particle::repel(vector<ofVec2f>repelPt){
    repelFrc.set(0, 0);
    
    for(int i=0; i<repelPt.size(); i++){
        float dist = pos.distance(repelPt[i]);
        if(dist < 150){
            ofVec2f newRepelPt;
            newRepelPt = repelPt[i] - pos; //to get the vector direction
            repelFrc += newRepelPt * 0.0005; //add repelFrc to the vector direction, scale it down
        }
    }
}
