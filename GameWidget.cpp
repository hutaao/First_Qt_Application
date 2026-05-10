#include "GameWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <QMessageBox>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent), m_rng(std::random_device{}()) {
    setFixedSize(960, 1460);        //屏幕比例
    setFocusPolicy(Qt::StrongFocus);
    m_timer.start(16, this);        //定时器
    m_sfxPlayer = new QMediaPlayer(this);  // 音效播放器
    m_player.setBulletAdder([this](const Bullet& b) { addPlayerBullet(b); });
    m_boss.setBulletAdder([this](const Bullet& b) { addEnemyBullet(b); });
    m_boss.setPhaseDropCb([this](float x, float y) { dropPhaseItems(x, y); });
    m_bg.load(":/res/bg.jpg");
    m_bgm.playMenu();
    m_pxSkBomb.load(":/res/skill_bomb.png");
    m_pxSkShield.load(":/res/skill_shield.png");
    m_pxSkFire.load(":/res/skill_fire.png");
    m_pxBulletPlayer.load(":/res/zidan1-1.png");
    m_pxBulletEne[0].load(":/res/3-1.png");
    m_pxBulletEne[1].load(":/res/3-2.png");
    m_pxBulletEne[2].load(":/res/3-3.png");
    m_pxBulletEne[3].load(":/res/3-4.png");
    m_pxBulletTrack.load(":/res/feidan.png");
    m_pxBulletBoss[0].load(":/res/4-1.png");
    m_pxBulletBoss[1].load(":/res/4-2.png");
    m_pxBulletBoss[2].load(":/res/4-3.png");
    m_pxBulletBoss[3].load(":/res/4-4.png");
    m_pxBulletSide.load(":/res/zidan1.png");
    int urBase = 40;
    m_pxUrchin = QPixmap(urBase * 4 + 8, urBase * 4 + 8);
    m_pxUrchin.fill(Qt::transparent);
    {
        QPainter pp(&m_pxUrchin);
        pp.setRenderHint(QPainter::Antialiasing);
        int cx = m_pxUrchin.width() / 2, cy = m_pxUrchin.height() / 2;
        QColor body(220, 50, 10), sp1(230, 60, 18), sp2(250, 155, 35);
        pp.setBrush(body);
        pp.setPen(Qt::NoPen);
        pp.drawEllipse(QPointF(cx, cy), urBase * 0.38f, urBase * 0.38f);
        pp.setBrush(QColor(255, 175, 40, 130));
        pp.drawEllipse(QPointF(cx, cy), urBase * 0.22f, urBase * 0.22f);
        int cnt = 18 + (int)(urBase * 0.6f);
        for (int i = 0; i < cnt; i++) {
            float a = i * 2.0f * (float)M_PI / cnt;
            float ir = urBase * 0.42f, or_ = urBase * 0.85f;
            pp.setPen(QPen((i % 3 == 0) ? sp1 : sp2, urBase * 0.035f + 0.5f));
            pp.drawLine(QPointF(cx + cos(a) * ir, cy + sin(a) * ir),
                        QPointF(cx + cos(a) * or_, cy + sin(a) * or_));
        }
    }
    int trW = 38, trH = 36;
    m_pxTrail = QPixmap(trW, trH);
    m_pxTrail.fill(Qt::transparent);
    {
        QPainter pp(&m_pxTrail);
        pp.setRenderHint(QPainter::Antialiasing);
        int cx = trW / 2, cy = trH / 2;
        QPainterPath path;
        path.moveTo(cx, cy - 20);
        path.lineTo(cx + 7, cy);
        path.lineTo(cx + 16, cy + 10);
        path.lineTo(cx - 16, cy + 10);
        path.lineTo(cx - 7, cy);
        path.closeSubpath();
        QLinearGradient grad(cx, cy - 20, cx, cy + 10);
        grad.setColorAt(0.0, QColor(255, 255, 80));
        grad.setColorAt(0.2, QColor(255, 220, 40));
        grad.setColorAt(0.6, QColor(255, 100, 20));
        grad.setColorAt(1.0, QColor(255, 30, 10));
        pp.setBrush(QColor(255, 180, 30, 50));
        pp.setPen(Qt::NoPen);
        pp.drawEllipse(QPointF(cx, cy - 5), 22, 22);
        pp.setBrush(grad);
        pp.setPen(QPen(QColor(255, 200, 60, 160), 1.5f));
        pp.drawPath(path);
    }
    m_pxSideScaled = m_pxBulletSide.scaled(m_pxBulletPlayer.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pxMenuBg.load(":/res/bg_menu.jpg");
    m_pxGameName.load(":/res/gamename.png");
    m_pxGameName = m_pxGameName.scaled(800, 400, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_pxUi1.load(":/res/ui1.png");
    m_pxUi1 = m_pxUi1.scaled(400, 110, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_pxUi2.load(":/res/ui2.png");
    m_pxUi2 = m_pxUi2.scaled(400, 120, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_pxUi3.load(":/res/ui3.png");
    m_pxUi3 = m_pxUi3.scaled(400, 110, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //  设置按钮区域
    int bw = m_pxUi1.width();
    int bh = m_pxUi1.height();
    int cx = (width() - bw) / 2;
    m_btnStart = QRect(cx, 620, bw, bh);
    m_btnHelp = QRect(cx, 780, bw, bh);
    m_btnMusic = QRect(cx, 940, bw, bh);

    // 暂停与结算界面按钮
    int smallW = 320, smallH = 70;
    int scx = (width() - smallW) / 2;
    m_btnResume = QRect(scx, height() / 2 - 90, smallW, smallH);
    m_btnPauseMusic = QRect(scx, height() / 2, smallW, smallH);
    m_btnMenu   = QRect(scx, height() / 2 + 90, smallW, smallH);
    m_btnRetry  = QRect(scx, height() / 2 + 30, smallW, smallH);
    m_btnHome   = QRect(scx, height() / 2 + 120, smallW, smallH);

}

void GameWidget::gameReset() {
    m_player.reset(width() / 2.0f, height() - 100.0f);//    初始位置
    m_enemies.clear();
    m_boss = Boss();        //  重置Boss对象
    m_boss.setBulletAdder([this](const Bullet& b) { addEnemyBullet(b); });
    m_boss.setPhaseDropCb([this](float x, float y) { dropPhaseItems(x, y); });
    memset(m_pBulletActive, 0, sizeof(m_pBulletActive));
    memset(m_eBulletActive, 0, sizeof(m_eBulletActive));
    m_pickups.clear(); m_pts.clear();
    m_wave = 0; m_score = 0; m_frame = 0; m_remain = 0; m_totalEnemies = 0; m_totBatches = 1; m_curBatch = 0; m_spawnedSoFar = 0; m_bossOn = false; m_victory = false;
    m_state = BREAK;
    m_breakTimer = 120;  // 统一2秒break
    m_bgm.playBattle();  // 战斗开始即播放战斗音乐
}

void GameWidget::startWave() {
    if (m_wave == m_totalWaves) {               //Boss 波
        m_boss.reset(width() / 2.0f, 220);
        m_bossOn = true;
        m_remain = 0;
        m_totBatches = 1; m_curBatch = 0; m_spawnedSoFar = 0;
    }
    else if (m_wave == 0) {
        m_totBatches = 1; m_curBatch = 0; m_spawnedSoFar = 0;
    }
    else {
        m_totalEnemies = 14 + m_wave * 6;        // 第1波20只 → 第4波38只
        m_batchCap = (int)qMin(4 + (m_wave - 1), 6);  // 每批4→5→6→6
        m_totBatches = (m_totalEnemies + m_batchCap - 1) / m_batchCap;  // 向上取整
        m_curBatch = 0; m_spawnedSoFar = 0;
        m_remain = 0;
        m_bossOn = false;
        spawnBatch();                            //立即生成第一批
    }
}

void GameWidget::tick() {
    m_frame++;
    m_bg.update(m_bgSpeed, height());
    m_player.updateSkills();
    // 仅清除敌方弹幕，保留敌人实体和波次状态
    if (m_player.bombSkill().active == 1) {
        m_player.bombSkill().active = 0;
        memset(m_eBulletActive, 0, sizeof(m_eBulletActive));
        genParticles(m_player.x(), m_player.y(), 60, QColor(255, 220, 50));        // 全屏金色冲击波特效
    }
    m_player.update(m_keys, width(), height(), m_frame);
    m_player.tryShoot();

    for (auto& e : m_enemies) { //  不创建临时变量，直接引用
        if (e.isAlive())
            e.update(m_player.x(), m_player.y(), m_frame);
    }
    if (m_bossOn && m_boss.isAlive()) m_boss.update(m_player.x(), m_player.y(), m_frame);   //更新boss状态
    if (m_bossOn && m_boss.isDyingDone()) {
        m_bossOn = false;
        genParticles(m_boss.data().x, m_boss.data().y, 80, QColor(255, 100, 0));
        genParticles(m_boss.data().x, m_boss.data().y, 50, QColor(255, 220, 0));
        genParticles(m_boss.data().x, m_boss.data().y, 40, Qt::white);
        m_state = VICTORY_FLY;
        memset(m_pBulletActive, 0, sizeof(m_pBulletActive));
        memset(m_eBulletActive, 0, sizeof(m_eBulletActive));
        m_pickups.clear();
    }

    for (int i = 0; i < MAX_PB; i++) {
        if (!m_pBulletActive[i]) continue;
        Bullet& b = m_pBullets[i];
        b.update(m_player.x(), m_player.y());
        if (b.isOutOfScreen(width(), height())) { m_pBulletActive[i] = false; continue; }
        for (auto& e : m_enemies) {         //  碰撞检测
            if (!e.isAlive()) continue;
            if (dist(b.x, b.y, e.data().x, e.data().y) < b.radius + e.radius()) {
                m_pBulletActive[i] = false;
                e.data().hp -= b.damage;
                if (e.data().hp <= 0) {
                    e.kill(); m_remain--; 
                    m_score += (e.data().type == 0 ? 150 : 300);   // 普通150，精英300
                    genParticles(e.data().x, e.data().y, 25, QColor(255, 80, 20));  // 橙红色爆炸粒子
                    if (e.data().type == 1) genPickup(e.data().x, e.data().y);  // 仅精英敌人掉落道具
                }
                break;   //  保证一个子弹只能命中一个敌人
            }
        }
        if (m_pBulletActive[i] && m_bossOn && m_boss.isAlive()) {
            if (dist(b.x, b.y, m_boss.data().x, m_boss.data().y) < b.radius + m_boss.radius()) {
                m_pBulletActive[i] = false;
                m_boss.data().hp -= b.damage;
                if (m_boss.data().hp <= 0) {
                    m_boss.data().hp = 0;
                    if (!m_boss.isDying()) {
                        m_boss.startDying();
                        m_victory = true;
                        m_score += 5000;
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_EB; i++) {
        if (!m_eBulletActive[i]) continue;
        Bullet& b = m_eBullets[i];
        b.update(m_player.x(), m_player.y());
        if (b.isOutOfScreen(width(), height())) { m_eBulletActive[i] = false; continue; }
        if (!m_player.isInv() && dist(b.x, b.y, m_player.x(), m_player.y()) < b.radius + m_player.radius()) {    //  碰撞检测
            m_eBulletActive[i] = false;
            m_player.hit();
            genParticles(m_player.x(), m_player.y(), 18, QColor(100, 200, 255));
            if (m_player.hp() <= 0) { m_victory = false; genParticles(m_player.x(), m_player.y(), 60, QColor(255, 100, 50)); m_state = OVER; return; }
        }
    }
    if (m_bossOn && m_boss.isAlive() && !m_player.isInv() && dist(m_boss.data().x, m_boss.data().y, m_player.x(), m_player.y()) < m_boss.radius() + m_player.radius()) {
        m_player.hit();
        genParticles(m_player.x(), m_player.y(), 18, QColor(100, 200, 255));
        if (m_player.hp() <= 0) { m_victory = false; int hitPen = qMin(m_player.hits(), 10) * 500; int pkBonus = qMin(m_player.pickupsCollected(), 20) * 200; m_score = qMax(0, m_score - hitPen + pkBonus); m_state = OVER; return; }
    }

    if (m_pickups.size() > 30)
        m_pickups.removeFirst();

    for (auto& pk : m_pickups) {
        if (!pk.alive) continue;
        pk.x += pk.vx; pk.y += pk.vy;
        float edge = m_player.radius() + 5.0f;  // 道具边界与玩家可达区域对齐
        if (pk.x < edge)            { pk.x = edge;     pk.vx = -pk.vx; }
        if (pk.x > width() - edge)  { pk.x = width() - edge; pk.vx = -pk.vx; }
        if (pk.y < edge)            { pk.y = edge;     pk.vy = -pk.vy; }
        if (pk.y > height() - edge) { pk.y = height() - edge; pk.vy = -pk.vy; }
        if (dist(m_player.x(), m_player.y(), pk.x, pk.y) < 55) {        //  道具被拾取
            pk.alive = false;
            if (pk.type == 2 && !m_muted && m_player.fireSkill().active <= 0) {
                m_sfxPlayer->stop();
                m_sfxPlayer->setMedia(QUrl("qrc:/res/rage.mp3"));
                m_sfxPlayer->setVolume(100);
                m_sfxPlayer->play();
            }
            m_player.getPickup(pk.type);
        }
    }

    for (auto& p : m_pts) {     //  粒子特效寿命
        if (p.alive) { p.x += p.vx; p.y += p.vy; p.life--; if (p.life <= 0) p.alive = false; }
    }

    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(), [](const EnemyUnit& e) { return !e.isAlive(); }), m_enemies.end());
    m_pickups.erase(std::remove_if(m_pickups.begin(), m_pickups.end(), [](const Pickup& p) { return !p.alive; }), m_pickups.end());
    m_pts.erase(std::remove_if(m_pts.begin(), m_pts.end(), [](const Particle& p) { return !p.alive; }), m_pts.end());

    checkWave();
}

void GameWidget::spawnBatch() {
    int batchSize = qMin(m_batchCap, m_totalEnemies - m_spawnedSoFar);

    for (int i = 0; i < batchSize; ++i) {
        bool isElite = (rf(0.0f, 1.0f) < 0.28f);  // 每只敌人28%概率为精英
        int typ = isElite ? 1 : 0;

        int pattern;
        if (typ == 0) {
            int r = ri(0, 3);           // 0,1,2,3
            pattern = (r >= 2) ? 0 : r; // 0:50%, 1:25%, 2:25%
        } else {
            pattern = ri(0, 3);         // 精英4种均等
        }

        EnemyUnit eu;
        eu.reset(rf(80, width() - 80), rf(60, height() / 3 - 60), typ, m_wave);
        eu.setBulletAdder([this](const Bullet& b) { addEnemyBullet(b); });
        eu.setPattern(pattern);
        int stagger = (60 / batchSize) * i;
        eu.data().phaseTimer = (stagger < 60) ? (60 - stagger) : (120 - stagger);
        eu.data().attackPhase = (stagger < 60) ? 0 : 1;
        m_enemies.append(eu);
    }
    m_remain += batchSize;
    m_spawnedSoFar += batchSize;
    m_curBatch++;
}

void GameWidget::checkWave() {
    if (m_state != PLAY) return;
    if (m_wave == m_totalWaves) return;
    if (m_remain <= 0 && m_enemies.isEmpty()) {
        if (m_curBatch < m_totBatches)
            spawnBatch();
        else {
            m_state = BREAK;
            m_breakTimer = 120;
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
            if (m_wave == m_totalWaves) {
                m_state = BOSS_WARN;
                m_warnTimer = m_warnLen;
                m_bgm.playBoss();
                memset(m_pBulletActive, 0, sizeof(m_pBulletActive));
                memset(m_eBulletActive, 0, sizeof(m_eBulletActive));
                update(); return;
            }
            else {
                m_state = PLAY;
            }
        }
        tick();
        update(); return;
    }
    if (m_state == BOSS_WARN) {                           // Boss 预警倒计时
        m_warnTimer--; m_frame++;
        m_bg.update(m_bgSpeed, height());
        m_player.updateSkills();
        m_player.update(m_keys, width(), height(), m_frame);
        if (m_warnTimer <= 0) { m_state = PLAY; startWave(); }
        update(); return;
    }
    if (m_state == VICTORY_FLY) {                         // 胜利离场
        m_frame++;
        m_bg.update(m_bgSpeed, height());
        m_player.updateSkills();
        m_player.flyOff();
        if (m_player.offScreen()) {
            int hitPen = qMin(m_player.hits(), 10) * 500;
            int pkBonus = qMin(m_player.pickupsCollected(), 20) * 200;
            m_score = qMax(0, m_score - hitPen + pkBonus);
            m_state = OVER;
        }
        update(); return;
    }
    tick(); update();
}

void GameWidget::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    if (m_state == MENU && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) { gameReset(); return; }
    if (m_state == OVER && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) { gameReset(); return; }
    if (m_state == OVER && event->key() == Qt::Key_Escape) {  // GameOver按Esc返回主菜单
        m_state = MENU;
        m_bgm.playMenu();
        return;
    }
    if ((m_state == PLAY || m_state == BREAK || m_state == BOSS_WARN) && event->key() == Qt::Key_Escape) {
        m_prePauseState = m_state;  // 记录暂停前的状态
        m_state = PAUSED;
        return;
    }
    if (m_state == PAUSED && event->key() == Qt::Key_Escape) {
        m_state = m_prePauseState;  // 恢复到暂停前的状态
        return;
    }
    if (m_state == PLAY || m_state == BREAK || m_state == BOSS_WARN) {
        if (event->key() == Qt::Key_1) m_player.useSkill(1);  // 释放炸弹
        if (event->key() == Qt::Key_2) m_player.useSkill(2);
    }
    if (m_state == PLAY || m_state == BREAK || m_state == BOSS_WARN) {
        m_keys.insert(event->key());
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    m_keys.remove(event->key());
}

void GameWidget::mousePressEvent(QMouseEvent* event) {
    QPoint pos = event->pos();

    if (m_state == MENU) {
        if (m_btnStart.contains(pos)) {
            gameReset();
        }
        else if (m_btnHelp.contains(pos)) {
            QMessageBox msg(this);
            msg.setWindowTitle(QStringLiteral("帮助"));
            msg.setTextFormat(Qt::RichText);
            msg.setText(QStringLiteral(
                "<h2 style='text-align:center;margin:0'>雷霆战机</h2>"
                "<hr>"
                "<h3 style='margin:8px 0 4px 0'>【操作说明】</h3>"
                "<p style='margin:2px 0'>方向键/WASD 移动机体&nbsp;&nbsp;|&nbsp;&nbsp;1/2/3 释放技能&nbsp;&nbsp;|&nbsp;&nbsp;Esc 暂停</p>"
                "<h3 style='margin:8px 0 4px 0'>【计分规则】</h3>"
                "<p style='margin:2px 0'>普通敌人 <b>+150</b>&nbsp;&nbsp;|&nbsp;&nbsp;精英敌人 <b>+300</b>&nbsp;&nbsp;|&nbsp;&nbsp;Boss 击杀 <b>+5000</b></p>"
                "<p style='margin:2px 0'>被击中 <b>-500</b>/次（最多计10次）</p>"
                "<p style='margin:2px 0'>道具收集 <b>+200</b>/个（最多计20个）</p>"
                "<h3 style='margin:8px 0 4px 0'>【技能介绍】</h3>"
                "<p style='margin:4px 0'><img src=':/res/skill_bomb.png' width='24' height='24' style='vertical-align:middle'> <b>炸弹清屏</b>（键1）&nbsp;—&nbsp;清除全屏敌方弹幕，冷却3秒</p>"
                "<p style='margin:4px 0'><img src=':/res/skill_shield.png' width='24' height='24' style='vertical-align:middle'> <b>能量护盾</b>（键2）&nbsp;—&nbsp;3秒无敌状态，免疫伤害</p>"
                "<p style='margin:4px 0'><img src=':/res/skill_fire.png' width='24' height='24' style='vertical-align:middle'> <b>火力增强</b>（键3）&nbsp;—&nbsp;伤害提升，持续7秒</p>"
            ));
            msg.exec();
        }
        else if (m_btnMusic.contains(pos)) {
            QMessageBox::StandardButton reply;
            if (m_muted) {
                // 当前已静音，弹窗确认是否打开
                reply = QMessageBox::question(this, "音乐设置", "是否打开音乐？",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    m_muted = false;
                    m_bgm.setMuted(false);
                }
            }
            else {
                reply = QMessageBox::question(this, "音乐设置", "是否静音？",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    m_muted = true;
                    m_bgm.setMuted(true);
                }
            }
        }
    }
    else if (m_state == PAUSED) {
        if (m_btnResume.contains(pos)) {
            m_state = PLAY;
        }
        else if (m_btnPauseMusic.contains(pos)) {
            // 暂停界面音乐静音切换
            m_muted = !m_muted;
            m_bgm.setMuted(m_muted);
        }
        else if (m_btnMenu.contains(pos)) {
            m_state = MENU;
            m_bgm.playMenu();
        }
    }
    else if (m_state == OVER) {
        if (m_btnRetry.contains(pos)) {
            gameReset();
        }
        else if (m_btnHome.contains(pos)) {
            m_state = MENU;
            m_bgm.playMenu();
        }
    }
}

void GameWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    m_bg.draw(p, width(), height());

    if (m_state == MENU) {
        if (!m_pxMenuBg.isNull())
            p.drawPixmap(0, 0, m_pxMenuBg);
        if (!m_pxGameName.isNull()) {
            int tx = (width() - m_pxGameName.width()) / 2;
            p.drawPixmap(tx, 280, m_pxGameName);
        }
        if (!m_pxUi1.isNull()) p.drawPixmap(m_btnStart.topLeft(), m_pxUi1);
        if (!m_pxUi2.isNull()) p.drawPixmap(m_btnHelp.topLeft(), m_pxUi2);
        if (!m_pxUi3.isNull()) p.drawPixmap(m_btnMusic.topLeft(), m_pxUi3);
        return;
    }
    if (m_state == OVER) {
        p.fillRect(rect(), QColor(0, 0, 0, 210));
        if (m_victory) {
            p.setPen(QColor(255, 215, 0));
            p.setFont(QFont("Microsoft YaHei", 38, QFont::Bold));
            p.drawText(QRect(0, height() / 2 - 180, width(), 50), Qt::AlignCenter, "VICTORY");
        } else {
            p.setPen(QColor(255, 50, 50));
            p.setFont(QFont("Microsoft YaHei", 34, QFont::Bold));
            p.drawText(QRect(0, height() / 2 - 180, width(), 50), Qt::AlignCenter, "GAME OVER");
        }
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 16));
        int hitPen = qMin(m_player.hits(), 10) * 500;
        int pkBonus = qMin(m_player.pickupsCollected(), 20) * 200;
        p.drawText(QRect(0, height() / 2 - 120, width(), 30), Qt::AlignCenter, QString("Base Score: %1").arg(m_score + hitPen - pkBonus));
        p.setPen(QColor(255, 150, 100));
        p.drawText(QRect(0, height() / 2 - 88, width(), 28), Qt::AlignCenter, QString("Hit Penalty  -%1  (%2 hits)").arg(hitPen).arg(qMin(m_player.hits(), 10)));
        p.setPen(QColor(100, 220, 120));
        p.drawText(QRect(0, height() / 2 - 60, width(), 28), Qt::AlignCenter, QString("Pickup Bonus  +%1  (%2 collected)").arg(pkBonus).arg(qMin(m_player.pickupsCollected(), 20)));
        p.setPen(Qt::yellow);
        p.setFont(QFont("Arial", 22, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 20, width(), 40), Qt::AlignCenter, QString("Final: %1").arg(m_score));
        p.fillRect(m_btnRetry, QColor(40, 120, 200, 200));
        p.setPen(Qt::white);
        p.setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
        p.drawText(m_btnRetry, Qt::AlignCenter, "重新开始 (Enter)");
        p.fillRect(m_btnHome, QColor(80, 80, 80, 200));
        p.drawText(m_btnHome, Qt::AlignCenter, "返回主菜单 (Esc)");
        return;
    }

    for (const auto& pk : m_pickups) {
        if (!pk.alive) continue;
        QPixmap* px = nullptr;
        if (pk.type == 0) px = &m_pxSkBomb;
        else if (pk.type == 1) px = &m_pxSkShield;
        else if (pk.type == 2) px = &m_pxSkFire;
        if (px && !px->isNull()) {
            QPixmap scaled = px->scaled(68, 68, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            p.drawPixmap(pk.x - scaled.width() / 2, pk.y - scaled.height() / 2, scaled);
        }
    }
    for (int i = 0; i < MAX_PB; i++) {
        if (!m_pBulletActive[i]) continue;
        const Bullet& b = m_pBullets[i];
        if (b.type == 5 && !m_pxBulletSide.isNull()) {
            float angle = atan2(b.vy, b.vx);
            p.save();
            p.translate(b.x, b.y);
            p.rotate(qRadiansToDegrees(angle) + 90.0);
            p.drawPixmap(-m_pxSideScaled.width() / 2, -m_pxSideScaled.height() / 2, m_pxSideScaled);
            p.restore();
        } else if (!m_pxBulletPlayer.isNull()) {
            p.drawPixmap(b.x - m_pxBulletPlayer.width() / 2, b.y - m_pxBulletPlayer.height() / 2, m_pxBulletPlayer);
        }
    }
    for (int i = 0; i < MAX_EB; i++) {
        if (!m_eBulletActive[i]) continue;
        const Bullet& b = m_eBullets[i];
        if (b.type == 2) {
            if (!m_pxBulletTrack.isNull()) {
                float angle = atan2(b.vy, b.vx);
                p.save();
                p.translate(b.x, b.y);
                p.rotate(qRadiansToDegrees(angle) + 90.0);
                p.drawPixmap(-m_pxBulletTrack.width() / 2, -m_pxBulletTrack.height() / 2, m_pxBulletTrack);
                p.restore();
            }
        } else if (b.type == 6) {
            float scale = b.radius / 36.0f;
            int half = m_pxUrchin.width() / 2;
            p.save();
            p.translate(b.x, b.y);
            p.scale(scale, scale);
            p.drawPixmap(-half, -half, m_pxUrchin);
            p.restore();
        } else if (b.type == 7) {
            float angle = atan2(b.vy, b.vx);
            p.save();
            p.translate(b.x,     b.y);
            p.rotate(qRadiansToDegrees(angle) + 90.0);
            p.drawPixmap(-m_pxTrail.width() / 2, -m_pxTrail.height() / 2, m_pxTrail);
            p.restore();
        } else if (b.type == 10) {
            float angle = atan2(b.vy, b.vx);
            p.save();
            p.translate(b.x, b.y);
            p.rotate(qRadiansToDegrees(angle) + 90.0);
            p.drawPixmap(-m_pxBulletPlayer.width() / 2, -m_pxBulletPlayer.height() / 2, m_pxBulletPlayer);
            p.restore();
        } else if (b.type == 8) {
            if (!m_pxBulletEne[1].isNull()) {
                p.drawPixmap(b.x - m_pxBulletEne[1].width() / 2, b.y - m_pxBulletEne[1].height() / 2, m_pxBulletEne[1]);
            }
        } else {
            int idx = (b.color.red() * 7 + b.color.green() * 13 + b.color.blue() * 17) % 4;
            QPixmap* px = b.isBoss ? &m_pxBulletBoss[idx] : &m_pxBulletEne[idx];
            if (px && !px->isNull()) {
                p.drawPixmap(b.x - px->width() / 2, b.y - px->height() / 2, *px);
            }
        }
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
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.drawText(10, 20, QString("Wave %1/%2  Score: %3  HP: %4/%5")
        .arg(m_wave).arg(m_totalWaves).arg(m_score).arg(m_player.hp()).arg(m_player.maxHp()));
    if (m_wave != m_totalWaves) {
        p.setFont(QFont("Arial", 11, QFont::Bold));
        int totRemain = m_remain + (m_totalEnemies - m_spawnedSoFar);
        p.drawText(10, 40, QString::fromWCharArray(L"剩余敌人：%1").arg(totRemain));
    }

    // Boss 三层重叠血条
    if (m_wave == m_totalWaves && m_bossOn && m_boss.isAlive()) {
        float barFullW = width() - 20.0f;
        float hp = (float)m_boss.data().hp;
        float maxHp = (float)m_boss.data().maxHp;
        float s1to2 = maxHp * 0.66f;  // 阶段1→2边界，约198
        float s2to3 = maxHp * 0.33f;  // 阶段2→3边界，约99
        int barH = 12;
        int barY = 28;

        p.fillRect(10, barY, (int)barFullW, barH, QColor(25, 25, 25));
        p.setPen(QColor(80, 80, 80));
        p.drawRect(10, barY, (int)barFullW, barH);

        float redW = 0, orangeW = 0, tealW = 0;

        if (hp > s1to2) { // 第一阶段：青绿色为最上层，仅青绿色受击削减
            redW = barFullW;
            orangeW = barFullW;
            tealW = barFullW * (hp - s1to2) / (maxHp - s1to2);
        }
        else if (hp > s2to3) { // 第二阶段：青绿色已耗尽，橙色为最上层，仅橙色受击削减
            redW = barFullW;
            orangeW = barFullW * (hp - s2to3) / (s1to2 - s2to3);
            tealW = 0;
        }
        else { // 第三阶段：仅红色存在，受击削减至透明背景
            redW = barFullW * hp / s2to3;
            orangeW = 0;
            tealW = 0;
        }

        if (redW > 0)    p.fillRect(10, barY, (int)redW, barH, QColor(200, 35, 35));
        if (orangeW > 0) p.fillRect(10, barY, (int)orangeW, barH, QColor(230, 150, 30));
        if (tealW > 0)   p.fillRect(10, barY, (int)tealW, barH, QColor(30, 200, 150));
    }

    if (m_state == BREAK) {
        p.fillRect(0, height() / 2 - 30, width(), 60, QColor(0, 0, 0, 150));
        p.setPen(Qt::white); p.setFont(QFont("Arial", 22, QFont::Bold));
        int sec = m_breakTimer / 60 + 1;
        p.drawText(rect(), Qt::AlignCenter, QString("Wave %1 incoming... %2").arg(m_wave + 1).arg(sec));
    }

    if (m_state == BOSS_WARN) {
        if (m_newWarn) {
            int ft = m_warnTimer;  // 180→0

            p.fillRect(rect(), Qt::black);

            {
                float cx = width() / 2.0f;
                float sep = fmodf(ft * 1.5f, 80.0f);
                for (int i = 0; i < 35; i++) {
                    float bx = i * 45.0f - sep;
                    float lx = fmodf(bx + 800.0f, 90.0f) - 45.0f;
                    float delta = lx - cx;
                    float dist = fabsf(delta);
                    float shift = dist * 0.2f;
                    float sx = (delta < 0) ? lx - shift : lx + shift;
                    bool main = (i % 4 == 0);
                    QColor sc = main ? QColor(200, 20, 10, 140) : QColor(140, 10, 5, 70);
                    p.setPen(QPen(sc, main ? 10 : 5));
                    p.drawLine(QPointF(sx + 40, -10), QPointF(sx - 40, height() + 10));
                    if (main) {
                        p.setPen(QPen(QColor(255, 35, 15, 35), 20));
                        p.drawLine(QPointF(sx + 40, -10), QPointF(sx - 40, height() + 10));
                    }
                }
            }
            //warning绘制
            {
                QFont rf("Arial", 16, QFont::Bold);
                p.setFont(rf);
                QString line;
                for (int i = 0; i < 25; i++) line += QString::fromWCharArray(L"  WARNING  ▸");
                float speed = 2.0f;
                float topOff = fmodf(ft * speed, 280.0f);
                float botOff = fmodf(-ft * speed, 280.0f);
                QColor txCol(170, 18, 10, 180);
                QColor stCol(100, 8, 4, 140);
                float ty = 145.0f, by = 1290.0f;
                p.save();
                p.translate(-topOff, ty);
                p.setPen(QPen(stCol, 1)); p.drawText(QPointF(0, 0), line);
                p.setPen(txCol);          p.drawText(QPointF(0, 0), line);
                p.restore();
                p.save();
                p.translate(-botOff, by);
                p.setPen(QPen(stCol, 1)); p.drawText(QPointF(0, 0), line);
                p.setPen(txCol);          p.drawText(QPointF(0, 0), line);
                p.restore();
            }

            bool mainVisible = (ft <= 150);

            if (mainVisible) {
                {
                    int barH = 38, barY = 8;
                    p.fillRect(0, barY, width(), barH, QColor(100, 8, 4, 220));
                    QLinearGradient grad(0, barY, 0, barY + barH);
                    grad.setColorAt(0.0, QColor(255, 30, 10, 90));
                    grad.setColorAt(0.5, QColor(255, 10, 5, 20));
                    grad.setColorAt(1.0, QColor(255, 30, 10, 90));
                    p.fillRect(0, barY, width(), barH, grad);
                    p.setPen(Qt::white);
                    p.setFont(QFont("Microsoft YaHei", 15, QFont::Bold));
                    p.drawText(22, barY, 200, barH, Qt::AlignVCenter | Qt::AlignLeft, QString::fromWCharArray(L"▌ BOSS  Lv.5"));
                    p.drawText(width() - 222, barY, 200, barH, Qt::AlignVCenter | Qt::AlignRight, QString::fromWCharArray(L"太空原种  ▐"));
                }

                {
                    auto drawSkull = [&](float cx, float cy) {
                        p.save();
                        p.translate(cx, cy);

                        p.setPen(Qt::NoPen);
                        p.setBrush(QColor(255, 15, 5, 25));
                        p.drawEllipse(QPointF(0, 0), 50, 50);
                        p.setBrush(QColor(255, 15, 5, 15));
                        p.drawEllipse(QPointF(0, 0), 65, 65);

                        QPolygonF tri;
                        tri << QPointF(0, -42) << QPointF(-36, 30) << QPointF(36, 30);
                        p.setBrush(QColor(15, 0, 0, 210));
                        p.setPen(QPen(QColor(200, 25, 15), 3));
                        p.drawPolygon(tri);

                        p.setPen(QPen(QColor(180, 40, 25), 2));
                        p.setBrush(QColor(25, 0, 0, 180));
                        p.drawEllipse(QPointF(0, -10), 14, 16);

                        p.setBrush(QColor(255, 25, 10));
                        p.setPen(Qt::NoPen);
                        p.drawEllipse(QPointF(-6, -12), 4, 5);
                        p.drawEllipse(QPointF(6, -12), 4, 5);
                        
                        p.setPen(QPen(QColor(180, 30, 20), 2));
                        p.drawLine(QPointF(-6, -4), QPointF(6, -4));
                        
                        p.setPen(QPen(QColor(180, 35, 25), 3));
                        p.drawLine(QPointF(-16, 8), QPointF(16, -2));
                        p.drawLine(QPointF(-16, -2), QPointF(16, 8));
                        p.restore();
                    };
                    drawSkull(width() * 0.17f, height() * 0.518f);
                    drawSkull(width() * 0.83f, height() * 0.518f);
                }

                {
                    float scale = 1.0f;
                    int flyStart = 150, flyEnd = 105;
                    if (ft > flyStart) {
                        scale = 3.0f;
                    } else if (ft > flyEnd) {
                        float prog = (float)(flyStart - ft) / (flyStart - flyEnd);
                        scale = 3.0f - prog * 2.0f;
                    }

                    bool glowOn = true;
                    if (ft <= flyEnd) {
                        glowOn = ((ft / 18) % 2 == 0);
                    }

                    p.save();
                    p.translate(width() / 2.0f, height() / 2.0f + 8);
                    p.scale(scale, scale);

                    QFont bigF("Microsoft YaHei", 42, QFont::Bold);
                    if (glowOn) {
                        for (int g = 3; g >= 0; g--) {
                            int a = 18 + g * 22;
                            float w = 8.0f + g * 7.0f;
                            p.setPen(QPen(QColor(255, 25, 10, a), w));
                            p.setFont(bigF);
                            p.drawText(QRectF(-260, -55, 520, 110), Qt::AlignCenter, QString::fromWCharArray(L"危险警告"));
                        }
                    }
                    p.setPen(QColor(230, 20, 10));
                    p.setFont(bigF);
                    p.drawText(QRectF(-260, -55, 520, 110), Qt::AlignCenter, QString::fromWCharArray(L"危险警告"));
                    p.restore();
                }

                {
                    int sec = (ft - 1) / 60 + 1;
                    if (sec < 1) sec = 1;
                    p.setPen(QColor(180, 30, 20, 160));
                    p.setFont(QFont("Arial", 16, QFont::Bold));
                    p.drawText(QRect(0, height() - 55, width(), 30), Qt::AlignCenter,
                        QString::fromWCharArray(L"距离 Boss 抵达  ") + QString::number(sec) + QString::fromWCharArray(L"  sec"));
                }
            }
        } else {
            p.fillRect(0, height() / 2 - 50, width(), 100, QColor(200, 0, 0, 200));
            p.setPen(Qt::white); p.setFont(QFont("Arial", 28, QFont::Bold));
            p.drawText(rect(), Qt::AlignCenter, "WARNING !!");
            p.setFont(QFont("Arial", 18));
            p.drawText(QRect(0, height() / 2, width(), 50), Qt::AlignCenter, "Boss Approaching...");
        }
    }

    if (m_state == VICTORY_FLY) {
        p.fillRect(0, height() / 2 - 80, width(), 160, QColor(0, 0, 0, 120));
        QFont titleFont("Microsoft YaHei", 48, QFont::Bold);
        p.setFont(titleFont);
        for (int g = 2; g >= 0; g--) {
            int a = 30 + g * 30;
            float w = 4.0f + g * 3.0f;
            p.setPen(QPen(QColor(255, 180, 0, a), w));
            p.drawText(QRect(0, height() / 2 - 75, width(), 150), Qt::AlignCenter, QString::fromWCharArray(L"挑战胜利"));
        }
        p.setPen(QColor(255, 215, 0));
        p.drawText(QRect(0, height() / 2 - 75, width(), 150), Qt::AlignCenter, QString::fromWCharArray(L"挑战胜利"));
    }

    int skW = 50, skH = 50;
    int skY = height() - skH - 25;

    auto drawIcon = [&](int x, QPixmap& px, const Skill& sk, int key) {
        if (px.isNull()) return;
        QPixmap scaled = px.scaled(skW, skH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(x, skY, scaled);
        // 冷却覆盖
        if (sk.cd > 0) {
            float cdPer = 1.0f - (float)sk.cd / sk.maxCD;
            p.fillRect(x, skY + skH * (1.0f - cdPer), skW, skH * cdPer, QColor(0, 0, 0, 150));
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(x, skY, skW, skH), Qt::AlignCenter, QString::number(sk.cd / 60 + 1));
        }
        else if (sk.active > 0) {
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(x, skY, skW, skH), Qt::AlignCenter, QString::number(sk.active / 60 + 1));
        }
        // 按键数字
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 9));
        p.drawText(QRect(x, skY + skH + 2, skW, 15), Qt::AlignCenter, QString("[%1]").arg(key));
        };

    drawIcon(20, m_pxSkBomb, m_player.bombSkill(), 1);
    // 炸弹库存角标
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 11, QFont::Bold));
    p.drawText(QRect(20, skY, skW, skH), Qt::AlignBottom | Qt::AlignRight, QString("x%1").arg(m_player.bombStock()));

    if (m_state == PAUSED) {
        p.fillRect(rect(), QColor(0, 0, 0, 200));
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 32, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 180, width(), 50), Qt::AlignCenter, "PAUSED");
        // 继续游戏按钮
        p.fillRect(m_btnResume, QColor(40, 160, 40, 200));
        p.setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
        p.drawText(m_btnResume, Qt::AlignCenter, "继续游戏 (ESC)");
        // 音乐静音切换按钮
        if (m_muted) {
            p.fillRect(m_btnPauseMusic, QColor(180, 140, 40, 200));
            p.drawText(m_btnPauseMusic, Qt::AlignCenter, "音乐：关");
        }
        else {
            p.fillRect(m_btnPauseMusic, QColor(40, 120, 180, 200));
            p.drawText(m_btnPauseMusic, Qt::AlignCenter, "音乐：开");
        }
        // 返回主菜单按钮
        p.fillRect(m_btnMenu, QColor(160, 40, 40, 200));
        p.drawText(m_btnMenu, Qt::AlignCenter, "返回主菜单");
    }
}

void GameWidget::genPickup(float x, float y) {
    if (ri(0, 4) < 2) {  // 40% 概率掉落
        Pickup pk; pk.x = x; pk.y = y; pk.type = ri(0, 2); pk.alive = true;
        float ang = (rf(0.0f, 1.0f) < 0.65f) ? rf(0.0f, static_cast<float>(M_PI)) : rf(static_cast<float>(M_PI), 2.0f * static_cast<float>(M_PI));  // 65%概率飘向下半屏
        float spd = rf(1.0f, 2.5f);  // 道具飘动速度（像素/帧）
        pk.vx = cos(ang) * spd;
        pk.vy = sin(ang) * spd;
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

void GameWidget::dropPhaseItems(float x, float y) {
    for (int typ = 0; typ < 3; typ++) {
        Pickup pk;
        pk.x = x + rf(-30.0f, 30.0f);
        pk.y = y + rf(-20.0f, 20.0f);
        pk.type = typ;
        pk.alive = true;
        float ang = rf(0.0f, 2.0f * static_cast<float>(M_PI));
        float spd = rf(1.0f, 2.2f);
        pk.vx = cos(ang) * spd;
        pk.vy = sin(ang) * spd;
        m_pickups.append(pk);
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