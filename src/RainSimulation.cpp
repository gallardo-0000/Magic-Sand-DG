#include "RainSimulation.h"
#include <algorithm>

int RainSimulation::index(int x, int y) const {
    return y * width + x;
}

void RainSimulation::setup(int w, int h) {
    width = w;
    height = h;
    terrain.assign(w * h, 0.0f);
    water.assign(w * h, 0.0f);
    nextWater.assign(w * h, 0.0f);
}

void RainSimulation::addRain(float amount) {
    for (int i = 0; i < (int)water.size(); i++) {
        water[i] += amount;
    }
}

void RainSimulation::clearWater() {
    std::fill(water.begin(), water.end(), 0.0f);
}

const std::vector<float>& RainSimulation::getWater() const {
    return water;
}

void RainSimulation::update(const std::vector<float>& terrainHeights) {
    if ((int)terrainHeights.size() != width * height) return;

    terrain = terrainHeights;
    nextWater = water;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int i = index(x, y);
            float currentSurface = terrain[i] + water[i];
            float available = water[i];

            const int dx[4] = {1, -1, 0, 0};
            const int dy[4] = {0, 0, 1, -1};

            for (int k = 0; k < 4; k++) {
                int nx = x + dx[k];
                int ny = y + dy[k];
                int ni = index(nx, ny);

                float neighborSurface = terrain[ni] + water[ni];
               
