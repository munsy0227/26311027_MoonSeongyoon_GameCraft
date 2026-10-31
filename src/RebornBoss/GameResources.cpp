#include "GameResources.h"
#include <cstdio>
#include <iterator>

namespace
{
    constexpr const char* imagePaths[] = {
        "resources/images/backgrounds/title_background.png",
        "resources/images/backgrounds/stage01_forest.png",
        "resources/images/ui/info_panel.png",
        "resources/images/portraits/leon_portrait.png",
        "resources/images/portraits/lillian_portrait.png",
        "resources/images/portraits/vivian_portrait.png",
        "resources/images/portraits/eleanor_portrait.png",
        "resources/images/characters/leon_animations.png",
        "resources/images/enemies/patrol_fairy_sheet.png",
        "resources/images/bullets/player_shot.png",
        "resources/images/ui/lock_icon.png",
        "resources/images/ui/hitbox_marker.png",
        "resources/images/characters/lillian_animations.png",
        "resources/images/bullets/enemy_petal.png",
        "resources/images/bullets/enemy_orb.png",
        "resources/images/ui/life_icon.png",
        "resources/images/ui/bomb_icon.png",
        "resources/images/effects/explosion_sheet.png",
        "resources/images/effects/hit_sheet.png",
        "resources/images/effects/bomb_sheet.png",
        "resources/images/ui/white_pixel.png",
        "resources/images/ui/surface.png"
    };
    constexpr const char* soundPaths[] = {
        "resources/audio/bgm/menu.wav",
        "resources/audio/bgm/stage01_forest.wav",
        "resources/audio/sfx/player_shot.wav",
        "resources/audio/sfx/menu_move.wav",
        "resources/audio/sfx/menu_confirm.wav",
        "resources/audio/bgm/boss.wav",
        "resources/audio/sfx/player_hit.wav",
        "resources/audio/sfx/explosion.wav",
        "resources/audio/sfx/bomb.wav",
        "resources/audio/sfx/stage_clear.wav",
        "resources/audio/sfx/game_over.wav"
    };
    static_assert(std::size(imagePaths) == static_cast<size_t>(ImageId::Count));
    static_assert(std::size(soundPaths) == static_cast<size_t>(SoundId::Count));
}

GameResources::GameResources()
{
    m_images.fill(-1);
    m_sounds.fill(-1);
    m_fonts.fill(-1);
}

int GameResources::Init()
{
    for (size_t i = 0; i < m_images.size(); ++i)
    {
        // PNG의 알파를 그대로 쓰고 흰색 외곽선의 색상 키 제거를 막는다.
        m_images[i] = g2_TextureLoad(imagePaths[i], 0);
        if (m_images[i] < 0)
        {
            std::fprintf(stderr, "Texture load failed: %s\n", imagePaths[i]);
            return -1;
        }
    }
    for (size_t i = 0; i < m_sounds.size(); ++i)
    {
        m_sounds[i] = g2_SoundLoad(soundPaths[i]);
        if (m_sounds[i] < 0)
        {
            std::fprintf(stderr, "Sound load failed: %s\n", soundPaths[i]);
            return -1;
        }
    }
    // glc2d의 일반 폰트 핸들은 g2_DestroyWin에서 함께 해제된다.
    constexpr int sizes[] = {40, 28, 22, 18};
    static_assert(std::size(sizes) == static_cast<size_t>(FontStyle::Count));
    for (size_t i = 0; i < m_fonts.size(); ++i)
    {
        m_fonts[i] = g2_FontCreate("Malgun Gothic", sizes[i]);
        if (m_fonts[i] < 0)
        {
            std::fprintf(stderr, "System font creation failed: Malgun Gothic\n");
            return -1;
        }
    }
    return 0;
}

void GameResources::Destroy()
{
    for (int& sound : m_sounds)
    {
        if (sound >= 0)
        {
            g2_SoundStop(sound);
            g2_SoundRelease(sound);
            sound = -1;
        }
    }
    for (int& image : m_images)
    {
        if (image >= 0)
        {
            g2_TextureRelease(image);
            image = -1;
        }
    }
    m_bgm = -1;
    m_fonts.fill(-1);
}

void GameResources::Draw(ImageId image, float x, float y, float scale, DWORD color) const
{
    const VEC2 position{x, y};
    const VEC2 scaling{scale, scale};
    g2_Draw2D(m_images[static_cast<size_t>(image)], nullptr, &position, &scaling,
              nullptr, 0.0F, color);
}

void GameResources::DrawFrame(ImageId image, int column, int row, int width, int height,
                             float x, float y) const
{
    const RECT source{column * width, row * height, (column + 1) * width, (row + 1) * height};
    const VEC2 position{x, y};
    g2_Draw2D(m_images[static_cast<size_t>(image)], &source, &position);
}

void GameResources::Rect(float x, float y, float width, float height, DWORD color) const
{
    if (width <= 0.0F || height <= 0.0F)
        return;
    const VEC2 position{x, y};
    const VEC2 scaling{width, height};
    g2_Draw2D(m_images[static_cast<size_t>(ImageId::White)], nullptr, &position, &scaling,
              nullptr, 0.0F, color);
}

void GameResources::Surface(float x, float y, float width, float height, DWORD color) const
{
    if (width < 24.0F || height < 24.0F)
    {
        Rect(x, y, width, height, color);
        return;
    }
    // Fixed-size corners keep the panel radius unchanged at every menu size.
    constexpr LONG cuts[] = {0, 12, 20, 32};
    const float xs[] = {x, x + 12.0F, x + width - 12.0F, x + width};
    const float ys[] = {y, y + 12.0F, y + height - 12.0F, y + height};
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column)
        {
            const RECT source{cuts[column], cuts[row], cuts[column + 1], cuts[row + 1]};
            const VEC2 position{xs[column], ys[row]};
            const VEC2 scaling{(xs[column + 1] - xs[column]) / (cuts[column + 1] - cuts[column]),
                               (ys[row + 1] - ys[row]) / (cuts[row + 1] - cuts[row])};
            g2_Draw2D(m_images[static_cast<size_t>(ImageId::Surface)], &source, &position, &scaling,
                      nullptr, 0.0F, color);
        }
}

void GameResources::PlayBgm(SoundId sound)
{
    const int handle = m_sounds[static_cast<size_t>(sound)];
    if (handle == m_bgm)
        return;
    if (m_bgm >= 0)
        g2_SoundStop(m_bgm);
    m_bgm = handle;
    g2_SoundReset(handle);
    g2_SoundPlay(handle, true);
}

void GameResources::PlaySound(SoundId sound) const
{
    const int handle = m_sounds[static_cast<size_t>(sound)];
    g2_SoundReset(handle);
    g2_SoundPlay(handle);
}
