#include "Boss.h"
#include <QtMath>

Boss::Boss() {
    m_px1.load(":/res/boss_1.png");
    m_px2.load(":/res/boss_2.png");
    m_px1 = m_px1.scaled(m_px1.size() * 1.35, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_px2 = m_px2.scaled(m_px2.size() * 1.35, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_radius = qMax(contentRadius(m_px1), contentRadius(m_px2));  // Alpha扫描缩放后的图片，碰撞半径自动匹配
}

void Boss::reset(float startX, float startY) {
    m_data = Enemy{};   // 清空数据
    m_data.x = startX; m_data.y = startY;   
    m_data.type = 2;
    m_data.maxHp = 10000;
    m_data.hp = m_data.maxHp;
    m_data.shootInterval = 30;   // 攻击间隔1秒，避免弹幕重叠杂乱
    m_data.shootTimer = 50;      // 初始延迟
    m_data.moveTimer = 0;
    m_data.stage = 1;
    m_data.stgTimer = 0;
    m_data.patIdx = 0;
    m_data.alive = true;
    m_changing = false;
    m_changeTimer = 0;
    m_moveState = 1;             // 初始等待状态
    m_moveDist = 0;
    m_moveDir = (rand() % 2) ? 1 : -1;
    m_waitTimer = 55;            // 初始等待55帧，确保首次射击在等待态完成
    m_dying = false;
    m_dyingTimer = 0;
    m_urchinSeq = -1;
    m_urchinDelay = 0;
    m_urchinRounds = 0;
    m_trailOn = false;
    m_trailTimer = 0;
    m_trailFireTimer = 0;
    m_dustOn = false;
    m_dustTimer = 0;
    m_dustWaves = 0;
    m_urchinSweep = false;
    m_urchinSweepAng = 0;
    m_urchinSweepDir = 1;
    m_urchinSweepCnt = 0;
    m_urchinSweepTimer = 0;
    m_nextDust = 0;
    m_arrayRotAngle = 0;
    m_netOn = false;
    m_netTimer = 0;
    m_netFireTimer = 0;
}

void Boss::setBulletAdder(std::function<void(const Bullet&)> adder) {
    m_addBullet = adder;
}

void Boss::setPhaseDropCb(std::function<void(float, float)> cb) {
    m_onPhaseDrop = cb;
}

void Boss::startDying() {
    m_dying = true;
    m_dyingTimer = 60;
}

bool Boss::isDying() const { return m_dying; }
bool Boss::isDyingDone() const { return m_dying && m_dyingTimer <= 0; }

void Boss::update(float px, float py, int frameCnt) {
    if (!m_data.alive) return;
    if (m_dying) {
        m_dyingTimer--;
        if (m_dyingTimer <= 0) m_data.alive = false;
        return;
    }

    m_data.moveTimer += 0.03f;
    m_data.y = 220 + sin(m_data.moveTimer * 0.7f) * 15;

    if (m_urchinSweep || m_netOn) {
    } else {
    float moveSpeed = (m_data.stage == 3) ? 3.0f : 2.0f;  // 阶段3加速
    if (m_moveState == 0) {  // 移动中
        m_data.x += m_moveDir * moveSpeed;
        m_moveDist -= moveSpeed;
        if (m_data.x <= m_radius + 50)  m_moveDir = 1;
        if (m_data.x >= 960.0f - m_radius - 50) m_moveDir = -1;
        if (m_moveDist <= 0) {
            m_moveState = 1;
            m_waitTimer = (m_data.stage == 1) ? 18 : (m_data.stage == 3) ? 20 : 30;
        }
    } else {  // 等待中
        m_waitTimer--;
        if (m_waitTimer <= 0) {
            m_moveDist = (m_data.stage == 3) ? (45.0f + rand() % 86) : (70.0f + rand() % 111);
            if (m_data.x < 400)       m_moveDir = (rand() % 10 < 7) ? 1 : -1;
            else if (m_data.x > 560)  m_moveDir = (rand() % 10 < 7) ? -1 : 1;
            else                       m_moveDir = (rand() % 2) ? 1 : -1;
            m_moveState = 0;
        }
    }
    if (m_trailOn && m_moveState == 1) m_waitTimer = qMin(m_waitTimer, 12);
    m_data.x = qBound(m_radius, m_data.x, 960.0f - m_radius);     //水平坐标限制
    }

    if (m_trailOn) {
        m_trailFireTimer--;
        if (m_trailFireTimer <= 0) {
            m_trailFireTimer = 5;
            float tx = m_data.x;
            float ty = m_data.y + 15 + m_px1.height() / 2.0f;
            float degs[] = {5.0f, 22.0f, 39.0f};
            float spd = 8.1f;
            for (int i = 0; i < 3; i++) {
                float rad = qDegreesToRadians(degs[i]);
                fireTrail(tx, ty,  spd * sin(rad), spd * cos(rad));
                fireTrail(tx, ty, -spd * sin(rad), spd * cos(rad));
            }
        }
        m_trailTimer--;
        if (m_trailTimer <= 0) {
            m_trailOn = false;
            m_data.shootTimer = m_data.shootInterval;
            int mod = (m_data.stage == 1) ? 8 : 6;
            m_data.patIdx = (m_data.patIdx + 1) % mod;
        }
    } else if (m_dustOn) {
        m_dustTimer--;
        if (m_dustTimer % 30 == 0 && m_dustWaves > 0) {
            fireDust();
            m_dustWaves--;
        }
        if (m_dustWaves <= 0) {
            m_dustOn = false;
            m_data.shootTimer = m_data.shootInterval;
            int mod = (m_data.stage == 1) ? 8 : (m_data.stage == 3) ? 7 : 6;
            m_data.patIdx = (m_data.patIdx + 1) % mod;
        }
    } else if (m_urchinSeq >= 0) {
        if (m_urchinDelay <= 0) {
            fireUrchinRow(m_urchinSeq);
            m_urchinSeq--;
            if (m_urchinSeq >= 0) {
                m_urchinDelay = 8;
            } else {
                m_urchinRounds++;
                if (m_urchinRounds < 3) {
                    m_urchinSeq = -2;
                    m_urchinDelay = 45;
                } else {
                    m_data.shootTimer = m_data.shootInterval;
                    int mod = (m_data.stage == 1) ? 8 : (m_data.stage == 3) ? 7 : 6;
                    m_data.patIdx = (m_data.patIdx + 1) % mod;
                    m_urchinRounds = 0;
                }
            }
        } else {
            m_urchinDelay--;
        }
    } else if (m_urchinSeq == -2) {
        m_urchinDelay--;
        if (m_urchinDelay <= 0) {
            m_urchinSeq = 4;
            m_urchinDelay = 0;
            if (m_urchinSweep) {
                m_arrayRotAngle = ((rand() % 4001) / 1000.0f - 2.0f) * 10.0f * (float)M_PI / 180.0f;
            }
        }
    } else if (m_netOn) {
        m_netFireTimer--;
        if (m_netFireTimer <= 0) {
            m_netFireTimer = 1;
            fireNetRows();
        }
        m_netTimer--;
        if (m_netTimer <= 0) {
            m_netOn = false;
            m_data.shootTimer = m_data.shootInterval;
            m_data.patIdx = (m_data.patIdx + 1) % 7;
        }
    } else if (--m_data.shootTimer <= 0) {
        if (m_data.stage == 1 && m_moveState == 0) {
            m_data.shootTimer = 0;  // 移动中阻滞，保持待发状态，等待结束后立即射击
        } else {
            m_data.shootTimer = (m_data.stage == 3) ? 48 : (m_data.stage == 2) ? 60 : 30;
            switch (m_data.stage) {
            case 1: s1(px, py); break;
            case 2: s2(px, py, frameCnt); break;
            case 3: s3(px, py); break;
            }
        }
    }
    if (m_urchinSweep) {
        m_urchinSweepTimer--;
        if (m_urchinSweepTimer <= 0) {
            m_urchinSweepTimer = 6;
            float rad = qDegreesToRadians(m_urchinSweepAng);
            float spd = 5.0f + 4.0f * sin((m_urchinSweepAng - 30.0f) * (float)M_PI / 120.0f);
            float lx = m_data.x;
            float ly = m_data.y + 15 + m_px1.height() / 2.0f;
            fireUrchin(lx, ly, spd * cos(rad), spd * sin(rad));
            m_urchinSweepCnt--;
            m_urchinSweepAng += m_urchinSweepDir * 12.0f;
            if (m_urchinSweepAng > 150.0f) { m_urchinSweepAng = 138.0f; m_urchinSweepDir = -1; }
            if (m_urchinSweepAng < 30.0f)  { m_urchinSweepAng = 42.0f;  m_urchinSweepDir = 1; }
            if (m_urchinSweepCnt <= 0) { m_urchinSweep = false; }
        }
    }
    m_data.stgTimer++;  //阶段内计时器每帧递增

    if (m_data.hp <= m_data.maxHp * 0.66f && m_data.stage == 1 && !m_changing) {
        m_changing = true; m_changeTimer = 90;
    }
    if (m_data.hp <= m_data.maxHp * 0.33f && m_data.stage == 2 && !m_changing) {
        m_changing = true; m_changeTimer = 90;
    }

    if (m_changing) {                                  
        m_changeTimer--;
        m_data.shootTimer = m_data.shootInterval;  // 重置射击计时器
        if (m_changeTimer <= 0) {
            m_changing = false;
            m_data.stage++;           // 进入下一阶段
            m_data.stgTimer = 0;
            m_data.patIdx = 0;
            m_moveState = 1;          // 阶段切换后从等待开始
            m_waitTimer = (m_data.stage == 3) ? 20 : 18;
            if (m_onPhaseDrop) m_onPhaseDrop(m_data.x, m_data.y);  // 掉落道具
        }
        return;   // 切换期间不移动、不射击、不累加 angle/stgTimer
    }
}

void Boss::draw(QPainter& p) const {
    if (!m_data.alive) return;

    if (m_dying && (m_dyingTimer / 6) % 2 == 0) return;  // 死亡闪烁

    const QPixmap* px = (m_data.stage == 1) ? &m_px1 : &m_px2;
    if (px->isNull()) return;

    if (m_changing && (m_changeTimer / 4) % 2 == 0) return;  // 切换时闪烁

    p.save();
    p.translate(m_data.x, m_data.y);

    // 绘制boss名称
    p.setPen(QColor(255, 220, 50));
    p.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    QRect nameRect(-80, -px->height() / 2 - 50, 180, 30);
    p.drawText(nameRect, Qt::AlignCenter, QStringLiteral("太空原种"));

    p.drawPixmap(-px->width() / 2, -px->height() / 2 + 15, *px);
    p.restore();
}

void Boss::s1(float px, float py) {
    switch (m_data.patIdx) {
    case 0:  // 扇形扩散弹
        fireFan(m_data.x, m_data.y, px, py, 12, 0.22f, 4.5f, Qt::yellow, 5);
        break;
    case 1:  // 3道直线压制 + 2颗追踪弹
        fireStraight(m_data.x, m_data.y, -1.5f, 5.5f, Qt::red, 5);
        fireStraight(m_data.x, m_data.y, 0, 6.0f, Qt::red, 5);
        fireStraight(m_data.x, m_data.y, 1.5f, 5.5f, Qt::red, 5);
        fireSeeking(m_data.x - 20, m_data.y, 0, 3.0f, QColor(255, 100, 255), 5);
        fireSeeking(m_data.x + 20, m_data.y, 0, 3.0f, QColor(255, 100, 255), 5);
        break;
    case 2:  // 圆环弹幕
        fireCircle(m_data.x, m_data.y, 24, 2.8f, QColor(0, 200, 220), 4);
        break;
    case 3:  // 交替扇形
        fireFan(m_data.x - 30, m_data.y, px, py, 7, 0.25f, 4.0f, QColor(255, 160, 60), 5);
        fireFan(m_data.x + 30, m_data.y, px, py, 7, 0.25f, 4.0f, QColor(255, 160, 60), 5);
        break;
    case 4:  // 海胆阵列
        m_urchinSeq = 4;
        m_urchinDelay = 0;
        return;
    case 5:  // 尾弹持续扫射
        m_trailOn = true;
        m_trailTimer = 240;
        m_trailFireTimer = 0;
        return;
    case 6:  // 旋涡
        fireVortex();
        break;
    case 7:  // 弹幕牢笼
        fireCage();
        break;
    }
    m_data.patIdx = (m_data.patIdx + 1) % 8;
}

void Boss::s2(float px, float py, int frameCnt) {
    switch (m_data.patIdx) {
    case 0:  // 模式0：扇形扩散
        fireFan(m_data.x, m_data.y, px, py, 10, 0.2f, 4.5f, QColor(255, 200, 80), 5);
        break;
    case 1:  // 模式1：交叉扇形散射 + 直线压制
        fireFan(m_data.x, m_data.y, px, py, 8, 0.2f, 4.5f, QColor(255, 180, 60), 5);
        fireStraight(m_data.x - 20, m_data.y, -1.2f, 5.0f, QColor(255, 100, 30), 5);
        fireStraight(m_data.x + 20, m_data.y, 1.2f, 5.0f, QColor(255, 100, 30), 5);
        break;
    case 2:  // 模式2：双层圆环弹幕（内外圈速度差）
        fireCircle(m_data.x, m_data.y, 20, 3.5f, QColor(0, 160, 200), 4);
        fireCircle(m_data.x, m_data.y, 16, 2.2f, QColor(180, 100, 255), 4);
        break;
    case 3:  // 模式3：追踪弹幕集群 + 直线压制
        fireSeeking(m_data.x, m_data.y, 0, 2.8f, QColor(255, 100, 255), 5);
        fireSeeking(m_data.x - 15, m_data.y, -0.5f, 2.5f, QColor(255, 100, 255), 5);
        fireSeeking(m_data.x + 15, m_data.y, 0.5f, 2.5f, QColor(255, 100, 255), 5);
        fireSeeking(m_data.x, m_data.y, 0, 3.2f, QColor(255, 150, 100), 5);
        break;
    case 4:  // 模式4：粉尘弹幕（每4次触发1次）
        if (m_nextDust > 0) {
            m_nextDust--;
            fireFan(m_data.x, m_data.y, px, py, 8, 0.18f, 4.0f, QColor(200, 180, 140), 5);
            break;
        }
        m_nextDust = 4;
        m_dustOn = true;
        m_dustTimer = 240;
        m_dustWaves = 8;
        return;
    case 5:  // 模式5：海胆阵列+角度扫射组合
        m_urchinSeq = 4;
        m_urchinDelay = 0;
        m_urchinRounds = 0;
        m_urchinSweep = true;
        m_urchinSweepAng = 30.0f;
        m_urchinSweepDir = 1;
        m_urchinSweepCnt = 21;
        m_urchinSweepTimer = 0;
        m_arrayRotAngle = ((rand() % 4001) / 1000.0f - 2.0f) * 10.0f * (float)M_PI / 180.0f;
        return;
    }
    m_data.patIdx = (m_data.patIdx + 1) % 6;
}

void Boss::s3(float px, float py) {
    switch (m_data.patIdx) {
    case 0:  // 圆形弹幕(32发)
        fireCircle(m_data.x, m_data.y, 32, 4.2f, QColor(0, 180, 200), 4);
        break;
    case 1:  // 半弧形密集直弹压制
        {
            float base = atan2(py - m_data.y, px - m_data.x);
            for (int i = -3; i <= 3; ++i) {
                fireStraight(m_data.x, m_data.y,
                    cos(base + i * 0.15f) * 5.5f, sin(base + i * 0.15f) * 5.5f, QColor(255, 100, 50), 4);
            }
        }
        break;
    case 2:  // 十字弹幕
        fireCrossBurst();
        break;
    case 3:  // 20发扇形 + 3向密集直弹
        fireFan(m_data.x, m_data.y, px, py, 20, 0.2f, 5.0f, QColor(255, 80, 80), 5);
        for (int k = -1; k <= 1; ++k) {
            for (int i = -2; i <= 2; ++i) {
                fireStraight(m_data.x + k * 25, m_data.y, i * 0.5f, 6.0f, QColor(255, 200, 50), 4);
            }
        }
        break;
    case 4:  // 粉尘弹幕（每4次触发1次）
        if (m_nextDust > 0) {
            m_nextDust--;
            fireFan(m_data.x, m_data.y, px, py, 8, 0.18f, 4.0f, QColor(200, 180, 140), 5);
            break;
        }
        m_nextDust = 3;
        m_dustOn = true;
        m_dustTimer = 240;
        m_dustWaves = 8;
        return;
    case 5:  // 海胆阵列+摆线
        m_urchinSeq = 4;
        m_urchinDelay = 0;
        m_urchinRounds = 0;
        m_urchinSweep = true;
        m_urchinSweepAng = 30.0f;
        m_urchinSweepDir = 1;
        m_urchinSweepCnt = 21;
        m_urchinSweepTimer = 0;
        m_arrayRotAngle = ((rand() % 4001) / 1000.0f - 2.0f) * 10.0f * (float)M_PI / 180.0f;
        return;
    case 6:  // 网状弹幕
        fireNet();
        return;
    }
    m_data.patIdx = (m_data.patIdx + 1) % 7;
}

// 辅助弹幕函数 
void Boss::fireCircle(float x, float y, int cnt, float spd, QColor c, float r)
{
    for (int i = 0; i < cnt; ++i) {
        float a = i * (2 * M_PI / cnt);
        fireStraight(x, y, cos(a) * spd, sin(a) * spd, c, r);
    }
}
void Boss::fireFan(float x, float y, float tx, float ty, int cnt, float spread, float spd, QColor c, float r) {
    float base = atan2(ty - y, tx - x);
    for (int i = 0; i < cnt; ++i) {
        float a = base + (i - (cnt - 1) / 2.0f) * spread;
        fireStraight(x, y, cos(a) * spd, sin(a) * spd, c, r);
    }
}
void Boss::fireStraight(float x, float y, float vx, float vy, QColor c, float r) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y; b.vx = vx * 1.2f; b.vy = vy * 1.2f;
    b.radius = r;
    b.color = c;
    b.damage = 1;
    b.type = 1;
    b.isBoss = true;
    b.alive = true;
    m_addBullet(b);
}
void Boss::fireSeeking(float x, float y, float vx, float vy, QColor c, float r) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y; b.vx = vx * 1.0f; b.vy = vy * 2.2f;
    b.radius = r; b.color = c; b.damage = 1;
    b.type = 2;
    b.isBoss = true;
    b.turnSpeed = 0.025f;
    b.trackDur = 150;
    b.alive = true;
    m_addBullet(b);
}

void Boss::fireUrchin(float x, float y, float vx, float vy) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y;
    b.vx = vx; b.vy = vy;
    b.radius = 7.0f;
    b.color = QColor(220, 55, 10);
    b.damage = 1;
    b.type = 6;
    b.isBoss = true;
    b.growRate = 0.3f;
    b.trackDur = 120;
    b.alive = true;
    m_addBullet(b);
}

void Boss::fireUrchinRow(int row) {
    float lx = m_data.x;
    float ly = m_data.y + 15 + m_px1.height() / 2.0f;
    if (m_urchinSweep) {
        float cosR = cos(m_arrayRotAngle);
        float sinR = sin(m_arrayRotAngle);
        auto fireRot = [&](float vx, float vy) {
            fireUrchin(lx, ly, vx * cosR - vy * sinR, vx * sinR + vy * cosR);
        };
        switch (row) {
        case 4: fireRot(-1.0f, 7.0f); fireRot(0, 7.0f); fireRot(1.0f, 7.0f); break;
        case 3: fireRot(-0.8f, 7.0f); fireRot(0.8f, 7.0f); break;
        case 2: fireRot(-1.3f, 7.0f); fireRot(0, 7.0f); fireRot(1.3f, 7.0f); break;
        case 1: fireRot(-1.1f, 7.0f); fireRot(1.1f, 7.0f); break;
        case 0: fireRot(-1.6f, 7.0f); fireRot(0, 7.0f); fireRot(1.6f, 7.0f); break;
        }
    } else {
        switch (row) {
        case 4: fireUrchin(lx, ly, -1.0f, 7.0f); fireUrchin(lx, ly, 0, 7.0f); fireUrchin(lx, ly, 1.0f, 7.0f); break;
        case 3: fireUrchin(lx, ly, -0.8f, 7.0f); fireUrchin(lx, ly, 0.8f, 7.0f); break;
        case 2: fireUrchin(lx, ly, -1.3f, 7.0f); fireUrchin(lx, ly, 0, 7.0f); fireUrchin(lx, ly, 1.3f, 7.0f); break;
        case 1: fireUrchin(lx, ly, -1.1f, 7.0f); fireUrchin(lx, ly, 1.1f, 7.0f); break;
        case 0: fireUrchin(lx, ly, -1.6f, 7.0f); fireUrchin(lx, ly, 0, 7.0f); fireUrchin(lx, ly, 1.6f, 7.0f); break;
        }
    }
}

void Boss::fireTrail(float x, float y, float vx, float vy) {
    if (!m_addBullet) return;
    Bullet b;
    b.x = x; b.y = y;
    b.vx = vx; b.vy = vy;
    b.radius = 6.0f;
    b.color = QColor(255, 180, 60);
    b.damage = 1;
    b.type = 7;
    b.isBoss = true;
    b.alive = true;
    m_addBullet(b);
}

void Boss::fireDust() {
    if (!m_addBullet) return;
    float regionH = 1460.0f / 3.0f;
    float cellW = 960.0f / 5.0f;
    float cellH = regionH / 2.0f;
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 2; row++) {
            Bullet b;
            b.x = col * cellW + 15.0f + (float)(rand() % (int)(cellW - 30.0f));
            b.y = row * cellH + 10.0f + (float)(rand() % (int)(cellH - 20.0f));
            b.vx = 0;
            b.vy = 7.0f;
            b.radius = 4.0f;
            b.color = QColor(200, 180, 140);
            b.damage = 1;
            b.type = 8;
            b.isBoss = true;
            b.alive = true;
            m_addBullet(b);
        }
    }
}

void Boss::fireCrossBurst() {
    float cx = m_data.x, cy = m_data.y + 40;
    float dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
    QColor cols[4] = {QColor(255,255,100), QColor(255,240,80), QColor(255,220,60), QColor(255,200,50)};
    for (int d = 0; d < 4; d++) {
        for (int j = -3; j <= 3; j++) {
            float spread = j * 0.07f;
            float baseA = atan2(dirs[d][1], dirs[d][0]);
            float a = baseA + spread;
            for (int k = 0; k < 3; k++) {
                float spd = 3.5f + k * 1.5f;
                fireStraight(cx, cy, cos(a) * spd, sin(a) * spd, cols[d], 3.0f + k * 0.5f);
            }
        }
    }
}

void Boss::fireVortex() {
    float tx = m_data.x, ty = m_data.y + 200;
    float ringR = 130;
    for (int i = 0; i < 20; i++) {
        float a = i * 2.0f * (float)M_PI / 20.0f;
        float sx = tx + cos(a) * ringR;
        float sy = ty + sin(a) * ringR;
        float toC = atan2(ty - sy, tx - sx);
        float radSpd = 1.8f;
        float tanSpd = 3.8f * ((i % 2) ? 1.0f : -1.0f);
        float vx = cos(toC) * radSpd + cos(toC + (float)M_PI / 2) * tanSpd;
        float vy = sin(toC) * radSpd + sin(toC + (float)M_PI / 2) * tanSpd;
        Bullet b;
        b.x = sx; b.y = sy; b.vx = vx; b.vy = vy; b.radius = 3.5f;
        b.color = QColor(80, 180, 240); b.damage = 1; b.type = 1;
        b.isBoss = true; b.alive = true; m_addBullet(b);
    }
    for (int i = 0; i < 16; i++) {
        float a = i * 2.0f * (float)M_PI / 16.0f;
        fireStraight(tx, ty, cos(a) * 5.5f, sin(a) * 5.5f, QColor(255, 80, 60), 3);
    }
}

void Boss::fireCage() {
    float ctrX = 480, ctrY = 960;
    float gap = 80;
    float side = 860.0f;  
    float yStart = ctrY - side / 2.0f; 
    float yEnd = ctrY + side / 2.0f; 
    for (float x = 50; x < 910; x += 45) {
        if (fabs(x - ctrX) < gap) continue;
        fireStraight(x, 480, 0, 3.5f, QColor(190, 50, 70), 3);
    }
    for (float x = 50; x < 910; x += 45) {
        if (fabs(x - ctrX) < gap) continue;
        fireStraight(x, 1440, 0, -3.5f, QColor(190, 50, 70), 3);
    }
    for (float y = yStart; y < yEnd; y += 45) {
        if (fabs(y - ctrY) < gap) continue;
        fireStraight(0, y, 3.5f, 0, QColor(190, 50, 70), 3);
    }
    for (float y = yStart; y < yEnd; y += 45) {
        if (fabs(y - ctrY) < gap) continue;
        fireStraight(960, y, -3.5f, 0, QColor(190, 50, 70), 3);
    }
}

void Boss::fireNet() {
    m_netOn = true;
    m_netTimer = 240;
    m_netFireTimer = 0;
}

void Boss::fireNetRows() {
    float cx = m_data.x;
    float midY = 280;
    float lowY = 480;
    float midX[2] = { cx - 200, cx + 200 };
    for (int i = 0; i < 2; i++) fireBulletPair(midX[i], midY);
    float lowX[4] = { cx - 350, cx - 120, cx + 120, cx + 350 };
    for (int i = 0; i < 4; i++) fireBulletPair(lowX[i], lowY);
}

void Boss::fireBulletPair(float sx, float sy) {
    float spd = 11.55f;
    float aL = 112.5f * (float)M_PI / 180.0f;
    float aR = 67.5f * (float)M_PI / 180.0f;
    Bullet b1;
    b1.x = sx; b1.y = sy; b1.vx = cos(aL) * spd; b1.vy = sin(aL) * spd;
    b1.radius = 5.0f; b1.color = QColor(255, 200, 60); b1.damage = 1;
    b1.type = 10; b1.isBoss = true; b1.alive = true;
    m_addBullet(b1);
    Bullet b2;
    b2.x = sx; b2.y = sy; b2.vx = cos(aR) * spd; b2.vy = sin(aR) * spd;
    b2.radius = 5.0f; b2.color = QColor(255, 200, 60); b2.damage = 1;
    b2.type = 10; b2.isBoss = true; b2.alive = true;
    m_addBullet(b2);
}
