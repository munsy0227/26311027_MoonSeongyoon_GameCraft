#ifndef NDEBUG
#include "ScenePlay.h"
#include <cassert>
#include <cmath>

void CheckWeek2Gameplay()
{
    const auto almostEqual = [](float a, float b) { return std::abs(a - b) < 0.001F; };
    GamePlayer normal;
    normal.Update(0.1F, 1, 0, false, false);
    assert(almostEqual(normal.X(), 264));
    GamePlayer diagonal;
    diagonal.Update(0.1F, 1, -1, false, false);
    assert(almostEqual(std::hypot(diagonal.X() - 240, diagonal.Y() - 520), 24));
    GamePlayer slow;
    slow.Update(0.1F, 1, 0, true, false);
    assert(almostEqual(slow.X(), 250));
    slow.Update(1000, -1, -1, false, false);
    assert(almostEqual(slow.X(), 16) && almostEqual(slow.Y(), 24));
    slow.Update(1000, 1, 1, false, false);
    assert(almostEqual(slow.X(), 464) && almostEqual(slow.Y(), 576));

    GamePlayer thirty;
    GamePlayer sixty;
    for (int i = 0; i < 3; ++i) thirty.Update(1.0F / 30, 1, 0, false, false);
    for (int i = 0; i < 6; ++i) sixty.Update(1.0F / 60, 1, 0, false, false);
    assert(almostEqual(thirty.X(), sixty.X()));

    BulletManager fine;
    BulletManager coarse;
    for (int i = 0; i < 95; ++i) fine.Update(0.01F, true, 240, 10000);
    for (int i = 0; i < 19; ++i) coarse.Update(0.05F, true, 240, 10000);
    assert(fine.Count() == 8 && coarse.Count() == 8);
    fine.Update(100, false, 240, 520);
    assert(fine.Count() == 0);
    coarse.Clear();
    assert(coarse.Count() == 0);

    GameMob mob;
    mob.Update(11);
    assert(mob.Outside());

    ScenePlay scene;
    scene.Tick(0.8F, 0, 0, false, false);
    assert(scene.m_mobs.size() == 1);
    const float elapsed = scene.m_elapsedSeconds;
    const float spawnTimer = scene.m_spawnTimer;
    const float mobY = scene.m_mobs.front().Y();
    const float playerX = scene.m_player.X();
    scene.m_paused = true;
    scene.Tick(30, 1, 0, true, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed));
    assert(almostEqual(scene.m_spawnTimer, spawnTimer));
    assert(almostEqual(scene.m_mobs.front().Y(), mobY));
    assert(almostEqual(scene.m_player.X(), playerX) && scene.m_bullets.Count() == 0);
    scene.m_paused = false;
    scene.Tick(0.05F, 0, 0, false, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed + 0.05F));
    assert(scene.m_bullets.Count() == 1);
}
#endif
