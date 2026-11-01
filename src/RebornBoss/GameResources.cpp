#include "GameResources.h"
#include <cstdio>
#include <iterator>
#include <cmath>

namespace
{
    constexpr int ForestHeight = 600;
    constexpr int ForestOverlap = 96;
    constexpr int ForestPeriod = ForestHeight - ForestOverlap;

    constexpr const char* imagePaths[] = {
        "resources/images/backgrounds/title_background.png",
        "resources/images/backgrounds/stage01_forest.png",
        "resources/images/portraits/leon_portrait.png",
        "resources/images/portraits/lillian_portrait.png",
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
        "resources/images/poses/leon/leon_idle.png",
        "resources/images/poses/lillian/lillian_idle.png",
        "resources/images/poses/vivian/vivian_idle.png",
        "resources/images/poses/eleanor/eleanor_idle.png",
        "resources/images/backgrounds/title_atmosphere.png",
        "resources/images/enemies/mist_familiar_sheet.png",
        "resources/images/enemies/barrier_guardian_sheet.png"
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
    // Private font files and handles are released together by g2_DestroyWin.
    constexpr const char* fontPaths[] = {
        "resources/fonts/Mona12TextKR.ttf",
        "resources/fonts/Cafe24Classictype-v1.1.ttf"
    };
    for (const char* path : fontPaths)
    {
        if (g2_FontRegisterFile(path) <= 0)
        {
            std::fprintf(stderr, "Private font registration failed: %s\n", path);
            return -1;
        }
    }
    constexpr const char* families[] = {
        "Cafe24 ClassicType", "Cafe24 ClassicType", "Mona12 Text KR", "Mona12 Text KR"
    };
    constexpr int sizes[] = {40, 28, 22, 18};
    static_assert(std::size(sizes) == static_cast<size_t>(FontStyle::Count));
    static_assert(std::size(families) == std::size(sizes));
    for (size_t i = 0; i < m_fonts.size(); ++i)
    {
        m_fonts[i] = g2_FontCreate(families[i], sizes[i]);
        if (m_fonts[i] < 0)
        {
            std::fprintf(stderr, "Private font creation failed: %s (%d px)\n", families[i], sizes[i]);
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
                             float x, float y, float scale) const
{
    const RECT source{column * width, row * height, (column + 1) * width, (row + 1) * height};
    const VEC2 position{x, y};
    const VEC2 scaling{scale, scale};
    g2_Draw2D(m_images[static_cast<size_t>(image)], &source, &position, &scaling);
}

void GameResources::DrawRotated(ImageId image, float centerX, float centerY, int width, int height,
                               float angleRadians, float scale) const
{
    const VEC2 position{centerX - width * scale * 0.5F, centerY - height * scale * 0.5F};
    const VEC2 scaling{scale, scale};
    // glc2d uses a back-buffer pivot and negates its input angle.
    const float screenScale = g2_GetScnScale();
    const VEC2 pivot{centerX * screenScale, centerY * screenScale};
    g2_Draw2D(m_images[static_cast<size_t>(image)], nullptr, &position, &scaling,
              &pivot, -angleRadians);
}

float GameResources::ForestScrollOffset(float combatSeconds)
{
    return std::fmod(combatSeconds * 24.0F, static_cast<float>(ForestPeriod));
}

void GameResources::DrawScrollingForest(float combatSeconds) const
{
    const float offset = ForestScrollOffset(combatSeconds);
    const int texture = m_images[static_cast<size_t>(ImageId::Forest)];
    // Overlap the painted edges so the original forest loops without a hard seam.
    for (int tile = -2; tile <= 1; ++tile)
    {
        const float top = offset + tile * ForestPeriod;
        if (top >= ForestHeight || top + ForestHeight <= 0.0F)
            continue;
        if (top + ForestOverlap < ForestHeight)
        {
            const RECT source{0, ForestOverlap, 480, ForestHeight};
            const VEC2 position{0.0F, top + ForestOverlap};
            g2_Draw2D(texture, &source, &position, nullptr, nullptr, 0.0F, 0xFFB8B8B8);
        }
        for (int row = 0; row < ForestOverlap; row += 4)
        {
            const float y = top + row;
            if (y >= ForestHeight || y + 4.0F <= 0.0F)
                continue;
            const RECT source{0, row, 480, row + 4};
            const VEC2 position{0.0F, y};
            const DWORD alpha = static_cast<DWORD>((row + 2) * 255 / ForestOverlap);
            g2_Draw2D(texture, &source, &position, nullptr, nullptr, 0.0F,
                      (alpha << 24) | 0x00B8B8B8);
        }
    }
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
