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

/***********************************************************************
ofApp.cpp - Edición Mejorada para Lluvia Localizada
***********************************************************************/

#include "ofApp.h"

void ofApp::setup() {
    ofSetFrameRate(60);
    ofBackground(0);
    ofSetVerticalSync(true);
    
    // Setup kinectProjector
    kinectProjector = std::make_shared<KinectProjector>(projWindow);
    kinectProjector->setup(true);
    
    // Setup sandSurfaceRenderer
    sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
    sandSurfaceRenderer->setup(true);
    
    ofVec2f kinectRes = kinectProjector->getKinectRes();
    ofVec2f projRes = ofVec2f(projWindow->getWidth(), projWindow->getHeight());
    ofRectangle kinectROI = kinectProjector->getKinectROI();

    mainWindowROI = ofRectangle((ofGetWindowWidth()-kinectRes.x)/2, (ofGetWindowHeight()-kinectRes.y)/2, kinectRes.x, kinectRes.y);

    mapGameController.setup(kinectProjector);
    mapGameController.setProjectorRes(projRes);
    mapGameController.setKinectRes(kinectRes);
    mapGameController.setKinectROI(kinectROI);

    boidGameController.setup(kinectProjector);
    boidGameController.setProjectorRes(projRes);
    boidGameController.setKinectRes(kinectRes);
    boidGameController.setKinectROI(kinectROI);

    // Inicializar variables de lluvia si no están en el .h
    rainActive = false;
    localRain = false;
    rainRadius = 15.0; 
}

void ofApp::update() {
    kinectProjector->update();
    sandSurfaceRenderer->update();

    // --- SIMULACIÓN DE TERRENO ---
    for(int y=0; y<simHeight; y++){
        for(int x=0; x<simWidth; x++){
            int i = y*simWidth + x;
            // Intentar obtener altura real, si falla usamos plano 0
            try {
                terrain[i] = sandSurfaceRenderer->getHeightAt(x, y);
            } catch (...) {
                terrain[i] = 0.0f; 
            }
        }
    }

    // --- POSICIÓN DE LA LLUVIA CON MANO (CORREGIDO) ---
    if(kinectProjector && kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        ofVec2f handPos = kinectProjector->getHandPosition(); 
        // Mapeo preciso a la resolución de la simulación
        rainX = ofMap(handPos.x, 0, kinectProjector->getKinectRes().x, 0, simWidth);
        rainY = ofMap(handPos.y, 0, kinectProjector->getKinectRes().y, 0, simHeight);
    }

    // --- LÓGICA DE LLUVIA ---
    if(rainActive) {
        rainSim.addRain(0.02); // Lluvia constante
    }
    
    if(localRain) {
        // Lluvia fuerte donde está la mano
        rainSim.addLocalizedRain(rainX, rainY, rainRadius, 0.2); 
    }

    rainSim.update(terrain);

    // Actualizar controladores de juegos
    mapGameController.update();
    boidGameController.update();
}

void ofApp::draw() {
    float x = mainWindowROI.x;
    float y = mainWindowROI.y;
    float w = mainWindowROI.width;
    float h = mainWindowROI.height;

    if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING) {
        sandSurfaceRenderer->drawMainWindow(x, y, w, h);
        boidGameController.drawMainWindow(x, y, w, h);
    }
    kinectProjector->drawMainWindow(x, y, w, h);
}

void ofApp::drawProjWindow(ofEventArgs &args) {
    if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        sandSurfaceRenderer->drawProjectorWindow();
        mapGameController.drawProjectorWindow();
        boidGameController.drawProjectorWindow();
        
        // --- DIBUJAR AGUA EN EL PROYECTOR ---
        const std::vector<float>& water = rainSim.getWater();
        float cellW = (float)ofGetWindowWidth() / simWidth;
        float cellH = (float)ofGetWindowHeight() / simHeight;

        ofEnableAlphaBlending();
        for(int y=0; y<simHeight; y++){
            for(int x=0; x<simWidth; x++){
                int i = y*simWidth + x;
                float w = water[i];
                if(w > 0.005){
                    // Color Azul Eléctrico para el agua
                    ofSetColor(0, 150, 255, ofMap(w, 0, 1, 80, 220));
                    ofDrawRectangle(x*cellW, y*cellH, cellW, cellH);
                }
            }
        }
        ofDisableAlphaBlending();
    }
    kinectProjector->drawProjectorWindow();
}

void ofApp::keyPressed(int key) {
    // Teclas estándar
    if (key == 'c') kinectProjector->SaveKinectColorImage();
    if (key == ' ') {
        if (kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_SETUP) {
            kinectProjector->startApplication();
        }
    }

    // --- CONTROLES DE LLUVIA ---
    if(key == 'b' || key == 'B') rainActive = !rainActive; // Lluvia Global
    if(key == 'n' || key == 'N') localRain = !localRain;   // Lluvia en Mano
    if(key == 'v' || key == 'V') rainSim.clearWater();    // Secar todo

    // Ajuste manual de radio de lluvia
    if(key == '+') rainRadius += 2;
    if(key == '-') rainRadius -= 2;
}
