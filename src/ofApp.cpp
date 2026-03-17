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
    // 1. Configuración de la ventana y rendimiento
    ofSetFrameRate(60);
    ofBackground(0);
    ofSetVerticalSync(true);
    
    // 2. Inicializar Kinect y Renderizador (Uso de smart pointers recomendado)
    kinectProjector = std::make_shared<KinectProjector>(projWindow);
    kinectProjector->setup(true);
    
    sandSurfaceRenderer = new SandSurfaceRenderer(kinectProjector, projWindow);
    sandSurfaceRenderer->setup(true);
    
    // 3. Preparar simulación de lluvia y terreno
    // Aseguramos que el vector tenga el tamaño correcto desde el inicio
    terrain.assign(simWidth * simHeight, 0.0f);
    rainSim.setup(simWidth, simHeight); // Inicialización interna de la lluvia

    // 4. Configuración de ROI (Región de Interés)
    ofVec2f kinectRes = kinectProjector->getKinectRes();
    mainWindowROI = ofRectangle((ofGetWindowWidth()-kinectRes.x)/2, 
                                 (ofGetWindowHeight()-kinectRes.y)/2, 
                                 kinectRes.x, kinectRes.y);

    mapGameController.setup(kinectProjector);
    boidGameController.setup(kinectProjector);
    
    // Inicializar estados
    rainActive = false;
    localRain = false;
    rainRadius = 15.0f;
}

void ofApp::update() {
    kinectProjector->update();
    sandSurfaceRenderer->update();

    // 5. Actualizar mapa de terreno con protección de datos
    for(int y=0; y<simHeight; y++){
        for(int x=0; x<simWidth; x++){
            int i = y*simWidth + x;
            float h = sandSurfaceRenderer->getHeightAt(x, y);
            
            // Protección: Si el sensor devuelve un valor no válido (NaN), lo tratamos como suelo
            terrain[i] = (h == h) ? h : 0.0f; 
        }
    }

    // 6. Lógica de seguimiento de mano para lluvia
    if(kinectProjector && kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        ofVec2f handPos = kinectProjector->getHandPosition();
        
        // Mapeo preciso: Convierte la coordenada del sensor a la de la simulación
        rainX = ofMap(handPos.x, 0, kinectRes.x, 0, simWidth, true);
        rainY = ofMap(handPos.y, 0, kinectRes.y, 0, simHeight, true);
    }

    // 7. Ejecutar simulación física
    if(rainActive) rainSim.addRain(0.012f); // Lluvia global ligera
    if(localRain)  rainSim.addLocalizedRain(rainX, rainY, rainRadius, 0.2f); // Lluvia fuerte en mano

    rainSim.update(terrain);

    mapGameController.update();
    boidGameController.update();
}

void ofApp::drawProjWindow(ofEventArgs &args) {
    if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_RUNNING){
        
        // Dibujar el mapa de colores de la arena primero
        sandSurfaceRenderer->drawProjectorWindow();
        
        // 8. Capa de AGUA (Renderizado mejorado)
        const std::vector<float>& water = rainSim.getWater();
        float cellW = (float)ofGetWindowWidth() / simWidth;
        float cellH = (float)ofGetWindowHeight() / simHeight;

        ofEnableAlphaBlending();
        for(int y=0; y<simHeight; y++){
            for(int x=0; x<simWidth; x++){
                float w = water[y*simWidth + x];
                if(w > 0.005f){ // Umbral para no dibujar gotas invisibles
                    
                    // El color se vuelve más opaco y oscuro cuanto más profunda es el agua
                    float opacity = ofMap(w, 0, 0.5, 80, 230, true);
                    ofSetColor(0, 130, 255, opacity);
                    
                    // Dibujamos rectángulos para una capa de agua continua
                    ofDrawRectangle(x*cellW, y*cellH, cellW + 0.5f, cellH + 0.5f);
                }
            }
        }
        ofDisableAlphaBlending();
        
        // Dibujar elementos de juego encima del agua
        mapGameController.drawProjectorWindow();
        boidGameController.drawProjectorWindow();
    }
}

void ofApp::keyPressed(int key) {
    // Controles de usuario
    if(key == 'b' || key == 'B') rainActive = !rainActive;
    if(key == 'n' || key == 'N') localRain = !localRain;
    if(key == 'v' || key == 'V') rainSim.clearWater();
    
    // Control de calibración/inicio
    if(key == ' '){
        if(kinectProjector->GetApplicationState() == KinectProjector::APPLICATION_STATE_SETUP){
            kinectProjector->startApplication();
        }
    }
}
