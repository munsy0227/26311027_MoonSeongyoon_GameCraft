#ifndef NDEBUG
#include "ScenePlay.h"
#include <algorithm>
#include <cassert>
#include <cmath>

void CheckGameplay()
{
    const auto almostEqual = [](float a, float b) { return std::abs(a - b) < 0.001F; };
    const auto waves = [](ScenePlay& scene)
    {
        scene.Reset();
        scene.m_stagePhase = ScenePlay::StagePhase::Waves;
    };
    const auto enemyBullet = [](float x, float y)
    {
        return GameBullet{x, y, 0.0F, 0.0F, BulletTeam::Enemy};
    };
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
    for (int i = 0; i < 95; ++i) fine.Update(0.01F, true, 240, 628);
    for (int i = 0; i < 19; ++i) coarse.Update(0.05F, true, 240, 628);
    assert(fine.Count() == 8 && coarse.Count() == 8);
    fine.Update(100, false, 240, 520);
    assert(fine.Count() == 0);
    coarse.Add(enemyBullet(240, 200));
    coarse.ClearEnemy();
    assert(coarse.Count() == 8);
    coarse.Clear();
    assert(coarse.Count() == 0);
    GameBullet sideways{490, 100, 0, 100, BulletTeam::Enemy};
    sideways.Update(0.1F);
    assert(sideways.Outside());
    assert((GameBullet{240, 613, 0}).Outside());

    GamePlayer hurt;
    assert(hurt.Damage(0) == 0 && hurt.Damage(-1) == 0);
    assert(hurt.Damage(1) == 1 && hurt.Health() == 2);
    assert(hurt.Damage(1) == 0 && hurt.Health() == 2);
    assert(hurt.UseBomb() && hurt.Bombs() == 2);
    assert(almostEqual(hurt.InvulnerabilitySeconds(), 1.5F));
    hurt.Update(1.25F, 0, 0, false, false);
    assert(hurt.UseBomb() && almostEqual(hurt.InvulnerabilitySeconds(), 1.0F));
    assert(hurt.UseBomb() && hurt.Bombs() == 0);
    assert(!hurt.UseBomb() && hurt.Bombs() == 0);
    hurt.Update(1.01F, 0, 0, false, false);
    assert(hurt.Damage(5) == 2 && hurt.Dead());
    assert(hurt.Damage(1) == 0 && !hurt.UseBomb());

    GameMob patrol;
    BulletManager patrolBullets;
    patrol.Update(1.5F, patrolBullets);
    assert(patrolBullets.Count() == 1);
    assert(patrolBullets.Bullets().front().team == BulletTeam::Enemy);
    assert(almostEqual(patrolBullets.Bullets().front().velocityY, 110));
    patrol.Update(11);
    assert(patrol.Outside());

    ScenePlay scene;
    waves(scene);
    scene.Tick(0.8F, 0, 0, false, false);
    assert(scene.m_mobs.size() == 1);
    const float elapsed = scene.m_elapsedSeconds;
    const float spawnTimer = scene.m_spawnTimer;
    const float mobY = scene.m_mobs.front().Y();
    const float playerX = scene.m_player.X();
    scene.m_player.Damage(1);
    scene.m_paused = true;
    scene.Tick(30, 1, 0, true, true, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed));
    assert(almostEqual(scene.m_spawnTimer, spawnTimer));
    assert(almostEqual(scene.m_mobs.front().Y(), mobY));
    assert(almostEqual(scene.m_player.X(), playerX) && scene.m_bullets.Count() == 0);
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 1.5F));
    assert(scene.m_player.Bombs() == 3);
    scene.m_paused = false;
    scene.Tick(0.05F, 0, 0, false, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed + 0.05F));
    assert(scene.m_bullets.Count() == 1);

    // 충돌 경계와 무적, 겹친 탄환의 중복 피해를 확인한다.
    waves(scene);
    scene.m_bullets.Add(enemyBullet(scene.m_player.X() + 9.01F, scene.m_player.Y()));
    scene.Collide();
    assert(scene.m_player.Health() == 3);
    scene.m_bullets.Clear();
    scene.m_bullets.Add(enemyBullet(scene.m_player.X() + 9.0F, scene.m_player.Y()));
    scene.m_bullets.Add(enemyBullet(scene.m_player.X(), scene.m_player.Y()));
    scene.Collide();
    assert(scene.m_player.Health() == 2 && scene.m_bullets.Count() == 0);
    scene.m_bullets.Add(enemyBullet(scene.m_player.X(), scene.m_player.Y()));
    scene.Collide();
    assert(scene.m_player.Health() == 2);

    waves(scene);
    GameMob target(240);
    target.Update(2);
    target.Damage(2);
    scene.m_mobs.push_back(target);
    scene.m_mobs.push_back(target);
    scene.m_bullets.Add({240, target.Y(), 0});
    scene.Collide();
    assert(scene.m_score == 100 && scene.m_mobs.size() == 1);
    assert(scene.m_mobs.front().Health() == 1 && scene.m_bullets.Count() == 0);
    scene.Collide();
    assert(scene.m_score == 100);
    scene.m_mobs.front().Update(11);
    scene.Collide();
    assert(scene.m_mobs.empty() && scene.m_score == 100);

    waves(scene);
    GameMob fastTarget(240);
    fastTarget.Update(260.0F / 60.0F);
    fastTarget.Damage(2);
    scene.m_mobs.push_back(fastTarget);
    scene.m_bullets.Add({240, 300, -600});
    scene.Tick(0.2F, 0, 0, false, false);
    assert(scene.m_score == 100 && scene.m_mobs.empty());

    waves(scene);
    scene.m_bullets.Add(enemyBullet(240, 200));
    scene.m_bullets.Add({240, 300, 0});
    scene.Tick(0.05F, 0, 0, false, false, true);
    assert(scene.m_player.Bombs() == 2 && scene.m_player.Health() == 3);
    assert(scene.m_bullets.Count() == 1);
    assert(scene.m_bullets.Bullets().front().team == BulletTeam::Player);
    scene.Tick(0.05F, 0, 0, false, false, false);
    assert(scene.m_player.Bombs() == 2);
    scene.Tick(0.01F, 0, 0, false, false, true);
    scene.Tick(0.01F, 0, 0, false, false, true);
    scene.Tick(0.01F, 0, 0, false, false, true);
    assert(scene.m_player.Bombs() == 0);

    waves(scene);
    scene.m_waveSeconds = 89.99F;
    scene.m_mobs.emplace_back(96);
    scene.m_bullets.Add(enemyBullet(400, 200));
    scene.Tick(0.02F, 0, 0, false, false);
    assert(scene.m_stagePhase == ScenePlay::StagePhase::BossIntro);
    assert(scene.m_mobs.empty() && scene.m_bullets.Count() == 0);
    const float dialogueTime = scene.m_elapsedSeconds;
    scene.m_player.Damage(1);
    scene.Tick(30, 1, 0, false, true, true);
    assert(almostEqual(scene.m_elapsedSeconds, dialogueTime));
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 1.5F));
    assert(scene.m_player.Bombs() == 3);

    GameBossMob boss;
    BulletManager bossBullets;
    boss.Update(1.2F, bossBullets);
    assert(bossBullets.Count() == 7);
    bossBullets.Add({240, 300, 0});
    boss.Damage(250);
    assert(boss.AdvancePhase(bossBullets) && boss.PhaseTwo());
    assert(bossBullets.Count() == 1 && !boss.AdvancePhase(bossBullets));
    boss.Update(1.5F, bossBullets);
    assert(bossBullets.Count() == 25);
    assert(std::count_if(bossBullets.Bullets().begin(), bossBullets.Bullets().end(),
        [](const GameBullet& bullet) { return bullet.team == BulletTeam::Enemy && bullet.orb; }) == 24);
    boss.Damage(250);
    assert(boss.Dead() && !boss.AdvancePhase(bossBullets));

    waves(scene);
    scene.m_stagePhase = ScenePlay::StagePhase::Boss;
    scene.m_boss.Damage(499);
    scene.m_player.Damage(2);
    scene.m_player.Update(2, 0, 0, false, false);
    scene.m_bullets.Add({scene.m_boss.X(), scene.m_boss.Y(), 0});
    scene.m_bullets.Add(enemyBullet(scene.m_player.X(), scene.m_player.Y()));
    scene.Collide();
    assert(scene.m_stagePhase == ScenePlay::StagePhase::Finished && scene.m_cleared);
    assert(scene.m_player.Dead() && scene.m_boss.Dead() && !scene.m_boss.PhaseTwo());
    assert(scene.m_score == 5000);
    scene.Collide();
    assert(scene.m_score == 5000);

    waves(scene);
    scene.m_player.Damage(2);
    scene.m_player.Update(2, 0, 0, false, false);
    scene.m_bullets.Add(enemyBullet(scene.m_player.X(), scene.m_player.Y()));
    scene.Collide();
    assert(scene.m_stagePhase == ScenePlay::StagePhase::Finished && !scene.m_cleared);
    scene.m_score = 1234;
    scene.m_paused = true;
    scene.Reset();
    assert(scene.m_stagePhase == ScenePlay::StagePhase::Intro);
    assert(scene.m_player.Health() == 3 && scene.m_player.Bombs() == 3);
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 0));
    assert(scene.m_score == 0 && scene.m_elapsedSeconds == 0 && scene.m_waveSeconds == 0);
    assert(scene.m_bullets.Count() == 0 && scene.m_mobs.empty() && scene.m_effects.empty());
    assert(scene.m_boss.Health() == 500 && !scene.m_boss.PhaseTwo());
    assert(!scene.m_paused && !scene.m_cleared);
    scene.Tick(10, 1, 0, true, true, true);
    assert(scene.m_elapsedSeconds == 0 && scene.m_bullets.Count() == 0);
}
#endif
