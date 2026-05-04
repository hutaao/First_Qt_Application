#include "GameWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent), m_rng(std::random_device{}()) {
    setFixedSize(640, 960);
    setFocusPolicy(Qt::StrongFocus);
    m_timer.start(16, this);
    m_player.setBulletAdder([this](const Bullet& b) { m_pBullets.append(b); });
    m_state = PLAY;     //暂无菜单
    m_player.reset(width() / 2.0f, height() - 100.0f);  //重置位置
}

void GameWidget::gameReset() {
    m_player.reset(width() / 2.0f, height() - 100.0f);
    m_pBullets.clear();
    m_score = 0;
    m_frame = 0;
    m_state = PLAY;
}

void GameWidget::tick() {
    m_frame++;
    m_player.updateSkills();
    m_player.update(m_keys, width(), height(), m_frame);
    m_player.tryShoot();

    // 子弹移出屏幕即失效
    for (auto& b : m_pBullets) {
        if (!b.alive) continue;
        b.update(m_player.x(), m_player.y());
        if (b.isOutOfScreen(width(), height())) b.alive = false;
    }
    m_pBullets.erase(std::remove_if(m_pBullets.begin(), m_pBullets.end(),
        [](const Bullet& b) { return !b.alive; }), m_pBullets.end());
}

void GameWidget::timerEvent(QTimerEvent*) {
    tick();
    update();
}

void GameWidget::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    m_keys.insert(event->key());
}

void GameWidget::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    m_keys.remove(event->key());
}

void GameWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::black);

    // 子弹
    for (const auto& b : m_pBullets) {
        if (!b.alive) continue;
        p.setBrush(b.color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(b.x, b.y), b.radius, b.radius);
    }
    // 玩家
    m_player.draw(p);

    // UI
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 11, QFont::Bold));
    p.drawText(10, 20, QString("Score: %1   HP: %2").arg(m_score).arg(m_player.hp()));

    // 技能栏
    p.setFont(QFont("Arial", 9));
    auto drawSkill = [&](int x, const QString& name, int key) {
        p.fillRect(x, height() - 50, 60, 30, QColor(50, 50, 50, 200));
        p.setPen(Qt::white);
        p.drawText(QRect(x, height() - 50, 60, 30), Qt::AlignCenter, QString("[%1] %2").arg(key).arg(name));
        };
    drawSkill(20, "Bomb", 1);
    drawSkill(100, "Shield", 2);
    drawSkill(180, "Fire+", 3);
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