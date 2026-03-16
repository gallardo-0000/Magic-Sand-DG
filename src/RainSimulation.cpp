#include "RainSimulation.h"
#include <algorithm>
#include <cmath>

int RainSimulation::index(int x, int y) const {
    return y * width + x;
}

void RainSimulation::setup(int w, int h) {
    width = w;
    height = h;

    terrain.assign(width * height, 0.0f);
    water.assign(width * height, 0.0f);
    nextWater.assign(width * height, 0.0f);
}

void RainSimulation::addRain(float amount) {
    for (int i = 0; i < (int)water.size(); i++) {
        water[i] += amount;
    }
}

void RainSimulation::addLocalizedRain(int cx, int cy, float radius, float amount) {
    float r2 = radius * radius;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = (float)x - cx;
            float dy = (float)y - cy;
            float d2 = dx * dx + dy * dy;

            if (d2 <= r2) {
                float falloff = 1.0f - (d2 / r2);
                water[index(x, y)] += amount * falloff;
            }
        }
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

            if (water[i] <= 0.0f) continue;

            float currentSurface = terrain[i] + water[i];
            float available = water[i];

            const int dx[4] = {1, -1, 0, 0};
            const int dy[4] = {0, 0, 1, -1};

            for (int k = 0; k < 4; k++) {
                int nx = x + dx[k];
                int ny = y + dy[k];
                int ni = index(nx, ny);

                float neighborSurface = terrain[ni] + water[ni];
                float diff = currentSurface - neighborSurface;

                if (diff > 0.0f && available > 0.0f) {
                    float flow = std::min(available, diff * flowRate * 0.25f);

                    nextWater[i] -= flow;
                    nextWater[ni] += flow;
                    available -= flow;
                }
            }
        }
    }

    for (int i = 0; i < (int)nextWater.size(); i++) {
        nextWater[i] -= infiltrationRate;
        if (nextWater[i] < 0.0f) nextWater[i] = 0.0f;
    }

    water = nextWater;
}
