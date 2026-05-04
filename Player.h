#ifndef PLAYER_H
#define PLAYER_H

#include "GameTypes.h"
#include <functional>
#include <QSet>
#include <QPainter>

class Player {
public:
    Player();
    void reset(float startX, float startY);
    void update(const QSet<int>& keys, int w, int h, int frameCnt);
    void draw(QPainter& p) const;

    void setBulletAdder(std::function<void(const Bullet&)> adder);
    void tryShoot();
    void useSkill(int id);
    void updateSkills();
    void hit();
    void getPickup(int type);

    float x() const { return m_x; }
    float y() const { return m_y; }
    int hp() const { return m_hp; }
    int maxHp() const { return m_maxHp; }
    int dmg() const { return m_dmg; }
    int invTimer() const { return m_invTimer; }
    int radius() const { return 12; }
    bool isInv() const { return m_invTimer > 0; }
    Skill& skillBomb() { return m_bomb; }
    Skill& skillShield() { return m_shield; }
    Skill& skillFire() { return m_fire; }

private:
    float m_x = 0, m_y = 0;
    float m_speed = 5.0f;
    int m_hp = 15, m_maxHp = 15;
    int m_shootTimer = 0;
    int m_shootInterval = 8;
    int m_invTimer = 0;
    int m_dmg = 1;
    Skill m_bomb, m_shield, m_fire;
    std::function<void(const Bullet&)> m_addBullet;
};

#endif