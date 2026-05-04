#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QBasicTimer>
#include <QSet>
#include <QList>
#include <random>
#include "GameTypes.h"
#include "Player.h"
#include "EnemyUnit.h"
#include "Boss.h"

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
    void startWave();
    void tick();    //逻辑更新函数
    void checkWave();   //检查敌人是否全灭
    void genPickup(float x, float y);
    void genParticles(float x, float y, int cnt, QColor col);

    float rf(float min, float max);
    int ri(int min, int max);
    float dist(float x1, float y1, float x2, float y2);

    QBasicTimer m_timer;
    GameState m_state = MENU;   //初始状态设为菜单
    Player m_player;
    QList<EnemyUnit> m_enemies;
    Boss m_boss;
    QList<Bullet> m_pBullets;
    QList<Bullet> m_eBullets;
    QList<Pickup> m_pickups;
    QList<Particle> m_pts;
    QSet<int> m_keys;       //按下键的集合

    int m_wave = 0;
    const int m_totalWaves = 5;
    int m_remain = 0;
    bool m_bossOn = false;
    int m_breakTimer = 0;   //波间倒计时
    const int m_breakLen = 120;
    int m_warnTimer = 0;         // Boss 预警倒计时
    const int m_warnLen = 180;   // Boss 预警时长（180帧）
    int m_score = 0;
    int m_frame = 0;

    std::mt19937 m_rng;
};

#endif