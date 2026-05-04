#include "Player.h"
#include <QtMath>

Player::Player() {
    m_bomb.maxCD = 300;
    m_shield.maxCD = 600;
    m_fire.maxCD = 450;
}

void Player::reset(float startX, float startY) {
    m_x = startX;
    m_y = startY;
    m_hp = m_maxHp = 15;
    m_shootTimer = 0;
    m_invTimer = 0;
    m_dmg = 1;
    m_bomb.cd = m_bomb.active = 0;  //冷却与持续时间
    m_shield.cd = m_shield.active = 0;
    m_fire.cd = m_fire.active = 0;
}

void Player::setBulletAdder(std::function<void(const Bullet&)> adder) {     //子弹添加回调
    m_addBullet = adder;
}

void Player::update(const QSet<int>& keys, int w, int h, int frameCnt) {
    float dx = 0, dy = 0;
    if (keys.contains(Qt::Key_Left) || keys.contains(Qt::Key_A)) dx -= 1;
    if (keys.contains(Qt::Key_Right) || keys.contains(Qt::Key_D)) dx += 1;
    if (keys.contains(Qt::Key_Up) || keys.contains(Qt::Key_W)) dy -= 1;
    if (keys.contains(Qt::Key_Down) || keys.contains(Qt::Key_S)) dy += 1;
    if (dx != 0 || dy != 0) {       //各方向速度归一化
        float len = sqrt(dx * dx + dy * dy);
        dx /= len; dy /= len;
    }
    m_x += dx * m_speed;
    m_y += dy * m_speed;

    m_x = qBound(20.0f, m_x, (float)w - 20);
    m_y = qBound(20.0f, m_y, (float)h - 20);

    if (m_invTimer > 0) m_invTimer--;
    if (--m_shootTimer <= 0) m_shootTimer = 0;
}

void Player::tryShoot() {
    if (m_shootTimer <= 0) {
        m_shootTimer = m_shootInterval;
        Bullet b;
        b.color = Qt::yellow;
        b.radius = 3;
        b.damage = m_dmg; 
        b.alive = true; 
        b.type = 0;
        b.vx = 0; b.vy = -8;
        b.y = m_y - 20;
        b.x = m_x - 8; m_addBullet(b);      //子弹发射位置
        b.x = m_x;     m_addBullet(b);
        b.x = m_x + 8; m_addBullet(b);
    }
}

void Player::useSkill(int id) {}    //预留

void Player::updateSkills() {
    if (m_bomb.cd > 0)  m_bomb.cd--;
    if (m_shield.cd > 0)    m_shield.cd--;
    if (m_shield.active > 0) { 
        m_shield.active--;
        m_invTimer = 3;
    }
    if (m_fire.cd > 0)   m_fire.cd--;
    if (m_fire.active > 0) 
    {
        m_fire.active--;
        if (m_fire.active == 0) m_dmg = 1;
    }
}

void Player::hit() {}       //预留

void Player::getPickup(int type) {}       //预留

void Player::draw(QPainter& p) const {
    bool visible = (m_invTimer <= 0) || ((m_invTimer / 4) % 2 == 0);
    if (!visible) return;
    p.save();
    p.translate(m_x, m_y);
    p.setBrush(QColor(0, 150, 255));    //蓝色
    p.setPen(QPen(Qt::cyan, 1));
    p.drawPolygon(QPolygonF({ QPointF(0,-20), QPointF(-12,10), QPointF(0,5), QPointF(12,10) }));
    p.drawRect(-3, -5, 6, 15);
    p.drawRect(-18, 0, 6, 10);
    p.drawRect(12, 0, 6, 10);
    p.restore();
}