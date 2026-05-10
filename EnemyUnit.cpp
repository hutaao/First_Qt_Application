#include "EnemyUnit.h"
#include <QtMath>
#include <cmath>

EnemyUnit::EnemyUnit() {
    m_pxNorm.load(":/res/enemy_normal.png");
    m_pxEli[0].load(":/res/enemy_elite1.png");
    m_pxEli[1].load(":/res/enemy_elite2.png");
    m_pxEli[1] = m_pxEli[1].scaled(m_pxEli[0].size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_radius = qMax(qMax(contentRadius(m_pxNorm), contentRadius(m_pxEli[0])), contentRadius(m_pxEli[1]));
}

void EnemyUnit::reset(float x, float y, int type, int wave) {
    m_data = Enemy{};   //用于缓冲防忘赋值
    m_data.x = x; m_data.y = y;
    m_data.type = type;
    m_data.baseX = x;
    m_data.baseY = y;
    m_data.floatPhase = (float)(rand() % 628) / 100.0f;
    m_data.moveTimer = (float)(rand() % 100) * 0.03f;
    m_data.randMoveTimer = 90 + rand() % 180;
    m_data.randVX = 0; m_data.randVY = 0;
    if (type == 1) m_ei = rand() % 2;
    m_data.shootInterval = qMax(25, 45 - wave * 6);
    m_data.shootTimer = 10 + rand() % m_data.shootInterval; //防止敌人初始同时开火
    m_data.attackPhase = 0;
    m_data.phaseTimer = 60;   // 初始攻击1秒
    m_data.alive = true;
    if (type == 0) {        //血量赋值
        m_data.maxHp = m_data.hp = 90 + 6*wave;
    }
    else {
        m_data.maxHp = m_data.hp = 120 + wave * 8;
    }   
}

void EnemyUnit::setBulletAdder(std::function<void(const Bullet&)> adder) {
    m_addBullet = adder;    //lambda表达式
}

void EnemyUnit::update(float px, float py, int frameCnt) {
    if (!m_data.alive) return;
    m_data.moveTimer += 0.03f;  //浮动摆动计时
    m_data.x = m_data.baseX;    // X固定在刷新位置，不再做正弦运动
    m_data.y = m_data.baseY + sin(m_data.moveTimer + m_data.floatPhase) * 2.0f;
    m_data.randMoveTimer--;
    if (m_data.randMoveTimer <= 0) {
        m_data.randMoveTimer = 90 + rand() % 180;
        m_data.randVX = ((rand() % 1001) / 1000.0f - 0.5f) * 1.2f;
        m_data.randVY = ((rand() % 1001) / 1000.0f - 0.5f) * 1.2f;
    }
    m_data.baseX += m_data.randVX;
    m_data.baseY += m_data.randVY;
    m_data.baseX = qBound(50.0f, m_data.baseX, 910.0f);
    m_data.baseY = qBound(60.0f, m_data.baseY, 320.0f);
    m_data.x = m_data.baseX;
    m_data.x = qBound(m_radius, m_data.x, 960.0f - m_radius);

    if (m_data.phaseTimer > 0) {
        m_data.phaseTimer--;
    }
    else {
        m_data.attackPhase = 1 - m_data.attackPhase;  // 切换阶段
        m_data.phaseTimer = 60;  // 每阶段1秒（60帧）
    }

    if (m_data.attackPhase == 0) {
        if (--m_data.shootTimer <= 0) {
            m_data.shootTimer = m_data.shootInterval;
            shoot(px, py, frameCnt);
        }
    }
    m_data.angle += m_data.angleStep;   //螺旋弹幕自增
}

void EnemyUnit::shoot(float px, float py, int frameCnt) {
    if (m_data.type == 0) {
        int pat = rand() % 3;  // 普通：每次随机抽一种弹幕
        switch (pat) {
        case 0:                     //直线弹幕
            for (int i = 0; i < 4; ++i) {
                float vx = (i - 1.5f) * 0.8f;
                fireStraight(m_data.x, m_data.y, vx, 4.5f, Qt::red, 5);
            }
            break;
        case 1: {                   //扇形散射
            float base = static_cast<float>(M_PI) / 2.0f;
            for (int i = 0; i < 4; ++i) {
                float a = base + (i - 1.5f) * 0.25f;
                fireStraight(m_data.x, m_data.y, cos(a) * 4.0f, sin(a) * 4.0f, QColor(255, 120, 80), 5);
            }
            break;
        }
        case 2:                     //追踪弹x2
            fireSeeking(m_data.x - 10, m_data.y, 0, 2.5f, QColor(255, 80, 200), 5);
            fireSeeking(m_data.x + 10, m_data.y, 0, 2.5f, QColor(255, 80, 200), 5);
            break;
        }
    }
    else {
        int pat = rand() % 4;  // 每次随机抽一种弹幕
        switch (pat) {
        case 0:                     //旋转螺旋弹，4道旋转发射
            for (int i = 0; i < 4; ++i) {
                float a = m_data.angle + i * M_PI / 6;
                fireStraight(m_data.x, m_data.y, cos(a) * 3.5f, sin(a) * 3.5f, QColor(200, 200, 0), 5);
            }
            break;
        case 1:
            for (int i = 0; i < 4; ++i) {
                float a = m_data.angle + i * M_PI / 2;
                fireStraight(m_data.x, m_data.y, cos(a) * 3.0f, sin(a) * 3.0f, QColor(255, 180, 0), 5);
                fireStraight(m_data.x, m_data.y, cos(-a) * 3.0f, sin(-a) * 3.0f, QColor(0, 200, 255), 5);
            }
            break;
        case 2:                     //散射+追踪
            for (int i = 0; i < 3; ++i) {
                float vx = (i - 1) * 1.0f;
                fireStraight(m_data.x, m_data.y, vx, 4.0f, QColor(255, 200, 50), 5);
            }
            fireSeeking(m_data.x, m_data.y, 0, 3.0f, QColor(255, 80, 200), 5);
            break;
        case 3:                     //圆形12道全方向弹幕
            for (int i = 0; i < 12; ++i) {
                float a = i * 2.0f * M_PI / 12;
                fireStraight(m_data.x, m_data.y, cos(a) * 3.5f, sin(a) * 3.5f, QColor(180, 100, 255), 5);
            }
            break;
        }
    }
}

void EnemyUnit::fireStraight(float x, float y, float vx, float vy, QColor color, float radius) {
    if (!m_addBullet) return; //防止调用空容器
    Bullet b;
    b.x = x; b.y = y;
    b.vx = vx * 1.2f; b.vy = vy * 1.2f;
    b.radius = radius;
    b.color = color;
    b.damage = 1;
    b.type = 1;
    b.alive = true;
    m_addBullet(b);
}

void EnemyUnit::fireSeeking(float x, float y, float vx, float vy, QColor color, float radius) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y;
    b.vx = vx * 1.6f; b.vy = vy * 1.6f;
    b.radius = radius;
    b.color = color;
    b.damage = 1;
    b.type = 2;              //追踪弹类型
    b.turnSpeed = 0.02f;     //转向速度
    b.trackDur = 120;        //追踪持续2秒
    b.alive = true;
    m_addBullet(b);
}

void EnemyUnit::draw(QPainter& p) const {
    if (!m_data.alive) return;
    const QPixmap* px = (m_data.type == 0) ? &m_pxNorm : &m_pxEli[m_ei];    //普通用 m_pxNorm，精英随机用 m_pxEli[0] 或 [1]
    if (px->isNull()) return;   // 图片没加载好就不画
    p.save();
    p.translate(m_data.x, m_data.y);
    p.drawPixmap(-px->width() / 2, -px->height() / 2 + 5, *px); 

    float hpPercent = (float)m_data.hp / m_data.maxHp;  // 血条：精英血量更厚
    if (m_data.type == 0) {
        p.fillRect(-15, -px->height() / 2 - 10, 30 * hpPercent, 4, Qt::green);
    } else {
        p.fillRect(-20, -px->height() / 2 - 10, 40 * hpPercent, 4, QColor(255, 140, 0));  // 精英橙色血条
    }
    p.restore();
}
