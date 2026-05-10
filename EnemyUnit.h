#ifndef ENEMYUNIT_H
#define ENEMYUNIT_H

#include "GameTypes.h"
#include <functional>
#include <QPainter>

class EnemyUnit {
    Enemy m_data;
    std::function<void(const Bullet&)> m_addBullet;

    void shoot(float px, float py, int frameCnt);   //弹幕由type决定
    void fireStraight(float x, float y, float vx, float vy, QColor color, float radius);    //直线弹声明
    void fireSeeking(float x, float y, float vx, float vy, QColor color, float radius);     //追踪弹声明
    QPixmap m_pxNorm;
    QPixmap m_pxEli[2];
    int m_ei = 0;
    float m_radius = 18;
    int m_pattern = -1;  // 弹幕模式：-1=随机，>=0=固定模式
public:
    EnemyUnit();
    void reset(float x, float y, int type, int wave);   
    void update(float px, float py, int frameCnt);      
    void draw(QPainter& p) const;

    void setBulletAdder(std::function<void(const Bullet&)> adder);  
    Enemy& data() { return m_data; }        //  返回敌人内部数据
    const Enemy& data() const { return m_data; }
    bool isAlive() const { return m_data.alive; }
    float radius() const { return m_radius; }
    void setPattern(int p) { m_pattern = p; }  // 设置固定弹幕模式
    void kill() { m_data.alive = false; }

};

#endif
