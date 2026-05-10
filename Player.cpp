#include "Player.h"
#include <QtMath>
#include <cmath>

Player::Player() {
    m_pixmap.load(":/res/player.png");
    m_pixmap = m_pixmap.scaled(m_pixmap.size() * 0.75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_radius = contentRadius(m_pixmap) * 0.45f;
    m_bomb.maxCD = 180;     // 炸弹3秒冷却
}

void Player::reset(float startX, float startY) {
    m_x = startX;
    m_y = startY;
    m_hp = m_maxHp = 30;
    m_shootTimer = 0;
    m_invTimer = 0;
    m_dmg = 2;
    m_bomb.cd = m_bomb.active = 0;      //重置冷却时间
    m_bombStock = 0;  // 重置炸弹数量
    m_shieldTimer = 0;  // 重置护盾计时
    m_hits = 0;          // 重置受击计数
    m_pickupsCollected = 0;  // 重置道具拾取计数
    m_fire.active = 0;
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

    m_x = qBound(m_radius, m_x, (float)w - m_radius);
    m_y = qBound(m_radius, m_y, (float)h - m_radius);

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
        b.vx = 0; b.vy = -15;
        b.y = m_y - m_pixmap.height() / 2;

        if (m_fire.active > 0) {
            b.x = m_x - 21; m_addBullet(b);
            b.x = m_x - 7;  m_addBullet(b);
            b.x = m_x + 7;  m_addBullet(b);
            b.x = m_x + 21; m_addBullet(b);
        } else {
            b.x = m_x - 14; m_addBullet(b);
            b.x = m_x;     m_addBullet(b);
            b.x = m_x + 14; m_addBullet(b);
        }

        b.type = 5;
        float sideOffset = 45.0f;
        float spd = 15.0f;
        if (m_fire.active > 0) {
            float degsFull[] = {4.0f, 8.0f, 12.0f, 16.0f};
            for (int i = 0; i < 4; i++) {
                float rad = qDegreesToRadians(degsFull[i]);
                b.x = m_x + sideOffset; b.vx =  spd * sin(rad); b.vy = -spd * cos(rad); m_addBullet(b);
                b.x = m_x - sideOffset; b.vx = -spd * sin(rad); b.vy = -spd * cos(rad); m_addBullet(b);
            }
        } else {
            float degs[] = {6.0f, 14.0f};
            for (int i = 0; i < 2; i++) {
                float rad = qDegreesToRadians(degs[i]);
                b.x = m_x + sideOffset; b.vx =  spd * sin(rad); b.vy = -spd * cos(rad); m_addBullet(b);
                b.x = m_x - sideOffset; b.vx = -spd * sin(rad); b.vy = -spd * cos(rad); m_addBullet(b);
            }
        }
    }
}

void Player::useSkill(int id) {
    if (id == 1 && m_bombStock > 0 && m_bomb.cd <= 0) {
        m_bombStock--;
        m_bomb.cd = m_bomb.maxCD;
        m_bomb.active = 1;  // 标记为已激活，GameWidget处理清屏
    }
}

void Player::updateSkills() {
    if (m_bomb.cd > 0) m_bomb.cd--;
    if (m_shieldTimer > 0) {
        m_shieldTimer--;
        if (m_shieldTimer == 0) m_invTimer = 0;  // 护盾结束，无敌状态取消
    }
    if (m_fire.active > 0) {
        m_fire.active--;
        if (m_fire.active == 0) m_dmg = 2;
    }
}

void Player::hit() {            
    if (m_invTimer > 0) return;
    m_hp--;
    m_hits++;                // 受击计数+1
    m_invTimer = 60;  // 受伤后短暂无敌1秒
}

void Player::getPickup(int type) {
    m_pickupsCollected++;  // 道具拾取计数+1
    switch (type) {
    case 0: m_bombStock++;  // 拾取炸弹道具，存储一枚
        break;
    case 1: m_shieldTimer = 180; m_invTimer = 180;  // 护盾即时生效：3秒无敌
        break;
    case 2: m_fire.active = 420; m_dmg = 3;  // 火力增强：基础×1.5
        break;
    }
}

void Player::draw(QPainter& p) const {
    bool visible = (m_invTimer <= 0) || ((m_invTimer / 4) % 2 == 0);  // 无敌时闪烁效果
    if (!visible) return;
    if (m_pixmap.isNull()) return;

    p.save();
    p.translate(m_x, m_y);

    if (m_shieldTimer > 0) {            // 护盾可视化：青蓝色光环
        int alpha = 70 + 45 * sin(m_shieldTimer * 0.12f);
        QPen shieldPen(QColor(0, 210, 255, alpha), 3);
        p.setPen(shieldPen);
        p.setBrush(QColor(0, 150, 255, 25));
        float sr = m_radius * 1.2f;
        p.drawEllipse(QPointF(0, 0), sr, sr);
    }

    p.drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);
    p.restore();
}

void Player::flyOff() {
    m_y -= 6.1f;
}

bool Player::offScreen() const {
    return m_y < -m_pixmap.height();
}
