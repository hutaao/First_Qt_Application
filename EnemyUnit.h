#ifndef ENEMYUNIT_H
#define ENEMYUNIT_H

#include "GameTypes.h"
#include <functional>
#include <QPainter>

class EnemyUnit {
public:
    EnemyUnit();
    void reset(float x, float y, int type, int wave);
    void update(float px, float py, int frameCnt);
    void draw(QPainter& p) const;

    void setBulletAdder(std::function<void(const Bullet&)> adder);
    Enemy& data() { return m_data; }
    const Enemy& data() const { return m_data; }
    bool isAlive() const { return m_data.alive; }
    void kill() { m_data.alive = false; }

private:
    Enemy m_data;
    std::function<void(const Bullet&)> m_addBullet;

    void shoot(float px, float py, int frameCnt);
    void fireStraight(float x, float y, float vx, float vy, QColor color, float radius);
};

#endif