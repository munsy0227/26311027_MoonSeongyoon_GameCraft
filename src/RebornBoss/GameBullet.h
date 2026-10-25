#pragma once

struct GameBullet
{
    float x{};
    float y{};
    float velocityY{-600.0F};
    void Update(float seconds) { y += velocityY * seconds; }
    bool Outside() const { return y < -12.0F; }
};
