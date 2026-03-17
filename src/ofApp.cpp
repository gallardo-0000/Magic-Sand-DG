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
    
    // Inicializar Kinect y Renderizador
    kinectProjector = std::make_shared<KinectProjector>(projWindow);
    kinectProjector->setup(true);
    sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
    sandSurfaceRenderer->setup(true);
    
    // Preparar el contenedor del terreno
    terrain.assign(simWidth * simHeight, 0.0f);

    // Configuración de ROI y Juegos
    ofVec2f kinectRes = kinectProjector->getKinectRes();
    mainWindowROI = ofRectangle((ofGetWindowWidth()-kinectRes.x)/2, (ofGetWindowHeight()-kinectRes.y)/2, kinectRes.x, kinectRes.y);

    mapGameController.setup(kinectProjector);
    boidGameController.setup(kinectProjector);
}

void ofApp::update() {
    kinectProjector->update();
    sandSurfaceRenderer->update();

    // 1. Actualizar el mapa de terreno desde el Kinect
    for(int y=0; y<simHeight; y++){
        for(int x=0; x<simWidth; x++){
            int i = y*simWidth + x;
            // Si tu clase SandSurfaceRenderer no tiene getHeightAt, 
            // asegúrate de implementarla o usa un valor fijo de 0.0 para probar.
            terrain[i] = sandSurfaceRenderer->getHeightAt(x, y); 
        }
    }

    // 2. Seguir la mano para la lluvia localizada
    if(kinectProjector && kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        ofVec2f handPos = kinectProjector->getHandPosition();
        // Convertimos la posición del Kinect a la escala de la simulación
        rainX = ofMap(handPos.x, 0, kinectProjector->getKinectRes().x, 0, simWidth);
        rainY = ofMap(handPos.y, 0, kinectProjector->getKinectRes().y, 0, simHeight);
    }

    // 3. Aplicar física de lluvia
    if(rainActive) rainSim.addRain(0.01);
    if(localRain)  rainSim.addLocalizedRain(rainX, rainY, rainRadius, 0.15);

    rainSim.update(terrain);

    mapGameController.update();
    boidGameController.update();
}

void ofApp::drawProjWindow(ofEventArgs &args) {
    if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        sandSurfaceRenderer->drawProjectorWindow();
        
        // --- DIBUJAR EL AGUA ---
        const std::vector<float>& water = rainSim.getWater();
        float cellW = (float)ofGetWindowWidth() / simWidth;
        float cellH = (float)ofGetWindowHeight() / simHeight;

        ofEnableAlphaBlending();
        for(int y=0; y<simHeight; y++){
            for(int x=0; x<simWidth; x++){
                float w = water[y*simWidth + x];
                if(w > 0.001){
                    // Color azul con transparencia basada en la cantidad de agua
                    ofSetColor(0, 120, 255, ofMap(w, 0, 1, 60, 220));
                    ofDrawCircle(x*cellW + cellW/2, y*cellH + cellH/2, ofMap(w, 0, 1, 2, 7));
                }
            }
        }
        ofDisableAlphaBlending();
        ofSetColor(255);
    }
}

void ofApp::keyPressed(int key) {
    // Controles de lluvia
    if(key == 'b' || key == 'B') rainActive = !rainActive;
    if(key == 'n' || key == 'N') localRain = !localRain;
    if(key == 'v' || key == 'V') rainSim.clearWater();
    
    // Iniciar aplicación si está en espera
    if(key == ' ' && kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_SETUP){
        kinectProjector->startApplication();
    }
}
