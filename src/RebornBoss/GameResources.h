#pragma once
#include <array>
#include <glc2d.h>

enum class ImageId
{
    Title, Forest, LeonPortrait, LillianPortrait,
    Leon, PatrolFairy, PlayerShot, Lock, Hitbox, Lillian, EnemyPetal, EnemyOrb,
    Life, Bomb, Explosion, PlayerHit, BombEffect, White, LeonArt, LillianArt, VivianArt, EleanorArt, Count
};

enum class SoundId { Menu, Forest, Shot, MenuMove, MenuConfirm, Boss, Hit, EnemyDown, Bomb, Clear, Fail, Count };
enum class FontStyle { Title, Heading, Body, Caption, Count };

namespace UIColor
{
    constexpr DWORD Background = 0xFF0E111A;
    constexpr DWORD Text = 0xFFF5F7FA;
    constexpr DWORD Muted = 0xFFAAB3C5;
    constexpr DWORD Accent = 0xFFA99CFF;
    constexpr DWORD Border = 0xFF39425A;
    constexpr DWORD Danger = 0xFFFFADAD;
}

class GameResources
{
public:
    GameResources();
    int Init();
    void Destroy();
    void Draw(ImageId image, float x, float y, float scale = 1.0F,
              DWORD color = 0xFFFFFFFF) const;
    void DrawFrame(ImageId image, int column, int row, int width, int height,
                   float x, float y) const;
    void Rect(float x, float y, float width, float height, DWORD color) const;
    void PlayBgm(SoundId sound);
    void PlaySound(SoundId sound) const;
    int Font(FontStyle style = FontStyle::Body) const { return m_fonts[static_cast<size_t>(style)]; }

private:
    std::array<int, static_cast<size_t>(ImageId::Count)> m_images;
    std::array<int, static_cast<size_t>(SoundId::Count)> m_sounds;
    std::array<int, static_cast<size_t>(FontStyle::Count)> m_fonts;
    int m_bgm{-1};
};
