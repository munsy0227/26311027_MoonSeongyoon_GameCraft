#include "ScenePlay.h"
#include "GameInput.h"
#include "App.h"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <cmath>

namespace
{
    struct DialogueLine { bool lillian; const char* line1; const char* line2; };
    constexpr DialogueLine IntroDialogue[] = {
        {false, "분명 내 방에서 잠들었는데...", "눈을 뜨니 숲이라니. 전생인가?"},
        {false, "보라색 망토에 붉은 마력 문양.", "누가 봐도 최종 보스의 차림이군!"},
        {false, "좋아. 이 숲을 내 영지로 삼겠다.", "큰 나무 밑에 성을 짓고 낮잠을..."},
        {true, "거기, 망토 입은 분!", "발밑의 꽃부터 피해서 걸어요!"},
        {false, "오, 벌써 내 등장을 알아봤나?", "숲의 지배자를 맞이할 준비는..."},
        {true, "침입자를 알아본 거예요.", "그쪽 길은 요정들이 지키고 있어요."},
        {false, "첫 영지에는 시련도 따르는 법!", "길만 비켜 주면 서로 편할 텐데."},
        {true, "편한 건 그쪽뿐이잖아요.", "꽃 한 송이도 밟지 말고 와 봐요!"},
    };

    constexpr DialogueLine BossDialogue[] = {
        {false, "드디어 이 숲의 책임자를 찾았군!", "오늘부터 이곳은 내 영지다!"},
        {true, "남의 집에 들어오자마자 무슨 소리예요?", "저는 이 숲을 지키는 릴리안이에요."},
        {false, "레온 애쉬포드. 곧 이세계의 최종 보스.", "통치에 앞서 나무 그늘부터..."},
        {true, "그 통치라는 게 설마 낮잠인가요?", "요정들이 일하는 동안 혼자만요?"},
        {false, "최종 보스에게도 휴식은 필요하다.", "싸우지 않고 영지를 얻을 방법은..."},
        {true, "있죠. 정복하겠다는 말부터 거둬요.", "그러면 손님용 그늘은 빌려줄게요."},
        {false, "빌리는 건 최종 보스답지 않다!", "초록 구슬쯤은 주워 줄 수도 있다만."},
        {true, "흘린 마력을 치워 주면 좋긴 하죠.", "그걸로 주인이 되는 건 아니에요."},
        {false, "P 문양의 마력까지 모으면...", "내 마법도 훨씬 넓게 퍼질 테지!"},
        {true, "힘을 모을 생각만 가득하네요.", "꽃을 피할 생각도 좀 해 줘요."},
        {false, "꽃만 피하면 길을 열어 주겠나?", "나도 품위 없는 정복은 사양이지."},
        {true, "우리 숲을 빼앗으려는 동안은 안 돼요.", "요정들 대신 제가 상대할게요."},
        {false, "좋다! 최종 보스의 첫 무대를 열겠다!", "레온 애쉬포드, 영지 확보를 개시한다!"},
        {true, "첫 무대부터 남의 집이라니...", "꽃잎 사이로 잘 피해 보세요!"},
    };

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
    m_pickups.clear();
    m_scorePopups.clear();
    m_dropRandom.seed(0x52424F53U);
    m_stagePhase = StagePhase::Intro;
    m_elapsedSeconds = m_waveSeconds = 0.0F;
    m_hitEffectSeconds = m_bombEffectSeconds = 0.0F;
    m_spawnTimer = 0.75F;
    m_spawnCount = m_score = m_selected = 0;
    m_events = 0;
    m_dialogueIndex = 0;
    m_fastDialogueSeconds = 0.0F;
    m_dialogueReady = false;
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
    m_pickups.clear();
    m_scorePopups.clear();
    return 0;
}

void ScenePlay::Finish(bool cleared)
{
    m_cleared = cleared;
    m_stagePhase = StagePhase::Finished;
}

int ScenePlay::DialogueCount() const
{
    if (m_stagePhase == StagePhase::Intro)
        return static_cast<int>(std::size(IntroDialogue));
    if (m_stagePhase == StagePhase::BossIntro)
        return static_cast<int>(std::size(BossDialogue));
    return 0;
}

bool ScenePlay::UpdateDialogue(float seconds, bool advanceHeld, bool advancePressed, bool fastHeld)
{
    if (m_paused || DialogueCount() == 0)
        return false;
    if (!advanceHeld)
        m_dialogueReady = true;
    m_fastDialogueSeconds = fastHeld ? m_fastDialogueSeconds + seconds : 0.0F;
    // Enter must be released between lines, including after entering or resuming dialogue.
    if (!(m_dialogueReady && advancePressed) && m_fastDialogueSeconds < 0.12F)
        return false;
    m_dialogueReady = false;
    m_fastDialogueSeconds = 0.0F;
    if (++m_dialogueIndex >= DialogueCount())
    {
        m_stagePhase = m_stagePhase == StagePhase::Intro ? StagePhase::Waves : StagePhase::Boss;
        m_dialogueIndex = 0;
    }
    return true;
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
                DropLoot(mob.X(), mob.Y(), std::uniform_int_distribution<int>{0, 99}(m_dropRandom));
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

void ScenePlay::DropLoot(float x, float y, int roll)
{
    x = std::clamp(x, 32.0F, 448.0F);
    y = std::clamp(y, 24.0F, 564.0F);
    m_pickups.push_back({x - 14.0F, y, PickupType::Score});
    m_pickups.push_back({x + 14.0F, y, PickupType::Power});
    if (roll < 5)
        m_pickups.push_back({x, y + 16.0F, PickupType::Health});
    else if (roll < 10)
        m_pickups.push_back({x, y + 16.0F, PickupType::Bomb});
}

void ScenePlay::UpdatePickups(float seconds)
{
    if (m_player.Dead())
        return;
    for (auto it = m_pickups.begin(); it != m_pickups.end();)
    {
        auto& item = *it;
        const float previousAge = item.age;
        item.age += seconds;
        const float dx = m_player.X() - item.x;
        const float dy = m_player.Y() - item.y;
        const float distance = std::hypot(dx, dy);
        const bool challenging = item.type == PickupType::Score || item.type == PickupType::Power;
        if (distance <= (challenging ? 36.0F : 80.0F))
            item.attracted = true;
        else if (challenging)
            item.attracted = false;
        if (item.attracted && distance > 0.001F)
        {
            const float travel = std::min(distance, (challenging ? 220.0F : 280.0F) * seconds);
            item.x += dx * travel / distance;
            item.y += dy * travel / distance;
        }
        else if (!item.attracted)
        {
            const float rising = std::min(seconds, std::max(0.0F, 0.25F - previousAge));
            item.y += -40.0F * rising + (challenging ? 90.0F : 28.0F) * (seconds - rising);
        }
        if (item.age < 12.0F && Touching(item.x, item.y, 18.0F,
                                        m_player.X(), m_player.Y(), GamePlayer::Radius))
        {
            switch (item.type)
            {
            case PickupType::Score:
                m_score += 100;
                m_scorePopups.push_back({item.x, item.y});
                break;
            case PickupType::Power: m_player.AddPower(); break;
            case PickupType::Health: m_player.Heal(); break;
            case PickupType::Bomb: m_player.AddBomb(); break;
            }
            m_events |= ItemCollected;
            it = m_pickups.erase(it);
        }
        else if (item.age >= 12.0F || item.y > 628.0F)
            it = m_pickups.erase(it);
        else
            ++it;
    }
}

void ScenePlay::Step(float seconds, float directionX, float directionY, bool slow, bool shooting)
{
    m_elapsedSeconds += seconds;
    m_hitEffectSeconds = std::max(0.0F, m_hitEffectSeconds - seconds);
    m_bombEffectSeconds = std::max(0.0F, m_bombEffectSeconds - seconds);
    for (auto& effect : m_effects)
        effect.age += seconds;
    std::erase_if(m_effects, [](const Effect& effect) { return effect.age >= 0.36F; });
    for (auto& popup : m_scorePopups)
        popup.age += seconds;
    std::erase_if(m_scorePopups, [](const ScorePopup& popup) { return popup.age >= 0.65F; });
    m_player.Update(seconds, directionX, directionY, slow, shooting);
    if (m_bullets.Update(seconds, shooting, m_player.X(), m_player.Y(), m_player.Power()))
        m_events |= Shot;
    if (m_stagePhase == StagePhase::Waves)
    {
        m_waveSeconds += seconds;
        m_spawnTimer -= seconds;
        struct Spawn { float x; MobType type; };
        constexpr Spawn spawns[] = {
            {96.0F, MobType::Patrol}, {384.0F, MobType::Familiar},
            {240.0F, MobType::Patrol}, {240.0F, MobType::Guardian},
            {96.0F, MobType::Familiar}, {384.0F, MobType::Patrol}
        };
        while (m_spawnTimer <= 0.0F)
        {
            const auto& spawn = spawns[m_spawnCount];
            m_mobs.emplace_back(spawn.x, spawn.type);
            m_spawnCount = (m_spawnCount + 1) % static_cast<int>(std::size(spawns));
            m_spawnTimer += 1.5F;
        }
        for (auto& mob : m_mobs)
            mob.Update(seconds, m_bullets, m_player.X(), m_player.Y());
    }
    else if (m_stagePhase == StagePhase::Boss)
        m_boss.Update(seconds, m_bullets, m_player.X(), m_player.Y());
    Collide();
    if (m_stagePhase != StagePhase::Finished)
        UpdatePickups(seconds);
    if (m_stagePhase == StagePhase::Waves && m_waveSeconds >= 90.0F)
    {
        m_mobs.clear();
        m_bullets.ClearEnemy();
        m_stagePhase = StagePhase::BossIntro;
        m_dialogueIndex = 0;
        m_dialogueReady = false;
        m_fastDialogueSeconds = 0.0F;
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
        m_dialogueReady = false;
        m_fastDialogueSeconds = 0.0F;
        return 0;
    }
    if (m_paused)
    {
        if (input->IsNewClassic() && input->Pressed('R'))
        {
            Init();
            return 0;
        }
        if (input->IsNewClassic() && input->Pressed('Q'))
        {
            g_pApp->ChangeScene(SCENE_BEGIN);
            return 0;
        }
        if (input->CancelPressed())
        {
            m_paused = false;
            return 0;
        }
        MoveSelection(m_selected, 4);
        if (input->ConfirmPressed())
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
    if (DialogueCount() > 0)
    {
        if (UpdateDialogue(g_pApp->DeltaSeconds(), input->Held(VK_RETURN),
                           input->Pressed(VK_RETURN), input->SkipDialogueHeld()))
        {
            g_pApp->Resources().PlaySound(SoundId::MenuMove);
            if (m_stagePhase == StagePhase::Boss)
                g_pApp->Resources().PlayBgm(SoundId::Boss);
        }
        return 0;
    }
    const float x = input->MovementX();
    const float y = input->MovementY();
    Tick(g_pApp->DeltaSeconds(), x, y, input->SlowHeld(), input->ShotHeld(), input->BombPressed());
    auto& resources = g_pApp->Resources();
    if (m_events & Shot) resources.PlaySound(SoundId::Shot);
    if (m_events & Hit) resources.PlaySound(SoundId::Hit);
    if (m_events & EnemyDown) resources.PlaySound(SoundId::EnemyDown);
    if (m_events & Bomb) resources.PlaySound(SoundId::Bomb);
    if (m_events & ItemCollected) resources.PlaySound(SoundId::MenuMove);
    if (m_stagePhase == StagePhase::Finished)
    {
        g_pApp->SetResult({m_cleared, m_score, m_elapsedSeconds});
        g_pApp->ChangeScene(SCENE_PLAYRESULT);
    }
    return 0;
}

int ScenePlay::Render()
{
    const bool newClassic = GetGameInput()->IsNewClassic();
    auto& resources = g_pApp->Resources();
    resources.DrawScrollingForest(m_elapsedSeconds);
    for (const auto& mob : m_mobs)
        mob.Render(resources);
    if (m_stagePhase == StagePhase::Boss || m_stagePhase == StagePhase::BossIntro)
        m_boss.Render(resources);
    m_bullets.Render(resources);
    for (const auto& item : m_pickups)
    {
        if (item.age > 10.0F && static_cast<int>(item.age * 8.0F) % 2 == 0)
            continue;
        if (item.type == PickupType::Score)
        {
            resources.Rect(item.x - 4, item.y - 11, 8, 22, 0xFF102C24);
            resources.Rect(item.x - 8, item.y - 8, 16, 16, 0xFF102C24);
            resources.Rect(item.x - 11, item.y - 4, 22, 8, 0xFF102C24);
            resources.Rect(item.x - 3, item.y - 9, 6, 18, 0xFF86E7AB);
            resources.Rect(item.x - 6, item.y - 6, 12, 12, 0xFF86E7AB);
            resources.Rect(item.x - 9, item.y - 3, 18, 6, 0xFF86E7AB);
            resources.Rect(item.x - 2, item.y - 4, 4, 6, 0xFFE5FFF0);
        }
        else if (item.type == PickupType::Power)
        {
            resources.Rect(item.x - 11, item.y - 11, 22, 22, UIColor::Accent);
            resources.Rect(item.x - 9, item.y - 9, 18, 18, 0xFF302B47);
            Text(static_cast<int>(item.x) - 5, static_cast<int>(item.y) - 9, "P", UIColor::Text, FontStyle::Caption);
        }
        else
        {
            resources.Rect(item.x - 14, item.y - 14, 28, 28, 0xE01A2030);
            resources.Draw(item.type == PickupType::Health ? ImageId::Life : ImageId::Bomb,
                           item.x - 12, item.y - 12);
        }
    }
    m_player.Render(resources);
    for (const auto& popup : m_scorePopups)
    {
        const int x = static_cast<int>(std::clamp(popup.x - 20, 8.0F, 424.0F));
        const int y = static_cast<int>(std::clamp(popup.y - 26.0F - popup.age * 32.0F, 8.0F, 556.0F));
        resources.Rect(x - 4.0F, y - 3.0F, 52, 24, 0xC0102C24);
        Text(x, y, "+100", 0xFF86E7AB, FontStyle::Caption);
    }
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
    for (int i = 0; i < GamePlayer::MaxHealth; ++i)
        resources.Draw(ImageId::Life, 608.0F + i * 32.0F, 278, 1.0F,
                       i < m_player.Health() ? 0xFFFFFFFF : 0x35FFFFFF);
    for (int i = 0; i < GamePlayer::MaxBombs; ++i)
        resources.Draw(ImageId::Bomb, 608.0F + i * 32.0F, 324, 1.0F,
                       i < m_player.Bombs() ? 0xFFFFFFFF : 0x35FFFFFF);
    resources.Rect(520, 372, 176, 1, UIColor::Border);
    Text(520, 398, "TIME", UIColor::Muted, FontStyle::Caption);
    const int elapsed = static_cast<int>(m_elapsedSeconds);
    std::snprintf(number, sizeof(number), "%02d:%02d", elapsed / 60, elapsed % 60);
    Text(520, 428, number, UIColor::Text, FontStyle::Body);
    resources.Rect(728, 390, 192, 80, 0xB00E111A);
    Text(736, 398, "POWER", UIColor::Muted, FontStyle::Caption);
    std::snprintf(number, sizeof(number), "%d / %d", m_player.Power(), GamePlayer::MaxPower);
    Text(736, 428, number, UIColor::Text, FontStyle::Body);
    if (m_player.Power() == GamePlayer::MaxPower)
        Text(812, 430, "MAX", UIColor::Accent, FontStyle::Caption);
    else
    {
        std::snprintf(number, sizeof(number), "P %d/%d",
                      m_player.PowerPoints() % GamePlayer::PowerPerLevel, GamePlayer::PowerPerLevel);
        Text(812, 430, number, UIColor::Muted, FontStyle::Caption);
    }
    for (int i = 0; i < GamePlayer::MaxPower; ++i)
    {
        const float fill = std::clamp((m_player.PowerPoints() - i * GamePlayer::PowerPerLevel)
                                      / static_cast<float>(GamePlayer::PowerPerLevel), 0.0F, 1.0F);
        resources.Rect(736.0F + i * 44.0F, 461, 36, 4, UIColor::Border);
        resources.Rect(736.0F + i * 44.0F, 461, 36 * fill, 4, UIColor::Accent);
    }
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
    if (DialogueCount() > 0)
    {
        const auto& line = m_stagePhase == StagePhase::BossIntro
            ? BossDialogue[m_dialogueIndex] : IntroDialogue[m_dialogueIndex];
        resources.Rect(0, 0, 960, 600, 0x40000000);
        resources.Rect(0, 402, 960, 198, 0xF0080D17);
        resources.Draw(line.lillian ? ImageId::LillianPortrait : ImageId::LeonPortrait, 32, 346, 0.5F);
        Text(224, 419, line.lillian ? "릴리안 블룸" : "레온 애쉬포드", UIColor::Accent, FontStyle::Heading);
        resources.Rect(224, 458, 672, 1, UIColor::Border);
        Text(224, 478, line.line1, UIColor::Text, FontStyle::Body);
        Text(224, 510, line.line2, UIColor::Text, FontStyle::Body);
        Text(224, 550, newClassic ? "Enter 다음 대사 · Ctrl / ; 빠르게 넘기기" : "Enter 다음 대사",
             UIColor::Muted, FontStyle::Caption);
        std::snprintf(number, sizeof(number), "%02d / %02d", m_dialogueIndex + 1, DialogueCount());
        Text(824, 550, number, UIColor::Muted, FontStyle::Caption);
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
        const char* keys[] = {newClassic ? "방향키/WASD" : "방향키",
                              newClassic ? "Z / J" : "Z",
                              newClassic ? "Shift/Space/L" : "Shift",
                              newClassic ? "X / K" : "X"};
        constexpr const char* actions[] = {"이동 / 텐키", "사격 / 결정", "저속 / 중심점", "폭탄 / 취소"};
        for (int i = 0; i < 4; ++i)
        {
            Text(560, 224 + i * 44, keys[i], UIColor::Accent, FontStyle::Caption);
            Text(720, 224 + i * 44, actions[i], UIColor::Text, FontStyle::Caption);
        }
        Text(560, 410, "초록: 점수 · P: 파워", UIColor::Muted, FontStyle::Caption);
        Text(560, 444, "작은 중심점으로 탄막을 피하세요.", UIColor::Muted, FontStyle::Caption);
        if (newClassic)
            Text(560, 492, "R 재도전 · Q 타이틀", UIColor::Muted, FontStyle::Caption);
        Text(96, 536, newClassic ? "Z / J / Enter 선택 · X / K / Esc 계속" : "Z로 선택 · X / Esc로 계속", UIColor::Muted, FontStyle::Caption);
    }
    return 0;
}
