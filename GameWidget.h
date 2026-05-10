#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QBasicTimer>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSet>
#include <QList>
#include <random>
#include <QMediaPlayer>
#include "GameTypes.h"
#include "Player.h"
#include "EnemyUnit.h"
#include "Boss.h"
#include "Background.h"
#include "BgmManager.h"

class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void timerEvent(QTimerEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void gameReset();
    void startWave();
    void spawnBatch();    // 生成当前波次的一个子批次
    void tick();    //逻辑更新函数
    void checkWave();   //检查敌人是否全灭
    void genPickup(float x, float y);
    void genParticles(float x, float y, int cnt, QColor col);
    void dropPhaseItems(float x, float y);  // Boss阶段切换掉落3种道具

    float rf(float min, float max);
    int ri(int min, int max);
    float dist(float x1, float y1, float x2, float y2);

    QBasicTimer m_timer;
    GameState m_state = MENU;   //初始状态设为菜单
    GameState m_prePauseState = MENU;  //记录暂停前的状态，用于恢复
    Player m_player;
    QList<EnemyUnit> m_enemies;
    Boss m_boss;
    Background m_bg;
    BgmManager m_bgm;
    QPixmap m_pxMenuBg;        
    QPixmap m_pxGameName;      
    QPixmap m_pxUi1, m_pxUi2, m_pxUi3;  
    QRect m_btnStart;   
    QRect m_btnHelp;    
    QRect m_btnMusic;
    QRect m_btnResume;
    QRect m_btnPauseMusic;
    QRect m_btnMenu;
    QRect m_btnRetry;
    QRect m_btnHome;
    bool m_muted = false;
    float m_bgSpeed = 3.0f;     // 滚动屏幕速度
    QPixmap m_pxSkBomb, m_pxSkShield, m_pxSkFire;
    QPixmap m_pxBulletPlayer;
    QPixmap m_pxBulletEne[4];
    QPixmap m_pxBulletTrack;
    QPixmap m_pxBulletBoss[4];
    QPixmap m_pxBulletSide;
    QPixmap m_pxSideScaled;
    QPixmap m_pxUrchin;
    QPixmap m_pxTrail;
    static constexpr int MAX_PB = 400;
    static constexpr int MAX_EB = 800;
    Bullet m_pBullets[MAX_PB];
    bool m_pBulletActive[MAX_PB] = {};
    Bullet m_eBullets[MAX_EB];
    bool m_eBulletActive[MAX_EB] = {};
    void addPlayerBullet(const Bullet& b) {
        for (int i = 0; i < MAX_PB; i++) {
            if (!m_pBulletActive[i]) { m_pBullets[i] = b; m_pBulletActive[i] = true; return; }
        }
    }
    void addEnemyBullet(const Bullet& b) {
        for (int i = 0; i < MAX_EB; i++) {
            if (!m_eBulletActive[i]) { m_eBullets[i] = b; m_eBulletActive[i] = true; return; }
        }
    }
    QList<Pickup> m_pickups;
    QList<Particle> m_pts;
    QSet<int> m_keys;       //按下键的集合

    int m_wave = 0;
    const int m_totalWaves = 5;
    int m_remain = 0;
    int m_totalEnemies = 0;   // 本波敌人总数
    int m_totBatches = 1;    // 当前波次的子波次数
    int m_batchCap = 4;
    int m_curBatch = 0;
    int m_spawnedSoFar = 0;  // 本波已生成的敌人数
    bool m_bossOn = false;
    bool m_victory = false;   // 结算类型：true=胜利, false=失败
    int m_breakTimer = 0;   //波间倒计时

    int m_warnTimer = 0;         // Boss 预警倒计时
    const int m_warnLen = 180;
    bool m_newWarn = true; 
    QMediaPlayer* m_sfxPlayer; 
    int m_score = 0;
    int m_frame = 0;

    std::mt19937 m_rng;
};

#endif