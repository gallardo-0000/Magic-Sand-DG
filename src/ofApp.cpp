/***********************************************************************
ofApp.cpp - main openframeworks app
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

#include "ofApp.h"

void ofApp::setup() {
	// OF basics
	ofSetFrameRate(60);
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetLogLevel("ofThread", OF_LOG_WARNING);
	ofSetLogLevel("ofFbo", OF_LOG_ERROR);
	ofSetLogLevel("ofShader", OF_LOG_ERROR);
	ofSetLogLevel("ofxKinect", OF_LOG_WARNING);

	// Setup kinectProjector
	kinectProjector = std::make_shared<KinectProjector>(projWindow);
	kinectProjector->setup(true);
	
	// Setup sandSurfaceRenderer
	sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
	sandSurfaceRenderer->setup(true);
	
	// Retrieve variables
	ofVec2f kinectRes = kinectProjector->getKinectRes();
	ofVec2f projRes = ofVec2f(projWindow->getWidth(), projWindow->getHeight());
	ofRectangle kinectROI = kinectProjector->getKinectROI();
	////mainWindowROI = ofRectangle(600, 30, 600, 450);
	//mainWindowROI = ofRectangle(0, 0, 640, 480);
	mainWindowROI = ofRectangle((ofGetWindowWidth()-kinectRes.x)/2, (ofGetWindowHeight()-kinectRes.y)/2, kinectRes.x, kinectRes.y);

	mapGameController.setup(kinectProjector);
	mapGameController.setProjectorRes(projRes);
	mapGameController.setKinectRes(kinectRes);
	mapGameController.setKinectROI(kinectROI);

	boidGameController.setup(kinectProjector);
	boidGameController.setProjectorRes(projRes);
	boidGameController.setKinectRes(kinectRes);
	boidGameController.setKinectROI(kinectROI);

}


void ofApp::update() {
kinectProjector->update();
sandSurfaceRenderer->update();

// --- SIMULACIÓN DE LLUVIA ---
for(int y=0; y<simHeight; y++){
    for(int x=0; x<simWidth; x++){
        int i = y*simWidth + x;
        // terreno real si tu SandSurfaceRenderer tiene función getHeightAt
        terrain[i] = sandSurfaceRenderer->getHeightAt(x, y); 
        // Si no tienes getHeightAt, usa esto de prueba:
        // terrain[i] = 0.02*y;
    }
}

// --- POSICIÓN DE LA LLUVIA CON MANO ---
if(kinectProjector){
    ofVec2f handPos = kinectProjector->getHandPosition();
    rainX = handPos.x / sandSurfaceRenderer->getWidth() * simWidth;
    rainY = handPos.y / sandSurfaceRenderer->getHeight() * simHeight;
}

// --- LLUVIA ---
if(rainActive) rainSim.addRain(0.01);           // global
if(localRain) rainSim.addLocalizedRain(rainX, rainY, rainRadius, 0.05); // localizada

rainSim.update(terrain);

// --- Resto de tus controles de juegos ---
if(kinectProjector->getKinectROI() != mapGameController.getKinectROI()){
    ofRectangle kinectROI = kinectProjector->getKinectROI();
    mapGameController.setKinectROI(kinectROI);
    boidGameController.setKinectROI(kinectROI);
}

mapGameController.update();
boidGameController.update();
}


void ofApp::draw() 
{
	float x = mainWindowROI.x;
	float y = mainWindowROI.y;
	float w = mainWindowROI.width;
	float h = mainWindowROI.height;

	if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING)
	{
		sandSurfaceRenderer->drawMainWindow(x, y, w, h);//400, 20, 400, 300);
		boidGameController.drawMainWindow(x, y, w, h);
	}

	kinectProjector->drawMainWindow(x, y, w, h);
}

void ofApp::drawProjWindow(ofEventArgs &args) 
{
	if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
    sandSurfaceRenderer->drawProjectorWindow();
    mapGameController.drawProjectorWindow();
    boidGameController.drawProjectorWindow();
}

// --- DIBUJAR LLUVIA ---
const std::vector<float>& water = rainSim.getWater();
float cellW = ofGetWidth()/simWidth;
float cellH = ofGetHeight()/simHeight;

ofEnableAlphaBlending();
ofEnableSmoothing();

for(int y=0; y<simHeight; y++){
    for(int x=0; x<simWidth; x++){
        int i = y*simWidth + x;
        float w = water[i];
        if(w>0.001){
            ofSetColor(0, 120, 255, ofMap(w, 0, 1, 50, 200));
            float radius = ofMap(w, 0, 1, 2, 6);
            ofDrawCircle(x*cellW + cellW/2, y*cellH + cellH/2, radius);
        }
    }
}

ofSetColor(255);

kinectProjector->drawProjectorWindow();
}

void ofApp::keyPressed(int key) 
{
    // --- CONTROLES EXISTENTES DE KINECT Y JUEGOS ---
    if (key == 'c')
    {
        kinectProjector->SaveKinectColorImage();
    }
    else if (key == 'd')
    {
        kinectProjector->SaveFilteredDepthImage();
    }
    else if (key == ' ')
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && 
            boidGameController.isIdle())
        {
            if (mapGameController.isIdle())
            {
                mapGameController.setDebug(kinectProjector->getDumpDebugFiles());
                mapGameController.StartGame();
            }
            else
            {
                mapGameController.ButtonPressed();
            }
        }
        else if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_SETUP)
        {
            kinectProjector->startApplication();
        }
    }
    else if (key == 'f' || key == 'r')
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING)
        {
            if (mapGameController.isIdle())
            {
                boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
                boidGameController.StartGame(2);
            }
            else 
            {
                mapGameController.EndButtonPressed();
            }
        }
    }
    else if (key == '1') // Absolute beginner
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && mapGameController.isIdle())
        {
            boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
            boidGameController.StartGame(0);
        }
    }
    else if (key == '2') 
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && mapGameController.isIdle())
        {
            boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
            boidGameController.StartGame(1);
        }
    }
    else if (key == '3')
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && mapGameController.isIdle())
        {
            boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
            boidGameController.StartGame(2);
        }
    }
    else if (key == '4')
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && mapGameController.isIdle())
        {
            boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
            boidGameController.StartGame(3);
        }
    }
    else if (key == 'm')
    {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING && mapGameController.isIdle())
        {
            boidGameController.setDebug(kinectProjector->getDumpDebugFiles());
            boidGameController.StartSeekMotherGame();
        }
    }
    else if (key == 't')
    {
        mapGameController.setDebug(kinectProjector->getDumpDebugFiles());
        mapGameController.RealTimeTestMe();
    }
    else if (key == 'w')
    {
        mapGameController.setDebug(kinectProjector->getDumpDebugFiles());
        mapGameController.DebugTestMe();
    }

    // --- NUEVOS CONTROLES DE LLUVIA ---
    if(key == 'b'){      // alterna lluvia global
        rainActive = !rainActive;
    }

    if(key == 'n'){      // alterna tormenta localizada sobre la mano
        localRain = !localRain;
    }

    if(key == 'v'){      // limpiar toda el agua
        rainSim.clearWater();
    }

    // mover tormenta manualmente con flechas
    if(key == OF_KEY_LEFT)  rainX -= 2;
    if(key == OF_KEY_RIGHT) rainX += 2;
    if(key == OF_KEY_UP)    rainY -= 2;
    if(key == OF_KEY_DOWN)  rainY += 2;
}
