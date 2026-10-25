#pragma once

class GameCharacter
{
public:
    virtual ~GameCharacter() = default;
    virtual int Damage(int damage);
    virtual int Attack(GameCharacter* pMob, int damage)
    {
        pMob->Damage(damage);
        return 0;
    }
    float X() const { return m_x; }
    float Y() const { return m_y; }

protected:
    float m_x{};
    float m_y{};
};
