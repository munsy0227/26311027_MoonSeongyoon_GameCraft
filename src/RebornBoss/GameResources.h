#pragma once
#include <array>
#include <glc2d.h>

enum class ImageId
{
    Title, Forest, Panel, LeonPortrait, LillianPortrait, VivianPortrait, EleanorPortrait,
    Leon, PatrolFairy, PlayerShot, Lock, Hitbox, Count
};

enum class SoundId { Menu, Forest, Shot, MenuMove, MenuConfirm, Count };

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
    void PlayBgm(SoundId sound);
    void PlaySound(SoundId sound) const;
    int Font() const { return m_font; }
    int TitleFont() const { return m_titleFont; }

private:
    std::array<int, static_cast<size_t>(ImageId::Count)> m_images;
    std::array<int, static_cast<size_t>(SoundId::Count)> m_sounds;
    int m_font{-1};
    int m_titleFont{-1};
    int m_bgm{-1};
};
