//
//  particle.h
//  anotherParticle
//
//  Created by Wei Yang on 9/21/23.
//
//

#ifndef particle_h
#define particle_h

#include "ofMain.h"

class particle
{
public:
    particle(); //constructor
    
    //methods
    void setup();
    void update(float speed, float noise, bool repel);
    void draw(float sizeDot);
    void repel(vector<ofVec2f>repelPt); //the ofVec2f points data is from the silhouette captured via openCv
    
    //vaiables
    ofVec2f pos;
    ofVec2f vel;
    ofVec2f frc;
    ofVec2f repelFrc;
    float drag; //counter the frc
    ofImage image;
   
    ofVec2f uniqueVal; //each particle has an unique value to use with ofNoise for its motion
    float size;
    int uniqueCol; //color selector
    
};

#endif /* particle_h */
