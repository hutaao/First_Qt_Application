#include "EnemyUnit.h"
#include <QtMath>

EnemyUnit::EnemyUnit() {}

void EnemyUnit::reset(float x, float y, int type, int wave) {
    m_data = Enemy{};   //用于缓冲防忘赋值
    m_data.x = x; m_data.y = y;
    m_data.type = type;
    m_data.baseX = x;
    if (type == 0) {
        m_data.moveAmp = 0.0f;
    }
    else if (type == 1) {
        m_data.moveAmp = 80.0f;
    }
    m_data.shootInterval = qMax(20, 45 - wave * 4);
    m_data.shootTimer = 10 + rand() % m_data.shootInterval; //防止敌人初始同时开火
    m_data.alive = true;
    if (type == 0) {        //血量赋值，后续调整
        m_data.maxHp = m_data.hp = 4 + wave;
    }
    else {
        m_data.maxHp = m_data.hp = 6 + wave * 2;
    }   
}

void EnemyUnit::setBulletAdder(std::function<void(const Bullet&)> adder) {
    m_addBullet = adder;    //lambda表达式
}

void EnemyUnit::update(float px, float py, int frameCnt) {
    if (!m_data.alive) return;
    m_data.moveTimer += 0.03f;  //浮动摆动计时
    if (m_data.type == 0) {
        m_data.x = m_data.baseX;    
        m_data.y += sin(m_data.moveTimer) * 0.3f;
    }
    else {
        m_data.x = m_data.baseX + sin(m_data.moveTimer) * m_data.moveAmp;
    }
    m_data.x = qBound(30.0f, m_data.x, 610.0f); //水平位置限制

    if (--m_data.shootTimer <= 0) {
        m_data.shootTimer = m_data.shootInterval;
        shoot(px, py, frameCnt);
    }
    m_data.angle += m_data.angleStep;   //螺旋弹幕自增
}

void EnemyUnit::shoot(float px, float py, int frameCnt) {
    if (m_data.type == 0) {     //普通敌人散弹
        for (int i = 0; i < 3; ++i) {
            float vx = (i - 1) * 0.5f;
            fireStraight(m_data.x, m_data.y, vx, 4.0f, Qt::red, 5);
        }
    }
    else {
        for (int i = 0; i < 6; ++i) {       //精英螺旋弹
            float a = m_data.angle + i * 2.0f * M_PI / 6;
            fireStraight(m_data.x, m_data.y, cos(a) * 3.5f, sin(a) * 3.5f, QColor(200, 200, 0), 5);
        }
    }
}

void EnemyUnit::fireStraight(float x, float y, float vx, float vy, QColor color, float radius) {
    if (!m_addBullet) return; //防止调用空容器
    Bullet b;
    b.x = x; b.y = y;
    b.vx = vx; b.vy = vy;
    b.radius = radius;
    b.color = color;
    b.damage = 1;
    b.type = 1;
    b.alive = true;
    m_addBullet(b);
}

void EnemyUnit::draw(QPainter& p) const {
    if (!m_data.alive) return;
    p.save();
    p.translate(m_data.x, m_data.y);
    if (m_data.type == 0) {
        p.setBrush(Qt::red);
        p.drawEllipse(QPointF(0, 0), 18, 18);
    }
    else {
        p.setBrush(QColor(255, 140, 0));
        p.drawEllipse(QPointF(0, 0), 20, 20);
        p.drawRect(-5, -25, 10, 10);
    }
    p.setPen(QPen(Qt::white, 1));
    p.drawEllipse(QPointF(0, 0), 18, 18);
    float hpPercent = (float)m_data.hp / m_data.maxHp;
    p.fillRect(-15, -28, 30 * hpPercent, 4, Qt::green);
    p.restore();
}