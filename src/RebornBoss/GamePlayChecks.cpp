#ifndef NDEBUG
#include "ScenePlay.h"
#include "GameInput.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void CheckGameplay()
{
    GameInput controls;
    assert(!controls.IsNewClassic());
    controls.ToggleControlPreset();
    assert(controls.IsNewClassic());
    GameInput defaultControls;
    assert(!defaultControls.IsNewClassic());
    controls.ToggleControlPreset();
    assert(!controls.IsNewClassic());

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
    const auto hasSpeed = [&](const GameBullet& bullet, float speed)
    {
        return std::isfinite(bullet.velocityX) && std::isfinite(bullet.velocityY)
            && almostEqual(std::hypot(bullet.velocityX, bullet.velocityY), speed);
    };
    const auto aimedAt = [&](const GameBullet& bullet, float x, float y, float speed)
    {
        const float dx = x - bullet.x;
        const float dy = y - bullet.y;
        const float distance = std::hypot(dx, dy);
        return almostEqual(bullet.velocityX, dx * speed / distance)
            && almostEqual(bullet.velocityY, dy * speed / distance);
    };
    GamePlayer normal;
    normal.Update(0.1F, 1, 0, false, false);
    assert(almostEqual(normal.X(), 254.37037F));
    GamePlayer diagonal;
    diagonal.Update(0.1F, 1, -1, false, false);
    assert(almostEqual(std::hypot(diagonal.X() - 240, diagonal.Y() - 520), 14.37037F));
    GamePlayer slow;
    slow.Update(0.1F, 1, 0, true, false);
    assert(almostEqual(slow.X(), 245.98766F));
    const float releasedX = normal.X();
    normal.Update(0.5F, 0, 0, false, false);
    assert(almostEqual(normal.X(), releasedX));
    normal.Update(0.1F, 1, 0, false, false);
    assert(almostEqual(normal.X() - releasedX, 14.37037F));
    GamePlayer focus;
    focus.Update(0.2F, 1, 0, false, false);
    const float focusX = focus.X();
    focus.Update(0.05F, 1, 0, true, false);
    assert(almostEqual(focus.X() - focusX, 5.0F));
    GamePlayer reverse;
    reverse.Update(0.1F, 1, 0, false, false);
    const float reverseX = reverse.X();
    reverse.Update(0.01F, -1, 0, false, false);
    assert(reverse.X() > reverseX);
    reverse.Update(0.1F, -1, 0, false, false);
    assert(reverse.X() < reverseX);
    slow.Update(1000, -1, -1, false, false);
    assert(almostEqual(slow.X(), 16) && almostEqual(slow.Y(), 24));
    slow.Update(1000, 1, 1, false, false);
    assert(almostEqual(slow.X(), 464) && almostEqual(slow.Y(), 576));
    slow.Update(0.01F, -1, 0, false, false);
    assert(slow.X() < 464.0F);
    GamePlayer stocked;
    assert(stocked.Health() == 3 && stocked.Bombs() == 3 && stocked.Power() == 0);
    assert(stocked.Heal() && stocked.Heal() && !stocked.Heal());
    assert(stocked.AddBomb() && stocked.AddBomb() && !stocked.AddBomb());
    assert(stocked.Health() == 5 && stocked.Bombs() == 5);
    for (int i = 0; i < GamePlayer::MaxPower; ++i) assert(stocked.AddPower());
    assert(stocked.Power() == 4 && !stocked.AddPower());
    assert(stocked.UseBomb() && stocked.Bombs() == 4);
    GamePlayer defeated;
    assert(defeated.Damage(3) == 3 && defeated.Dead());
    assert(!defeated.Heal() && !defeated.AddBomb() && !defeated.AddPower());
    assert(defeated.Health() == 0 && defeated.Bombs() == 3 && defeated.Power() == 0);
    GamePlayer thirty;
    GamePlayer sixty;
    for (int i = 0; i < 3; ++i) thirty.Update(1.0F / 30, 1, 0, false, false);
    for (int i = 0; i < 6; ++i) sixty.Update(1.0F / 60, 1, 0, false, false);
    assert(almostEqual(thirty.X(), sixty.X()));

    for (int power = -1; power <= GamePlayer::MaxPower + 1; ++power)
    {
        BulletManager spread;
        assert(spread.Update(0, true, 240, 520, power));
        const int level = std::clamp(power, 0, GamePlayer::MaxPower);
        const auto count = static_cast<std::size_t>(2 * level + 1);
        assert(spread.Count() == count);
        for (const auto& bullet : spread.Bullets())
        {
            assert(hasSpeed(bullet, 600) && bullet.team == BulletTeam::Player && bullet.damage == 1);
            assert(bullet.velocityY < 0);
        }
        assert(almostEqual(spread.Bullets()[level].velocityX, 0));
        assert(almostEqual(spread.Bullets()[level].velocityY, -600));
        assert(almostEqual(spread.Bullets().front().velocityX, -spread.Bullets().back().velocityX));
        assert(almostEqual(spread.Bullets().front().velocityY, spread.Bullets().back().velocityY));
        assert(!spread.Update(0.05F, true, 240, 520, power));
        assert(spread.Update(0.07F, true, 240, 520, power));
        assert(spread.Count() == count * 2);
    }
    BulletManager growing;
    growing.Update(0, true, 240, 520, 0);
    growing.Update(0.12F, true, 240, 520, 4);
    assert(growing.Count() == 10 && almostEqual(growing.Bullets().front().velocityX, 0));

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

    BulletManager aimed;
    aimed.AddAimedFan(240, 100, 96, 500, 3, 8, 150);
    assert(aimed.Count() == 3 && aimedAt(aimed.Bullets()[1], 96, 500, 150));
    const GameBullet snapshot = aimed.Bullets()[1];
    aimed.Update(0.1F, false, 384, 500);
    const auto& released = aimed.Bullets()[1];
    assert(almostEqual(released.velocityX, snapshot.velocityX));
    assert(almostEqual(released.velocityY, snapshot.velocityY));
    assert(almostEqual(released.x, snapshot.x + snapshot.velocityX * 0.1F));
    assert(almostEqual(released.y, snapshot.y + snapshot.velocityY * 0.1F));
    for (const auto& bullet : aimed.Bullets()) assert(hasSpeed(bullet, 150));
    aimed.Clear();
    aimed.AddAimedFan(240, 100, 384, 500, 3, 8, 150);
    assert(aimedAt(aimed.Bullets()[1], 384, 500, 150));
    assert(snapshot.velocityX < 0 && aimed.Bullets()[1].velocityX > 0);
    aimed.Clear();
    aimed.AddAimedFan(240, 100, 240, 100, 3, 8, 150);
    assert(aimed.Count() == 0);

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
    patrol.Update(1.4F, patrolBullets, 96, 520);
    assert(patrol.Type() == MobType::Patrol && patrol.Health() == 3);
    assert(patrolBullets.Count() == 3);
    assert(patrolBullets.Bullets().front().team == BulletTeam::Enemy);
    assert(aimedAt(patrolBullets.Bullets()[1], 96, 520, 90));
    for (const auto& bullet : patrolBullets.Bullets()) assert(hasSpeed(bullet, 90));
    patrol.Update(11);
    assert(patrol.Outside());
    GameMob familiar(240, MobType::Familiar);
    assert(familiar.Type() == MobType::Familiar && familiar.Health() == 3);
    familiar.Update(0.75F);
    assert(almostEqual(familiar.X(), 288) && almostEqual(familiar.Y(), 17.5F));
    familiar.Update(0.75F);
    assert(almostEqual(familiar.X(), 240) && almostEqual(familiar.Y(), 55));
    familiar.Update(0.75F);
    assert(almostEqual(familiar.X(), 192));
    GameMob leftEdge(32, MobType::Familiar);
    GameMob rightEdge(448, MobType::Familiar);
    leftEdge.Update(2.25F);
    rightEdge.Update(0.75F);
    assert(almostEqual(leftEdge.X(), 32) && almostEqual(rightEdge.X(), 448));
    GameMob guardian(240, MobType::Guardian);
    assert(guardian.Type() == MobType::Guardian && guardian.Health() == 5);
    guardian.Update(1);
    assert(almostEqual(guardian.Y(), 65));
    guardian.Update(1);
    assert(almostEqual(guardian.Y(), 150));
    guardian.Update(2.5F);
    assert(almostEqual(guardian.Y(), 150));
    guardian.Update(0.5F);
    assert(almostEqual(guardian.Y(), 192.5F));
    assert(guardian.Damage(4) == 4 && !guardian.Dead());
    assert(guardian.Damage(1) == 1 && guardian.Dead());
    GameMob stationary(240, MobType::Guardian);
    BulletManager stationaryShots;
    stationary.Update(1.9F, stationaryShots, 240, 520);
    assert(stationaryShots.Count() == 0);
    stationary.Update(0.1F, stationaryShots, 240, 520);
    assert(stationaryShots.Count() == 5);
    stationary.Update(2, stationaryShots, 240, 520);
    assert(stationaryShots.Count() == 10);
    stationary.Update(2, stationaryShots, 240, 520);
    assert(stationaryShots.Count() == 10 && stationary.Y() > 150);
    constexpr MobType types[] = {MobType::Patrol, MobType::Familiar, MobType::Guardian};
    for (MobType type : types)
    {
        const float interval = type == MobType::Patrol ? 1.4F : type == MobType::Familiar ? 1.6F : 2.0F;
        const int count = type == MobType::Patrol ? 3 : type == MobType::Familiar ? 1 : 5;
        const float speed = type == MobType::Patrol ? 90.0F : type == MobType::Familiar ? 85.0F : 80.0F;
        GameMob cautious(240, type);
        BulletManager cautiousBullets;
        cautious.Update(interval, cautiousBullets, 240, type == MobType::Guardian ? 240.0F : 150.0F);
        cautious.Update(0.1F, cautiousBullets, 240, 520);
        assert(cautiousBullets.Count() == 0);
        cautious.Update(interval - 0.1F, cautiousBullets, 240, 520);
        assert(cautiousBullets.Count() == static_cast<std::size_t>(count));
        assert(aimedAt(cautiousBullets.Bullets()[count / 2], 240, 520, speed));
        for (const auto& bullet : cautiousBullets.Bullets()) assert(hasSpeed(bullet, speed));
        GameMob behind(240, type);
        BulletManager behindBullets;
        behind.Update(interval, behindBullets, 16, 20);
        assert(behindBullets.Count() == 0);
        GameMob mob30(240, type);
        GameMob mob60(240, type);
        BulletManager shots30;
        BulletManager shots60;
        for (int i = 0; i < 135; ++i) mob30.Update(1.0F / 30, shots30, 240, 520);
        for (int i = 0; i < 270; ++i) mob60.Update(1.0F / 60, shots60, 240, 520);
        const int expected = type == MobType::Patrol ? 9 : type == MobType::Familiar ? 2 : 10;
        assert(shots30.Count() == static_cast<std::size_t>(expected));
        assert(shots60.Count() == static_cast<std::size_t>(expected));
        assert(almostEqual(mob30.X(), mob60.X()) && almostEqual(mob30.Y(), mob60.Y()));
    }

    ScenePlay scene;
    waves(scene);
    scene.Tick(0.8F, 0, 0, false, false);
    assert(scene.m_mobs.size() == 1);
    const float elapsed = scene.m_elapsedSeconds;
    const float spawnTimer = scene.m_spawnTimer;
    const float mobY = scene.m_mobs.front().Y();
    const float playerX = scene.m_player.X();
    scene.m_player.Damage(1);
    scene.m_pickups.push_back({400, 200, ScenePlay::PickupType::Score});
    scene.m_scorePopups.push_back({240, 400});
    scene.m_paused = true;
    scene.Tick(30, 1, 0, true, true, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed));
    assert(almostEqual(scene.m_spawnTimer, spawnTimer));
    assert(almostEqual(scene.m_mobs.front().Y(), mobY));
    assert(almostEqual(scene.m_player.X(), playerX) && scene.m_bullets.Count() == 0);
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 1.5F));
    assert(scene.m_player.Bombs() == 3);
    assert(scene.m_pickups.front().age == 0 && scene.m_pickups.front().y == 200);
    assert(scene.m_scorePopups.front().age == 0);
    scene.m_paused = false;
    scene.Tick(0.05F, 0, 0, false, true);
    assert(almostEqual(scene.m_elapsedSeconds, elapsed + 0.05F));
    assert(scene.m_bullets.Count() == 1);

    waves(scene);
    scene.Tick(5.4F, 0, 0, false, false);
    assert(scene.m_stagePhase == ScenePlay::StagePhase::Waves && scene.m_mobs.size() == 4);
    assert(scene.m_player.Health() == 3);
    for (MobType type : types)
        assert(std::any_of(scene.m_mobs.begin(), scene.m_mobs.end(),
                          [type](const GameMob& mob) { return mob.Type() == type; }));

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
    assert(scene.m_score == 0 && scene.m_mobs.size() == 1);
    const auto dropCount = scene.m_pickups.size();
    assert(dropCount >= 2 && dropCount <= 3);
    assert(scene.m_mobs.front().Health() == 1 && scene.m_bullets.Count() == 0);
    scene.Collide();
    assert(scene.m_score == 0 && scene.m_pickups.size() == dropCount);
    scene.m_mobs.front().Update(11);
    scene.Collide();
    assert(scene.m_mobs.empty() && scene.m_score == 0 && scene.m_pickups.size() == dropCount);
    for (auto& item : scene.m_pickups) { item.x = scene.m_player.X(); item.y = scene.m_player.Y(); }
    scene.UpdatePickups(0);
    assert(scene.m_score == 100 && scene.m_player.Power() == 1 && scene.m_pickups.empty());
    assert(scene.m_scorePopups.size() == 1);
    scene.UpdatePickups(0);
    assert(scene.m_score == 100 && scene.m_player.Power() == 1);

    waves(scene);
    GameMob fastTarget(240);
    fastTarget.Update(260.0F / 60.0F);
    fastTarget.Damage(2);
    scene.m_mobs.push_back(fastTarget);
    scene.m_bullets.Add({240, 300, -600});
    scene.Tick(0.2F, 0, 0, false, false);
    assert(scene.m_score == 0 && scene.m_mobs.empty() && scene.m_pickups.size() >= 2);

    // Drop thresholds are deterministic in checks; gameplay rolls uniformly from 0 to 99.
    for (const int roll : {0, 4, 5, 9, 10, 99})
    {
        waves(scene);
        scene.DropLoot(-1000, -1000, roll);
        assert(scene.m_pickups.size() == static_cast<std::size_t>(roll < 10 ? 3 : 2));
        assert(scene.m_pickups[0].type == ScenePlay::PickupType::Score);
        assert(scene.m_pickups[1].type == ScenePlay::PickupType::Power);
        if (roll < 10)
            assert(scene.m_pickups[2].type == (roll < 5 ? ScenePlay::PickupType::Health : ScenePlay::PickupType::Bomb));
        for (const auto& item : scene.m_pickups)
            assert(item.x >= 14 && item.x <= 466 && item.y >= 14 && item.y <= 586);
        scene.m_pickups.clear();
        scene.DropLoot(1000, 1000, roll);
        for (const auto& item : scene.m_pickups)
            assert(item.x >= 14 && item.x <= 466 && item.y >= 14 && item.y <= 586);
    }
    waves(scene);
    scene.m_pickups.push_back({scene.m_player.X() + 22.01F, scene.m_player.Y(), ScenePlay::PickupType::Score});
    scene.UpdatePickups(0);
    assert(scene.m_score == 0 && scene.m_pickups.size() == 1);
    scene.m_pickups.front().x = scene.m_player.X() + 22.0F;
    scene.UpdatePickups(0);
    assert(scene.m_score == 100 && scene.m_pickups.empty());
    waves(scene);
    scene.m_pickups.push_back({scene.m_player.X() + 80, scene.m_player.Y(), ScenePlay::PickupType::Score});
    scene.UpdatePickups(0.05F);
    assert(scene.m_pickups.front().attracted && almostEqual(scene.m_pickups.front().x, scene.m_player.X() + 66));
    scene.UpdatePickups(0.2F);
    assert(scene.m_score == 100 && scene.m_pickups.empty());
    waves(scene);
    scene.m_pickups.push_back({scene.m_player.X() + 80.01F, scene.m_player.Y(), ScenePlay::PickupType::Score});
    scene.UpdatePickups(0.1F);
    assert(!scene.m_pickups.front().attracted && almostEqual(scene.m_pickups.front().y, 516));
    scene.UpdatePickups(11.9F);
    assert(scene.m_pickups.empty() && scene.m_score == 0);
    waves(scene);
    for (int i = 0; i < 3; ++i)
    {
        scene.m_pickups.push_back({240, 520, ScenePlay::PickupType::Health});
        scene.m_pickups.push_back({240, 520, ScenePlay::PickupType::Bomb});
    }
    for (int i = 0; i < 6; ++i) scene.m_pickups.push_back({240, 520, ScenePlay::PickupType::Power});
    scene.UpdatePickups(0);
    assert(scene.m_player.Health() == 5 && scene.m_player.Bombs() == 5 && scene.m_player.Power() == 4);
    assert(scene.m_pickups.empty() && scene.m_score == 0);
    waves(scene);
    scene.m_player.Damage(2);
    scene.m_player.Update(2, 0, 0, false, false);
    scene.m_pickups.push_back({240, 520, ScenePlay::PickupType::Health});
    scene.m_bullets.Add(enemyBullet(240, 520));
    scene.Tick(0.01F, 0, 0, false, false);
    assert(scene.m_player.Dead() && scene.m_stagePhase == ScenePlay::StagePhase::Finished);
    assert(scene.m_pickups.size() == 1 && scene.m_player.Health() == 0);

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
    scene.m_pickups.push_back({400, 200, ScenePlay::PickupType::Power});
    scene.m_mobs.emplace_back(96);
    scene.m_bullets.Add(enemyBullet(400, 200));
    scene.Tick(0.02F, 0, 0, false, false);
    assert(scene.m_stagePhase == ScenePlay::StagePhase::BossIntro);
    assert(scene.m_mobs.empty() && scene.m_bullets.Count() == 0);
    assert(scene.m_pickups.size() == 1);
    const float dialogueItemAge = scene.m_pickups.front().age;
    const float dialogueItemY = scene.m_pickups.front().y;
    const float dialogueTime = scene.m_elapsedSeconds;
    const float dialogueBossX = scene.m_boss.X();
    const float dialogueBossY = scene.m_boss.Y();
    scene.m_player.Damage(1);
    scene.Tick(30, 1, 0, false, true, true);
    assert(almostEqual(scene.m_elapsedSeconds, dialogueTime));
    assert(almostEqual(scene.m_pickups.front().age, dialogueItemAge));
    assert(almostEqual(scene.m_pickups.front().y, dialogueItemY));
    assert(almostEqual(scene.m_boss.X(), dialogueBossX) && almostEqual(scene.m_boss.Y(), dialogueBossY));
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 1.5F));
    assert(scene.m_player.Bombs() == 3);

    GameBossMob moving;
    BulletManager movingShots;
    moving.Update(2.625F, movingShots, 240, 520);
    assert(almostEqual(moving.X(), 340) && !almostEqual(moving.Y(), 120));
    for (int i = 0; i < 28; ++i)
    {
        moving.Update(0.25F, movingShots, 240, 520);
        assert(moving.X() >= 140 && moving.X() <= 340);
        assert(moving.Y() >= 108 && moving.Y() <= 132);
    }
    moving = GameBossMob{};
    assert(almostEqual(moving.X(), 240) && almostEqual(moving.Y(), 120));

    GameBossMob boss;
    BulletManager bossBullets;
    boss.Update(0.85F, bossBullets, 96, 520);
    assert(bossBullets.Count() == 7 && aimedAt(bossBullets.Bullets()[3], 96, 520, 155));
    for (const auto& bullet : bossBullets.Bullets()) assert(hasSpeed(bullet, 155));
    bossBullets.Add({240, 300, 0});
    boss.Damage(250);
    assert(boss.AdvancePhase(bossBullets) && boss.PhaseTwo());
    assert(bossBullets.Count() == 1 && !boss.AdvancePhase(bossBullets));
    boss.Update(0.99F, bossBullets, 384, 520);
    assert(bossBullets.Count() == 1);
    boss.Update(0.01F, bossBullets, 384, 520);
    assert(bossBullets.Count() == 37);
    const GameBullet firstRing = bossBullets.Bullets()[1];
    boss.Update(1.0F, bossBullets, 384, 520);
    assert(bossBullets.Count() == 78);
    const auto& secondRing = bossBullets.Bullets()[37];
    assert(almostEqual(std::atan2(secondRing.velocityY, secondRing.velocityX)
                     - std::atan2(firstRing.velocityY, firstRing.velocityX),
                       std::numbers::pi_v<float> / 30.0F));
    assert(std::count_if(bossBullets.Bullets().begin(), bossBullets.Bullets().end(),
        [](const GameBullet& bullet) { return bullet.team == BulletTeam::Enemy && bullet.orb; }) == 72);
    assert(aimedAt(bossBullets.Bullets()[75], 384, 520, 175));
    for (std::size_t i = 1; i < bossBullets.Count(); ++i)
        assert(hasSpeed(bossBullets.Bullets()[i], i < 73 ? 125.0F : 175.0F));
    boss.Damage(250);
    assert(boss.Dead() && !boss.AdvancePhase(bossBullets));
    boss.Update(2, bossBullets, 384, 520);
    assert(bossBullets.Count() == 78);
    GameBossMob boss30;
    GameBossMob boss60;
    BulletManager bossShots30;
    BulletManager bossShots60;
    for (int i = 0; i < 90; ++i) boss30.Update(1.0F / 30, bossShots30, 96, 520);
    for (int i = 0; i < 180; ++i) boss60.Update(1.0F / 60, bossShots60, 96, 520);
    assert(bossShots30.Count() == 21 && bossShots60.Count() == 21);
    assert(almostEqual(boss30.X(), boss60.X()) && almostEqual(boss30.Y(), boss60.Y()));
    boss30.Damage(250);
    boss60.Damage(250);
    assert(boss30.AdvancePhase(bossShots30) && boss60.AdvancePhase(bossShots60));
    for (int i = 0; i < 93; ++i) boss30.Update(1.0F / 30, bossShots30, 384, 520);
    for (int i = 0; i < 186; ++i) boss60.Update(1.0F / 60, bossShots60, 384, 520);
    assert(bossShots30.Count() == 113 && bossShots60.Count() == 113);
    assert(almostEqual(boss30.X(), boss60.X()) && almostEqual(boss30.Y(), boss60.Y()));

    waves(scene);
    scene.m_stagePhase = ScenePlay::StagePhase::Boss;
    scene.Tick(0.4F, 0, 0, false, false);
    const float pausedBossX = scene.m_boss.X();
    const float pausedBossY = scene.m_boss.Y();
    const float pausedBossTime = scene.m_elapsedSeconds;
    scene.m_paused = true;
    scene.Tick(30, 1, 0, false, true, true);
    assert(almostEqual(scene.m_boss.X(), pausedBossX) && almostEqual(scene.m_boss.Y(), pausedBossY));
    assert(almostEqual(scene.m_elapsedSeconds, pausedBossTime));
    scene.m_paused = false;
    scene.Tick(0.1F, 0, 0, false, false);
    assert(!almostEqual(scene.m_boss.X(), pausedBossX));

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
    scene.m_player = GamePlayer{};
    scene.m_player.Heal(); scene.m_player.Heal();
    scene.m_player.AddBomb(); scene.m_player.AddBomb();
    scene.m_player.AddPower();
    scene.m_pickups.push_back({240, 200, ScenePlay::PickupType::Score});
    scene.m_scorePopups.push_back({240, 400});
    scene.m_score = 1234;
    scene.m_paused = true;
    scene.Reset();
    assert(scene.m_stagePhase == ScenePlay::StagePhase::Intro);
    assert(scene.m_player.Health() == 3 && scene.m_player.Bombs() == 3 && scene.m_player.Power() == 0);
    assert(almostEqual(scene.m_player.InvulnerabilitySeconds(), 0));
    assert(scene.m_score == 0 && scene.m_elapsedSeconds == 0 && scene.m_waveSeconds == 0);
    assert(scene.m_bullets.Count() == 0 && scene.m_mobs.empty() && scene.m_effects.empty());
    assert(scene.m_pickups.empty() && scene.m_scorePopups.empty());
    assert(scene.m_boss.Health() == 500 && !scene.m_boss.PhaseTwo());
    assert(almostEqual(scene.m_boss.X(), 240) && almostEqual(scene.m_boss.Y(), 120));
    assert(!scene.m_paused && !scene.m_cleared);
    scene.Tick(10, 1, 0, true, true, true);
    assert(scene.m_elapsedSeconds == 0 && scene.m_bullets.Count() == 0);
}
#endif
