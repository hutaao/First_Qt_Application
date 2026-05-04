#include "Boss.h"
#include <QtMath>

Boss::Boss() {}

void Boss::reset(float startX, float startY) {
	m_data = Enemy{};   // 清空数据
	m_data.x = startX; m_data.y = startY;   // 初始位置,后续调整
    m_data.type = 2;
    m_data.maxHp = 150;
    m_data.hp = m_data.maxHp;
	m_data.moveAmp = 100;   // 横向移动幅度,后续调整
	m_data.shootInterval = 25;  // 射击间隔,后续调整
    m_data.shootTimer = 40; // 射击间隔,后续调整
    m_data.moveTimer = 0;
    m_data.baseX = startX;
    m_data.stage = 1;
    m_data.stgTimer = 0;    //阶段内计时器归零
    m_data.patIdx = 0;
    m_data.alive = true;
}

void Boss::setBulletAdder(std::function<void(const Bullet&)> adder) {
    m_addBullet = adder;
}

void Boss::update(float px, float py, int frameCnt) {
    if (!m_data.alive) return;
    m_data.moveTimer += 0.03f;
    m_data.x = m_data.baseX + sin(m_data.moveTimer) * m_data.moveAmp;
    m_data.y = 120 + sin(m_data.moveTimer * 0.7f) * 15;     //y向为更小的振幅
    m_data.x = qBound(30.0f, m_data.x, 610.0f);     //水平坐标限制

    if (--m_data.shootTimer <= 0) {
        m_data.shootTimer = m_data.shootInterval;
        switch (m_data.stage) {
        case 1: s1(px, py); break;
        case 2: s2(px, py, frameCnt); break;
        case 3: s3(px, py); break;
        }
    }
    m_data.angle += m_data.angleStep;   //螺旋弹
	m_data.stgTimer++;  //阶段内计时器每帧递增

    if (m_data.hp <= m_data.maxHp * 0.66f && m_data.stage < 2) {
        m_data.stage = 2; m_data.stgTimer = 0; m_data.patIdx = 0;
    }
    if (m_data.hp <= m_data.maxHp * 0.33f && m_data.stage < 3) {
        m_data.stage = 3; m_data.stgTimer = 0; m_data.patIdx = 0;
    }
}

void Boss::draw(QPainter& p) const {
    if (!m_data.alive) return;
    p.save();
    p.translate(m_data.x, m_data.y);
    p.setBrush(QColor(100, 50, 150));
    p.setPen(QPen(Qt::white, 2));
    p.drawEllipse(QPointF(0, 0), 40, 30);
    p.setBrush(QColor(70, 30, 120));
    p.drawEllipse(QPointF(-35, -5), 25, 15);
    p.drawEllipse(QPointF(35, -5), 25, 15);
    p.setBrush(Qt::red);
    p.drawEllipse(QPointF(-12, -10), 8, 10);
    p.drawEllipse(QPointF(12, -10), 8, 10);
    float hpPercent = (float)m_data.hp / m_data.maxHp;
    p.fillRect(-30, -45, 60 * hpPercent, 5, Qt::green);
    p.restore();
}

void Boss::s1(float px, float py) {
    fireFan(m_data.x, m_data.y, px, py, 8, 0.35f, 5.0f, Qt::yellow, 5);
    fireStraight(m_data.x, m_data.y, 0, 6, Qt::red, 5);
    fireStraight(m_data.x, m_data.y, -2, 6, Qt::red, 5);
	fireStraight(m_data.x, m_data.y, 2, 6, Qt::red, 5);// 3道平行直线弹
    if (m_data.stgTimer % 3 == 0)       //追踪弹发射间隔，后续调整
        fireSeeking(m_data.x, m_data.y, 0, 3, QColor(255, 100, 255), 5);
}

void Boss::s2(float px, float py, int frameCnt) {
    for (int i = 0; i < 4; ++i) {
		float a = m_data.stgTimer * 0.4f + i * M_PI / 2;    //4道海胆弹间隔90度，每帧旋转0.4弧度，后续调整
        fireNeedle(m_data.x, m_data.y, a, 3.0f, 0.03f, Qt::white, 4); //每帧膨胀0.03弧度，后续调整
    }
    if (frameCnt % 80 < 20)
		fireSeeking(m_data.x, m_data.y, 0, 3, QColor(255, 100, 255), 5);//追踪弹发射只占1/4时间，后续调整
}

void Boss::s3(float px, float py) {
    if (m_data.patIdx == 0) {
        fireCircle(m_data.x, m_data.y, 18, 2.5f, QColor(0, 180, 200), 5);   //圆形弹幕，速度2.5/帧
        m_data.patIdx = 1;
    }
    else if (m_data.patIdx == 1) {
		fireBoomerang(m_data.x, m_data.y, -M_PI / 4.0f, 5, QColor(255, 128, 0), 6);//-45度回旋镖弹，速度5/帧
		fireBoomerang(m_data.x, m_data.y, M_PI / 4.0f, 5, QColor(255, 128, 0), 6);//45度回旋镖弹，速度5/帧
        m_data.patIdx = 2;
    }
    else {
        float base = atan2(py - m_data.y, px - m_data.x);   //不对称直弹压制，右侧留空
        for (int i = -4; i <= 0; ++i)
            fireStraight(m_data.x, m_data.y, cos(base + i * 0.2f) * 5, sin(base + i * 0.2f) * 5, QColor(255, 165, 0), 4);
        m_data.patIdx = 0;
    }
}

// 辅助弹幕函数
void Boss::fireCircle(float x, float y, int cnt, float spd, QColor c, float r) //圆形弹幕，均分2PI发射cnt道
{
    for (int i = 0; i < cnt; ++i) {
        float a = i * (2 * M_PI / cnt);
        fireStraight(x, y, cos(a) * spd, sin(a) * spd, c, r);
    }
}
void Boss::fireFan(float x, float y, float tx, float ty, int cnt, float spread, float spd, QColor c, float r) { //在spread弧度范围内发射cnt发散射弹
    float base = atan2(ty - y, tx - x);
    for (int i = 0; i < cnt; ++i) {
        float a = base + (i - (cnt - 1) / 2.0f) * spread;
        fireStraight(x, y, cos(a) * spd, sin(a) * spd, c, r);
    }
}
void Boss::fireStraight(float x, float y, float vx, float vy, QColor c, float r) {
	if (!m_addBullet) return;   //没有设置回调函数则不发射，保护代码
    Bullet b;
    b.x = x; b.y = y; b.vx = vx; b.vy = vy;
    b.radius = r; 
    b.color = c;
    b.damage = 1;
    b.type = 1;
    b.alive = true;
    m_addBullet(b);
}
void Boss::fireSeeking(float x, float y, float vx, float vy, QColor c, float r) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y; b.vx = vx; b.vy = vy;
    b.radius = r; b.color = c; b.damage = 1;
	b.type = 2;
    b.turnSpeed = 0.025f; 
    b.trackDur = 90;
    b.alive = true;  //转向速度与追踪持续时间，后续调整
    m_addBullet(b);
}
void Boss::fireNeedle(float x, float y, float ang, float spd, float grow, QColor c, float r) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y;
    b.vx = cos(ang) * spd;
    b.vy = sin(ang) * spd;
    b.radius = r; 
    b.color = c;
    b.damage = 1;
    b.type = 3;
    b.growRate = grow; 
    b.alive = true;
    m_addBullet(b);
}
void Boss::fireBoomerang(float x, float y, float ang, float spd, QColor c, float r) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y;
    b.vx = cos(ang) * spd;
    b.vy = sin(ang) * spd;
    b.radius = r;
    b.color = c;
    b.damage = 1;
    b.type = 4;
	b.turnSpeed = 0.025f;   //转向速度，后续调整
    b.alive = true;
    m_addBullet(b);
}