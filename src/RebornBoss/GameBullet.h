#pragma once

enum class BulletTeam { Player, Enemy };

struct GameBullet
{
    float x{};
    float y{};
    float velocityY{-600.0F};
    float velocityX{};
    BulletTeam team{BulletTeam::Player};
    float radius{5.0F};
    int damage{1};
    bool active{true};
    bool orb{};
    void Update(float seconds)
    {
        x += velocityX * seconds;
        y += velocityY * seconds;
    }
    bool Outside() const { return x < -12.0F || x > 492.0F || y < -12.0F || y > 612.0F; }
};
