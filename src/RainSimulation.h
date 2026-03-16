#pragma once
#include <vector>

class RainSimulation {
public:
    void setup(int w, int h);
    void update(const std::vector<float>& terrainHeights);

    void addRain(float amount);
    void addLocalizedRain(int cx, int cy, float radius, float amount);
    void clearWater();

    const std::vector<float>& getWater() const;

private:
    int width = 0;
    int height = 0;

    std::vector<float> terrain;
    std::vector<float> water;
    std::vector<float> nextWater;

    float infiltrationRate = 0.001f;
    float flowRate = 0.15f;

    int index(int x, int y) const;
};
