#include "ScenePlay.h"
#include "GameInput.h"
#include "App.h"
#include <algorithm>
#include <cstdio>

namespace
{
    bool Touching(float x1, float y1, float r1, float x2, float y2, float r2)
    {
        const float dx = x1 - x2;
        const float dy = y1 - y2;
        const float radius = r1 + r2;
        return dx * dx + dy * dy <= radius * radius;
    }
}

void ScenePlay::Reset()
{
    m_player = GamePlayer{};
    m_boss = GameBossMob{};
    m_bullets.Clear();
    m_mobs.clear();
    m_effects.clear();
    m_stagePhase = StagePhase::Intro;
    m_elapsedSeconds = m_waveSeconds = 0.0F;
    m_hitEffectSeconds = m_bombEffectSeconds = 0.0F;
    m_spawnTimer = 0.75F;
    m_spawnCount = m_score = m_selected = 0;
    m_events = 0;
    m_paused = m_cleared = false;
}

int ScenePlay::Init()
{
    Reset();
    g_pApp->Resources().PlayBgm(SoundId::Forest);
    return 0;
}

int ScenePlay::Destroy()
{
    m_bullets.Clear();
    m_mobs.clear();
    m_effects.clear();
    return 0;
}

void ScenePlay::Finish(bool cleared)
{
    m_cleared = cleared;
    m_stagePhase = StagePhase::Finished;
}

void ScenePlay::Collide()
{
    if (m_stagePhase == StagePhase::Finished)
        return;
    for (auto& bullet : m_bullets.Bullets())
    {
        if (!bullet.active || bullet.team != BulletTeam::Player)
            continue;
        for (auto& mob : m_mobs)
        {
            if (mob.Dead() || !Touching(bullet.x, bullet.y, bullet.radius,
                                        mob.X(), mob.Y(), GameMob::Radius))
                continue;
            bullet.active = false;
            if (mob.Damage(bullet.damage) > 0 && mob.Dead())
            {
                m_score += 100;
                m_effects.push_back({mob.X(), mob.Y()});
                m_events |= EnemyDown;
            }
            break;
        }
        if (bullet.active && m_stagePhase == StagePhase::Boss && !m_boss.Dead()
            && Touching(bullet.x, bullet.y, bullet.radius,
                        m_boss.X(), m_boss.Y(), GameBossMob::Radius))
        {
            bullet.active = false;
            m_boss.Damage(bullet.damage);
        }
    }
    for (auto& bullet : m_bullets.Bullets())
    {
        if (bullet.active && bullet.team == BulletTeam::Enemy
            && Touching(bullet.x, bullet.y, bullet.radius,
                        m_player.X(), m_player.Y(), GamePlayer::Radius))
        {
            bullet.active = false;
            if (m_player.Damage(bullet.damage) > 0)
            {
                m_hitEffectSeconds = 0.24F;
                m_events |= Hit;
            }
        }
    }
    for (const auto& mob : m_mobs)
        if (!mob.Dead() && Touching(mob.X(), mob.Y(), GameMob::Radius,
                                   m_player.X(), m_player.Y(), GamePlayer::Radius)
            && m_player.Damage(1) > 0)
        {
            m_hitEffectSeconds = 0.24F;
            m_events |= Hit;
        }
    if (m_stagePhase == StagePhase::Boss && !m_boss.Dead()
        && Touching(m_boss.X(), m_boss.Y(), GameBossMob::Radius,
                    m_player.X(), m_player.Y(), GamePlayer::Radius)
        && m_player.Damage(1) > 0)
    {
        m_hitEffectSeconds = 0.24F;
        m_events |= Hit;
    }
    // 동시 마지막 피격과 반피 전환보다 보스 처치를 먼저 확정한다.
    if (m_stagePhase == StagePhase::Boss && m_boss.Dead())
    {
        m_score += 5000;
        m_effects.push_back({m_boss.X(), m_boss.Y()});
        m_bullets.ClearEnemy();
        Finish(true);
    }
    else if (m_player.Dead())
        Finish(false);
    else if (m_stagePhase == StagePhase::Boss)
        m_boss.AdvancePhase(m_bullets);
    m_bullets.RemoveInactive();
    std::erase_if(m_mobs, [](const GameMob& mob) { return mob.Dead() || mob.Outside(); });
}

void ScenePlay::Step(float seconds, float directionX, float directionY, bool slow, bool shooting)
{
    m_elapsedSeconds += seconds;
    m_hitEffectSeconds = std::max(0.0F, m_hitEffectSeconds - seconds);
    m_bombEffectSeconds = std::max(0.0F, m_bombEffectSeconds - seconds);
    for (auto& effect : m_effects)
        effect.age += seconds;
    std::erase_if(m_effects, [](const Effect& effect) { return effect.age >= 0.36F; });
    m_player.Update(seconds, directionX, directionY, slow, shooting);
    if (m_bullets.Update(seconds, shooting, m_player.X(), m_player.Y()))
        m_events |= Shot;
    if (m_stagePhase == StagePhase::Waves)
    {
        m_waveSeconds += seconds;
        m_spawnTimer -= seconds;
        constexpr float positions[] = {96.0F, 240.0F, 384.0F};
        while (m_spawnTimer <= 0.0F)
        {
            m_mobs.emplace_back(positions[m_spawnCount % 3]);
            m_spawnCount = (m_spawnCount + 1) % 3;
            m_spawnTimer += 1.5F;
        }
        for (auto& mob : m_mobs)
            mob.Update(seconds, m_bullets);
    }
    else if (m_stagePhase == StagePhase::Boss)
        m_boss.Update(seconds, m_bullets);
    Collide();
    if (m_stagePhase == StagePhase::Waves && m_waveSeconds >= 90.0F)
    {
        m_mobs.clear();
        m_bullets.ClearEnemy();
        m_stagePhase = StagePhase::BossIntro;
    }
}

bool ScenePlay::Tick(float seconds, float directionX, float directionY, bool slow,
                     bool shooting, bool bombPressed)
{
    m_events = 0;
    if (m_paused || (m_stagePhase != StagePhase::Waves && m_stagePhase != StagePhase::Boss))
        return false;
    if (bombPressed && m_player.UseBomb())
    {
        m_bullets.ClearEnemy();
        m_bombEffectSeconds = 0.36F;
        m_events |= Bomb;
    }
    // 빠른 플레이어 탄이 낮은 프레임 속도에서 작은 적을 건너뛰지 않게 한다.
    while (seconds > 0.000001F
           && (m_stagePhase == StagePhase::Waves || m_stagePhase == StagePhase::Boss))
    {
        const float step = std::min(seconds, 1.0F / 120.0F);
        Step(step, directionX, directionY, slow, shooting);
        seconds -= step;
    }
    return (m_events & Shot) != 0;
}

int ScenePlay::Update()
{
    const auto* input = GetGameInput();
    if (input->Pressed(VK_ESCAPE))
    {
        m_paused = !m_paused;
        m_selected = 0;
        return 0;
    }
    if (m_paused)
    {
        MoveSelection(m_selected, 4);
        if (input->Pressed(VK_RETURN))
        {
            g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
            if (m_selected == 0)
                m_paused = false;
            else if (m_selected == 1)
                Init();
            else
                g_pApp->ChangeScene(m_selected == 2 ? SCENE_LOBBY : SCENE_BEGIN);
        }
        return 0;
    }
    if (m_stagePhase == StagePhase::Intro || m_stagePhase == StagePhase::BossIntro)
    {
        if (input->Pressed(VK_RETURN))
        {
            g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
            if (m_stagePhase == StagePhase::Intro)
                m_stagePhase = StagePhase::Waves;
            else
            {
                m_stagePhase = StagePhase::Boss;
                g_pApp->Resources().PlayBgm(SoundId::Boss);
            }
        }
        return 0;
    }
    const float x = static_cast<float>(input->Held(VK_RIGHT)) - input->Held(VK_LEFT);
    const float y = static_cast<float>(input->Held(VK_DOWN)) - input->Held(VK_UP);
    Tick(g_pApp->DeltaSeconds(), x, y, input->Held(VK_SHIFT), input->Held('Z'), input->Pressed('X'));
    auto& resources = g_pApp->Resources();
    if (m_events & Shot) resources.PlaySound(SoundId::Shot);
    if (m_events & Hit) resources.PlaySound(SoundId::Hit);
    if (m_events & EnemyDown) resources.PlaySound(SoundId::EnemyDown);
    if (m_events & Bomb) resources.PlaySound(SoundId::Bomb);
    if (m_stagePhase == StagePhase::Finished)
    {
        g_pApp->SetResult({m_cleared, m_score, m_elapsedSeconds});
        g_pApp->ChangeScene(SCENE_PLAYRESULT);
    }
    return 0;
}

int ScenePlay::Render()
{
    auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Forest, 0, 0, 1.0F, 0xFFB8B8B8);
    for (const auto& mob : m_mobs)
        mob.Render(resources);
    if (m_stagePhase == StagePhase::Boss || m_stagePhase == StagePhase::BossIntro)
        m_boss.Render(resources);
    m_bullets.Render(resources);
    m_player.Render(resources);
    for (const auto& effect : m_effects)
        resources.DrawFrame(ImageId::Explosion, std::min(5, static_cast<int>(effect.age / 0.06F)),
                            0, 64, 64, effect.x - 32.0F, effect.y - 32.0F);
    if (m_hitEffectSeconds > 0.0F)
        resources.DrawFrame(ImageId::PlayerHit, std::min(3, static_cast<int>((0.24F - m_hitEffectSeconds) / 0.06F)),
                            0, 64, 64, m_player.X() - 32.0F, m_player.Y() - 32.0F);
    if (m_bombEffectSeconds > 0.0F)
    {
        resources.Rect(0, 0, 480, 600, 0x30A99CFF);
        resources.DrawFrame(ImageId::BombEffect, std::min(5, static_cast<int>((0.36F - m_bombEffectSeconds) / 0.06F)),
                            0, 128, 128, m_player.X() - 64.0F, m_player.Y() - 64.0F);
    }
    // Draw the sidebar after combat effects so their sprites stay inside the playfield.
    resources.DrawFrame(ImageId::Title, 1, 0, 480, 600, 480, 0);
    resources.Rect(480, 0, 480, 600, 0xDC0E111A);
    resources.Draw(ImageId::LeonArt, 688, 244, 0.35F, 0xB0FFFFFF);
    resources.Rect(480, 0, 1, 600, 0x80454B69);
    Text(520, 32, "REBORNBOSS", UIColor::Muted, FontStyle::Caption);
    Text(520, 72, "꽃빛 요정의 숲", UIColor::Text, FontStyle::Heading);
    Text(520, 115, "STAGE 01 / LILLIAN BLOOM", UIColor::Muted, FontStyle::Caption);
    resources.Rect(520, 152, 392, 1, UIColor::Border);
    Text(520, 176, "SCORE", UIColor::Muted, FontStyle::Caption);
    char number[64]{};
    std::snprintf(number, sizeof(number), "%07d", m_score);
    Text(520, 204, number, UIColor::Text, FontStyle::Title);
    Text(520, 280, "LIFE", UIColor::Muted, FontStyle::Caption);
    Text(520, 326, "BOMB", UIColor::Muted, FontStyle::Caption);
    for (int i = 0; i < m_player.Health(); ++i)
        resources.Draw(ImageId::Life, 608.0F + i * 32.0F, 278);
    for (int i = 0; i < m_player.Bombs(); ++i)
        resources.Draw(ImageId::Bomb, 608.0F + i * 32.0F, 324);
    if (m_player.Health() == 0)
        Text(608, 280, "0", UIColor::Muted, FontStyle::Caption);
    if (m_player.Bombs() == 0)
        Text(608, 326, "0", UIColor::Muted, FontStyle::Caption);
    resources.Rect(520, 372, 176, 1, UIColor::Border);
    Text(520, 398, "TIME", UIColor::Muted, FontStyle::Caption);
    const int elapsed = static_cast<int>(m_elapsedSeconds);
    std::snprintf(number, sizeof(number), "%02d:%02d", elapsed / 60, elapsed % 60);
    Text(520, 428, number, UIColor::Text, FontStyle::Body);
    const bool boss = m_stagePhase == StagePhase::Boss || m_stagePhase == StagePhase::BossIntro;
    Text(520, 488, boss ? "BOSS BATTLE" : "FOREST", UIColor::Muted, FontStyle::Caption);
    if (!boss)
    {
        resources.Rect(520, 522, 176, 3, UIColor::Border);
        resources.Rect(520, 522, 176.0F * std::clamp(m_waveSeconds / 90.0F, 0.0F, 1.0F), 3, UIColor::Accent);
    }
    Text(520, 560, "Esc  일시정지", UIColor::Muted, FontStyle::Caption);
    if (m_stagePhase == StagePhase::Boss)
    {
        resources.Rect(0, 0, 480, 64, 0xC0080D17);
        Text(24, 14, m_boss.PhaseTwo() ? "릴리안 블룸 · 만개" : "릴리안 블룸 · 꽃잎 부채",
             UIColor::Text, FontStyle::Caption);
        resources.Rect(24, 46, 432, 4, UIColor::Border);
        resources.Rect(24, 46, 432.0F * m_boss.Health() / GameBossMob::MaxHealth, 4, UIColor::Accent);
    }
    if (m_stagePhase == StagePhase::Intro || m_stagePhase == StagePhase::BossIntro)
    {
        const bool bossIntro = m_stagePhase == StagePhase::BossIntro;
        resources.Rect(0, 0, 960, 600, 0x40000000);
        resources.Rect(0, 402, 960, 198, 0xF0080D17);
        resources.Draw(bossIntro ? ImageId::LillianPortrait : ImageId::LeonPortrait, 32, 346, 0.5F);
        Text(224, 419, bossIntro ? "릴리안 블룸" : "레온 애쉬포드", UIColor::Accent, FontStyle::Heading);
        resources.Rect(224, 458, 672, 1, UIColor::Border);
        Text(224, 480, bossIntro ? "남의 집에 들어오자마자 무슨 소리예요?"
                                   : "좋아, 오늘부터 이 세계의 최종 보스는 나다!",
             UIColor::Text, FontStyle::Body);
        Text(224, 550, "Enter로 계속", UIColor::Muted, FontStyle::Caption);
    }
    if (m_paused)
    {
        resources.Rect(0, 0, 960, 600, 0xF0080D17);
        Text(96, 100, "일시정지", UIColor::Text, FontStyle::Title);
        resources.Rect(96, 166, 64, 2, UIColor::Accent);
        constexpr const char* labels[] = {"계속하기", "처음부터 다시", "스테이지 선택", "타이틀로"};
        for (int i = 0; i < 4; ++i)
            MenuItem(96, 210 + i * 68, labels[i], m_selected == i, 344);
        Text(560, 140, "조작", UIColor::Text, FontStyle::Heading);
        resources.Rect(560, 188, 304, 1, UIColor::Border);
        constexpr const char* keys[] = {"방향키", "Z", "Shift", "X"};
        constexpr const char* actions[] = {"이동", "연속 사격", "저속 / 중심점", "폭탄"};
        for (int i = 0; i < 4; ++i)
        {
            Text(560, 224 + i * 44, keys[i], UIColor::Accent, FontStyle::Caption);
            Text(650, 224 + i * 44, actions[i], UIColor::Text, FontStyle::Caption);
        }
        Text(560, 444, "작은 중심점으로 탄막을 피하세요.", UIColor::Muted, FontStyle::Caption);
        Text(96, 536, "Enter로 선택 · Esc로 계속", UIColor::Muted, FontStyle::Caption);
    }
    return 0;
}
