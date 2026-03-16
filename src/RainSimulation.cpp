#include "RainSimulation.h"
#include <algorithm>
#include <cmath>

int RainSimulation::index(int x,int y) const{
    return y*width + x;
}

void RainSimulation::setup(int w,int h){

    width = w;
    height = h;

    terrainHeight.assign(width*height,0);
    water.assign(width*height,0);
    nextWater.assign(width*height,0);
}

void RainSimulation::addRain(float amount){

    for(int i=0;i<water.size();i++){
        water[i]+=amount;
    }
}

void RainSimulation::addLocalizedRain(int cx,int cy,float radius,float amount){

    float r2 = radius*radius;

    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){

            float dx = x-cx;
            float dy = y-cy;

            float d = dx*dx+dy*dy;

            if(d<r2){

                float factor = 1.0-(d/r2);

                water[index(x,y)] += amount*factor;

            }
        }
    }
}

void RainSimulation::clearWater(){

    std::fill(water.begin(),water.end(),0);
}

const std::vector<float>& RainSimulation::getWater() const{

    return water;
}

void RainSimulation::update(const std::vector<float>& terrain){

    terrainHeight = terrain;

    nextWater = water;

    for(int y=1;y<height-1;y++){
        for(int x=1;x<width-1;x++){

            int i = index(x,y);

            float surface = terrainHeight[i]+water[i];

            float available = water[i];

            int dx[4]={1,-1,0,0};
            int dy[4]={0,0,1,-1};

            for(int k=0;k<4;k++){

                int nx = x+dx[k];
                int ny = y+dy[k];

                int ni = index(nx,ny);

                float neighborSurface = terrainHeight[ni]+water[ni];

                float diff = surface-neighborSurface;

                if(diff>0 && available>0){

                    float flow = std::min(available,diff*flowSpeed*0.25f);

                    nextWater[i]-=flow;

                    nextWater[ni]+=flow;

                    available-=flow;

                }

            }

        }
    }

    for(int i=0;i<nextWater.size();i++){

        nextWater[i]-=infiltration;

        if(nextWater[i]<0) nextWater[i]=0;

    }

    water = nextWater;
}
