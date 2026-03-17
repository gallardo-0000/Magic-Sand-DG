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
    
    // 1. Inicializar Kinect y Renderizador
    kinectProjector = std::make_shared<KinectProjector>(projWindow);
    kinectProjector->setup(true);
    
    sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
    sandSurfaceRenderer->setup(true);
    
    // 2. Preparar simulación y terreno (Asegúrate que simWidth=160 y simHeight=120 en el .h)
    terrain.assign(simWidth * simHeight, 0.0f);
    // Nota: Si tu RainSimulation tiene un setup(), llámalo aquí: rainSim.setup(simWidth, simHeight);

    // 3. Configurar dimensiones para los juegos
    ofVec2f kRes = kinectProjector->getKinectRes();
    mainWindowROI = ofRectangle((ofGetWindowWidth()-kRes.x)/2, (ofGetWindowHeight()-kRes.y)/2, kRes.x, kRes.y);

    mapGameController.setup(kinectProjector);
    boidGameController.setup(kinectProjector);
    
    // 4. Inicializar variables de control
    rainActive = false;
    localRain = false;
    rainRadius = 15.0f;
}

void ofApp::update() {
    kinectProjector->update();
    sandSurfaceRenderer->update();

    // 5. Actualizar terreno (Protección contra valores basura del sensor)
    for(int y=0; y<simHeight; y++){
        for(int x=0; x<simWidth; x++){
            int i = y*simWidth + x;
            float h = sandSurfaceRenderer->getHeightAt(x, y);
            terrain[i] = (h == h) ? h : 0.0f; // Si h es NaN, ponemos 0
        }
    }

    // 6. Mapeo de la mano (CORREGIDO: usando kinectProjector->getKinectRes())
    if(kinectProjector && kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        ofVec2f handPos = kinectProjector->getHandPosition();
        ofVec2f kRes = kinectProjector->getKinectRes();
        
        // El "true" al final evita que el agua se salga de los límites si la mano sale de pantalla
        rainX = ofMap(handPos.x, 0, kRes.x, 0, simWidth, true);
        rainY = ofMap(handPos.y, 0, kRes.y, 0, simHeight, true);
    }

    // 7. Aplicar física de lluvia
    if(rainActive) rainSim.addRain(0.012f); 
    if(localRain)  rainSim.addLocalizedRain(rainX, rainY, rainRadius, 0.2f); 

    rainSim.update(terrain);

    mapGameController.update();
    boidGameController.update();
}

void ofApp::drawProjWindow(ofEventArgs &args) {
    if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        
        // Dibujar arena
        sandSurfaceRenderer->drawProjectorWindow();
        
        // 8. Dibujar Agua con Renderizado Eficiente
        const std::vector<float>& water = rainSim.getWater();
        float cellW = (float)ofGetWindowWidth() / simWidth;
        float cellH = (float)ofGetWindowHeight() / simHeight;

        ofEnableAlphaBlending();
        for(int y=0; y<simHeight; y++){
            for(int x=0; x<simWidth; x++){
                float w = water[y*simWidth + x];
                if(w > 0.005f){ 
                    // Color azul más profundo según la cantidad de agua
                    float blueLevel = ofMap(w, 0, 0.5, 180, 255, true);
                    float alpha = ofMap(w, 0, 0.5, 100, 240, true);
                    
                    ofSetColor(0, 120, blueLevel, alpha);
                    // Rectángulos ligeramente más grandes (+0.5) para evitar líneas negras entre celdas
                    ofDrawRectangle(x*cellW, y*cellH, cellW + 0.5f, cellH + 0.5f);
                }
            }
        }
        ofDisableAlphaBlending();
        
        // Dibujar juegos (Peces, etc) para que se vean SOBRE el agua
        mapGameController.drawProjectorWindow();
        boidGameController.drawProjectorWindow();
    }
}

void ofApp::keyPressed(int key) {
    if(key == 'b' || key == 'B') rainActive = !rainActive;
    if(key == 'n' || key == 'N') localRain = !localRain;
    if(key == 'v' || key == 'V') rainSim.clearWater();
    
    if(key == ' '){
        if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_SETUP){
            kinectProjector->startApplication();
        }
    }
}
