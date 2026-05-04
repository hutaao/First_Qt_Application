#ifndef BOSS_H
#define BOSS_H

#include "GameTypes.h"
#include <functional>
#include <QPainter>

class Boss {
    Enemy m_data;
    std::function<void(const Bullet&)> m_addBullet;     //子弹回调容器

    void s1(float px, float py);    //  扇形扩散弹+直线弹+追踪弹
    void s2(float px, float py, int frameCnt);  //  海胆膨胀弹+追踪弹
    void s3(float px, float py);    //  满屏小圆弹+回旋镖+半弧形散弹

    void fireCircle(float x, float y, int cnt, float spd, QColor c, float r);
    void fireFan(float x, float y, float tx, float ty, int cnt, float spread, float spd, QColor c, float r);
    void fireStraight(float x, float y, float vx, float vy, QColor c, float r);
    void fireSeeking(float x, float y, float vx, float vy, QColor c, float r);
    void fireNeedle(float x, float y, float ang, float spd, float grow, QColor c, float r);
    void fireBoomerang(float x, float y, float ang, float spd, QColor c, float r);

public:
    Boss();
    void reset(float startX, float startY);
    void update(float px, float py, int frameCnt);
    void draw(QPainter& p) const;

    void setBulletAdder(std::function<void(const Bullet&)> adder);  // 设置子弹回调函数
    Enemy& data() { return m_data; }
    const Enemy& data() const { return m_data; }    //只读返回boss数据
    bool isAlive() const { return m_data.alive; }
};

#endif