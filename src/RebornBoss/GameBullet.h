#pragma once
#include <cmath>

enum class BulletTeam { Player, Enemy };

struct GameBullet
{
    static constexpr float RenderScale = 1.25F;
    static constexpr float CullMargin = 20.0F;
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
    float HeadingRadians() const
    {
        return std::hypot(velocityX, velocityY) <= 0.001F ? 0.0F : std::atan2(velocityX, -velocityY);
    }
    bool Outside() const
    {
        return x < -CullMargin || x > 480.0F + CullMargin
            || y < -CullMargin || y > 600.0F + CullMargin;
    }
};
