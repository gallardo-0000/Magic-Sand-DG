#pragma once
#include <vector>

class RainSimulation {

public:

    void setup(int w, int h);

    void update(const std::vector<float>& terrain);

    void addRain(float amount);

    void addLocalizedRain(int cx, int cy, float radius, float amount);

    void clearWater();

    const std::vector<float>& getWater() const;

private:

    int width;
    int height;

    std::vector<float> terrainHeight;
    std::vector<float> water;
    std::vector<float> nextWater;

    float infiltration = 0.001f;
    float flowSpeed = 0.2f;

    int index(int x, int y) const;
};
