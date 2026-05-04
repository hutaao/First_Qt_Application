#include "GameWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent), m_rng(std::random_device{}()) {
    setFixedSize(640, 960);
    setFocusPolicy(Qt::StrongFocus);
    m_timer.start(16, this);
    m_player.setBulletAdder([this](const Bullet& b) { m_pBullets.append(b); });
    m_boss.setBulletAdder([this](const Bullet& b) { m_eBullets.append(b); });
    gameReset();
}

void GameWidget::gameReset() {
    m_player.reset(width() / 2.0f, height() - 100.0f);
    m_enemies.clear();
	m_boss = Boss();        //  重置Boss对象
    m_boss.setBulletAdder([this](const Bullet& b) { m_eBullets.append(b); });  //回调丢失重新设定
    m_pBullets.clear(); m_eBullets.clear(); m_pickups.clear(); m_pts.clear();
    m_wave = 0; m_score = 0; m_frame = 0; m_remain = 0; m_bossOn = false;   //无boss在场
    m_state = BREAK; m_breakTimer = m_breakLen;
}

void GameWidget::startWave() {
    if (m_wave == m_totalWaves) {               //Boss 波
        m_boss.reset(width() / 2.0f, 120);
        m_bossOn = true;
        m_remain = 0;   //不计算小怪击杀数
    }
    else {
        int cnt = 6 + m_wave * 2; m_remain = cnt;
        for (int i = 0; i < cnt; ++i) {
            EnemyUnit eu;
            float x = rf(80, width() - 80), y = rf(60, height() / 2 - 80);
            int typ = (ri(0, 9) < 7) ? 0 : 1;
            eu.reset(x, y, typ, m_wave);
            eu.setBulletAdder([this](const Bullet& b) { m_eBullets.append(b); });
            m_enemies.append(eu);
        }
        m_bossOn = false;
    }
}

void GameWidget::tick() {
    m_frame++;
    m_player.updateSkills();
    m_player.update(m_keys, width(), height(), m_frame);
    m_player.tryShoot();

    for (auto& e : m_enemies) {
        if (e.isAlive())
            e.update(m_player.x(), m_player.y(), m_frame);
    }
    if (m_bossOn && m_boss.isAlive()) m_boss.update(m_player.x(), m_player.y(), m_frame);   //更新boss状态
    
    for (auto& b : m_pBullets) {
        if (!b.alive) continue;
        b.update(m_player.x(), m_player.y());
        if (b.isOutOfScreen(width(), height())) { b.alive = false; continue; }
        for (auto& e : m_enemies) {
            if (!e.isAlive()) continue;
            if (dist(b.x, b.y, e.data().x, e.data().y) < b.radius + 18.0f) {
                b.alive = false;
                e.data().hp -= b.damage;
                if (e.data().hp <= 0) {
                    e.kill(); m_remain--; m_score += 150;
                    genParticles(e.data().x, e.data().y, 20, Qt::red);
                    genPickup(e.data().x, e.data().y);
                }
                break;
            }
        }
        if (b.alive && m_bossOn && m_boss.isAlive()) {
            if (dist(b.x, b.y, m_boss.data().x, m_boss.data().y) < b.radius + 35.0f) {  //boss碰撞半径设为35
                b.alive = false;
                m_boss.data().hp -= b.damage;
                if (m_boss.data().hp <= 0) {
                    m_boss.data().alive = false; 
                    m_bossOn = false; 
                    m_score += 1000;    //加分后续分段
                    genParticles(m_boss.data().x, m_boss.data().y, 40, Qt::red);    //40个爆炸粒子
					genPickup(m_boss.data().x, m_boss.data().y);        //占位，Boss死后掉落一个特殊道具
                }
            }
        }
    }

    for (auto& b : m_eBullets) {
        if (!b.alive) continue;
        b.update(m_player.x(), m_player.y());
        if (b.isOutOfScreen(width(), height())) { b.alive = false; continue; }
        if (!m_player.isInv() && dist(b.x, b.y, m_player.x(), m_player.y()) < b.radius + m_player.radius()) {
            b.alive = false;
            m_player.hit();
            genParticles(m_player.x(), m_player.y(), 10, Qt::cyan);
            if (m_player.hp() <= 0) { m_state = OVER; return; }
        }
    }

    for (auto& pk : m_pickups) {
        if (!pk.alive) continue;
        pk.life--; if (pk.life <= 0) { pk.alive = false; continue; }
        if (dist(m_player.x(), m_player.y(), pk.x, pk.y) < 22) {
            pk.alive = false;
            m_player.getPickup(pk.type);
        }
    }

    for (auto& p : m_pts) {
        if (p.alive) { p.x += p.vx; p.y += p.vy; p.life--; if (p.life <= 0) p.alive = false; }
    }

    m_pBullets.erase(std::remove_if(m_pBullets.begin(), m_pBullets.end(), [](const Bullet& b) { return !b.alive; }), m_pBullets.end());
    m_eBullets.erase(std::remove_if(m_eBullets.begin(), m_eBullets.end(), [](const Bullet& b) { return !b.alive; }), m_eBullets.end());
    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(), [](const EnemyUnit& e) { return !e.isAlive(); }), m_enemies.end());
    m_pickups.erase(std::remove_if(m_pickups.begin(), m_pickups.end(), [](const Pickup& p) { return !p.alive; }), m_pickups.end());
    m_pts.erase(std::remove_if(m_pts.begin(), m_pts.end(), [](const Particle& p) { return !p.alive; }), m_pts.end());

    checkWave();
}

void GameWidget::checkWave() {
    if (m_state != PLAY) return;
    if (m_wave == m_totalWaves) {           // boss波结束检查
        if (!m_bossOn && !m_boss.isAlive())  
            m_state = OVER; 
    }
    else {
        if (m_remain <= 0 && m_enemies.isEmpty()) {
            m_state = BREAK; m_breakTimer = m_breakLen;
        }
    }
}

void GameWidget::timerEvent(QTimerEvent*) {
    if (m_state == MENU || m_state == PAUSED || m_state == OVER) { update(); return; }
    if (m_state == BREAK) {
        m_breakTimer--;
        if (m_breakTimer <= 0) {
            m_wave++;
            startWave();
            m_state = (m_wave == m_totalWaves) ? BOSS_WARN : PLAY;  // 进Boss前预警
            if (m_state == BOSS_WARN) m_warnTimer = m_warnLen;
        }
        update(); return;
    }
    if (m_state == BOSS_WARN) {                           // Boss 预警倒计时
        m_warnTimer--;
        if (m_warnTimer <= 0) { m_state = PLAY; startWave(); }
        update(); return;
    }
    tick(); update();
}

void GameWidget::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    if (m_state == MENU && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) { gameReset(); return; }
    if (m_state == OVER && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) { gameReset(); return; }
    if (m_state == PLAY && event->key() == Qt::Key_Escape) { m_state = PAUSED; return; }
    if (m_state == PAUSED && event->key() == Qt::Key_Escape) { m_state = PLAY; return; }
    if (m_state == PLAY) {
        if (event->key() == Qt::Key_1) m_player.useSkill(1);
        if (event->key() == Qt::Key_2) m_player.useSkill(2);
        if (event->key() == Qt::Key_3) m_player.useSkill(3);
        m_keys.insert(event->key());
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    m_keys.remove(event->key());
}

void GameWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::black);

    if (m_state == MENU) {
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 20, QFont::Bold));
        QRect r(40, height() / 3, width() - 80, height() / 3);
        p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, "THUNDER ZONE\n\nPress ENTER to Start");
        return;
    }
    if (m_state == OVER) {
        p.fillRect(rect(), QColor(0, 0, 0, 220));
        p.setPen(Qt::red);
        p.setFont(QFont("Arial", 24, QFont::Bold));
        QRect r(40, height() / 3, width() - 80, height() / 3);
        p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, "GAME OVER\n\nPress ENTER to restart");
        return;
    }

    for (const auto& pk : m_pickups) {
        if (!pk.alive) continue;
        QColor c = (pk.type == 0) ? Qt::green : (pk.type == 1) ? Qt::cyan : QColor(255, 165, 0);
        p.setBrush(c); p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(pk.x, pk.y), 6, 6);
    }
    for (const auto& b : m_pBullets) {
        if (!b.alive) continue;
        p.setBrush(b.color); p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(b.x, b.y), b.radius, b.radius);
    }
    for (const auto& b : m_eBullets) {
        if (!b.alive) continue;
        p.setBrush(b.color); p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(b.x, b.y), b.radius, b.radius);
    }
    for (const auto& e : m_enemies) e.draw(p);

    if (m_bossOn && m_boss.isAlive()) m_boss.draw(p);       // 绘制boss
    m_player.draw(p);
    for (const auto& pt : m_pts) {
        if (!pt.alive) continue;
        float alpha = (float)pt.life / pt.maxLife;
        QColor c = pt.color; c.setAlphaF(alpha);
        p.setBrush(c); p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(pt.x, pt.y), pt.radius * alpha, pt.radius * alpha);
    }

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 11, QFont::Bold));
    p.drawText(10, 20, QString("Wave %1/%2  Score: %3  HP: %4").arg(m_wave).arg(m_totalWaves).arg(m_score).arg(m_player.hp()));

    if (m_state == PLAY && m_wave != m_totalWaves) {
        int total = m_remain + m_enemies.size();
        if (total > 0) {
            float prog = (float)(total - m_enemies.size()) / total;
            int bw = width() - 20;
            p.fillRect(10, 30, bw * prog, 8, QColor(0, 180, 255));
            p.setPen(Qt::gray); p.drawRect(10, 30, bw, 8);
        }
    }
    else if (m_bossOn && m_boss.isAlive()) {              // Boss血条
        float prog = 1.0f - (float)m_boss.data().hp / m_boss.data().maxHp;
        int bw = width() - 20;
        p.fillRect(10, 30, bw * prog, 8, QColor(200, 0, 0));
        p.setPen(Qt::gray); p.drawRect(10, 30, bw, 8);
    }

    if (m_state == BREAK) {
        p.fillRect(0, height() / 2 - 30, width(), 60, QColor(0, 0, 0, 150));
        p.setPen(Qt::white); p.setFont(QFont("Arial", 22, QFont::Bold));
        int sec = m_breakTimer / 60 + 1;
        p.drawText(rect(), Qt::AlignCenter, QString("Wave %1 incoming... %2").arg(m_wave + 1).arg(sec));
    }

	if (m_state == BOSS_WARN) {         // boss预警界面
        p.fillRect(0, height() / 2 - 50, width(), 100, QColor(200, 0, 0, 200));
        p.setPen(Qt::white); p.setFont(QFont("Arial", 28, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, "WARNING !!");
        p.setFont(QFont("Arial", 18));
        p.drawText(QRect(0, height() / 2, width(), 50), Qt::AlignCenter, "Boss Approaching...");
    }

    p.setFont(QFont("Arial", 9));
    auto drawSkill = [&](int x, const Skill& sk, const QString& name, int key) {
        p.fillRect(x, height() - 50, 60, 30, QColor(50, 50, 50, 200));
        p.setPen(Qt::white);
        if (sk.cd > 0) {
            float cdPer = 1.0f - (float)sk.cd / sk.maxCD;
            p.fillRect(x, height() - 50, 60 * cdPer, 30, QColor(0, 120, 0, 150));
            p.drawText(QRect(x, height() - 50, 60, 30), Qt::AlignCenter, QString("%1s").arg(sk.cd / 60 + 1));
        }
        else if (sk.active > 0) {
            p.fillRect(x, height() - 50, 60, 30, QColor(0, 200, 200, 150));
            p.drawText(QRect(x, height() - 50, 60, 30), Qt::AlignCenter, QString("%1s").arg(sk.active / 60 + 1));
        }
        else {
            p.drawText(QRect(x, height() - 50, 60, 30), Qt::AlignCenter, QString("[%1] %2").arg(key).arg(name));
        }
        };
    drawSkill(20, m_player.skillBomb(), "Bomb", 1);
    drawSkill(100, m_player.skillShield(), "Shield", 2);
    drawSkill(180, m_player.skillFire(), "Fire+", 3);

    if (m_state == PAUSED) {
        p.fillRect(rect(), QColor(0, 0, 0, 180));
        p.setPen(Qt::white); p.setFont(QFont("Arial", 22));
        p.drawText(QRect(40, height() / 3, width() - 80, height() / 3), Qt::AlignCenter, "PAUSED\n\nPress ESC to resume");
    }
}

void GameWidget::genPickup(float x, float y) {
    if (ri(0, 2) == 0) {
        Pickup pk; pk.x = x; pk.y = y; pk.type = ri(0, 2); pk.alive = true;
        m_pickups.append(pk);
    }
}
void GameWidget::genParticles(float x, float y, int cnt, QColor col) {
    for (int i = 0; i < cnt; ++i) {
        float ang = rf(0.0f, 2.0f * static_cast<float>(M_PI));
        float spd = rf(1.0f, 4.0f);
        Particle p;
        p.x = x; p.y = y;
        p.vx = cos(ang) * spd; p.vy = sin(ang) * spd;
        p.life = p.maxLife = 15 + ri(0, 10);
        p.radius = 2.0f + rf(1.0f, 3.0f);
        p.color = col; p.alive = true;
        m_pts.append(p);
    }
}
float GameWidget::rf(float min, float max) {
    std::uniform_real_distribution<float> d(min, max);
    return d(m_rng);
}
int GameWidget::ri(int min, int max) {
    std::uniform_int_distribution<int> d(min, max);
    return d(m_rng);
}
float GameWidget::dist(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}