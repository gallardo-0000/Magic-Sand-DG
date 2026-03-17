/***********************************************************************
ofApp.h - main openframeworks app
Copyright (c) 2016-2017 Thomas Wolf and Rasmus R. Paulsen (people.compute.dtu.dk/rapa)

This file is part of the Magic Sand.

The Magic Sand is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Magic Sand is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Augmented Reality Sandbox; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#pragma once

#include "ofMain.h"
#include "KinectProjector/KinectProjector.h"
#include "SandSurfaceRenderer/SandSurfaceRenderer.h"
#include "Games/MapGameController.h"
#include "Games/BoidGameController.h"
#include "RainSimulation.h" 

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();
    void drawProjWindow(ofEventArgs& args);
    void keyPressed(int key);

    std::shared_ptr<ofAppBaseWindow> projWindow;

private:
    // --- SISTEMA DE LLUVIA ---
    RainSimulation rainSim;
    bool rainActive = false;
    bool localRain = false;

    const int simWidth = 160;
    const int simHeight = 120;
    std::vector<float> terrain;

    float rainX = 80;
    float rainY = 60;
    float rainRadius = 15.0;

    // --- COMPONENTES CORE ---
    std::shared_ptr<KinectProjector> kinectProjector;
    SandSurfaceRenderer* sandSurfaceRenderer;
    CMapGameController mapGameController;
    CBoidGameController boidGameController;
    
    ofRectangle mainWindowROI;
};
