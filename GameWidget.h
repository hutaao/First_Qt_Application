#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QBasicTimer>
#include <QSet>
#include <QList>
#include <random>
#include "GameTypes.h"
#include "Player.h"

class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void timerEvent(QTimerEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

private:
    void gameReset();
    void tick();    //逻辑更新函数

    float rf(float min, float max);     //辅助生成随机数
    int ri(int min, int max);
    float dist(float x1, float y1, float x2, float y2);

    QBasicTimer m_timer;
    GameState m_state = MENU;
    Player m_player;
    QList<Bullet> m_pBullets;
    QSet<int> m_keys;

    int m_score = 0;
    int m_frame = 0;

    std::mt19937 m_rng;
};

#endif