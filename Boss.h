#ifndef BOSS_H
#define BOSS_H

#include "GameTypes.h"
#include <functional>
#include <QPainter>

class Boss {
    Enemy m_data;
    QPixmap m_px1;
    QPixmap m_px2;
    float m_radius = 50;  // 基于图片尺寸计算的碰撞半径

    bool m_changing = false;   // 是否正在切换阶段
    int m_changeTimer = 0;     // 切换倒计时

    int m_moveState = 1;       // 随机游走状态：0=移动中, 1=等待中
    float m_moveDist = 0;      // 本次移动剩余距离(像素)
    int m_moveDir = 1;         // 移动方向(-1左, +1右)
    int m_waitTimer = 0;
    bool m_dying = false; 
    int m_dyingTimer = 0; 
    int m_urchinSeq = -1; 
    int m_urchinDelay = 0;
    int m_urchinRounds = 0;
    bool m_trailOn = false; 
    int m_trailTimer = 0; 
    int m_trailFireTimer = 0;
    bool m_dustOn = false;       // 粉尘弹幕模式
    int m_dustTimer = 0;
    int m_dustWaves = 0; 
    bool m_urchinSweep = false;  // 海胆角度扫射模式
    float m_urchinSweepAng = 0; 
    int m_urchinSweepDir = 1; 
    int m_urchinSweepCnt = 0; 
    int m_urchinSweepTimer = 0;  // 扫射发射间隔计时
    int m_nextDust = 0;            // 粉尘弹冷却计数（≥3才触发）
    float m_arrayRotAngle = 0;     // 海胆阵列整体旋转角（弧度）
    bool m_netOn = false;
    int m_netTimer = 0; 
    int m_netFireTimer = 0; 

    std::function<void(const Bullet&)> m_addBullet;     //子弹回调容器
    std::function<void(float, float)> m_onPhaseDrop;      //阶段切换回调（掉落道具）

    void s1(float px, float py); 
    void s2(float px, float py, int frameCnt);
    void s3(float px, float py);

    void fireCircle(float x, float y, int cnt, float spd, QColor c, float r);
    void fireFan(float x, float y, float tx, float ty, int cnt, float spread, float spd, QColor c, float r);
    void fireStraight(float x, float y, float vx, float vy, QColor c, float r);
    void fireSeeking(float x, float y, float vx, float vy, QColor c, float r);
    void fireUrchin(float x, float y, float vx, float vy);
    void fireUrchinRow(int row);
    void fireTrail(float x, float y, float vx, float vy);
    void fireDust();
    void fireCrossBurst();
    void fireVortex();
    void fireCage();
    void fireNet();
    void fireNetRows();
    void fireBulletPair(float sx, float sy);

public:
    Boss();
    void reset(float startX, float startY);
    void update(float px, float py, int frameCnt);
    void draw(QPainter& p) const;

    void setBulletAdder(std::function<void(const Bullet&)> adder);  // 设置子弹回调函数
    void setPhaseDropCb(std::function<void(float, float)> cb);      // 阶段切换道具掉落
    void startDying();                   // 开始死亡闪烁
    bool isDying() const;                // 是否正在闪烁
    bool isDyingDone() const;            // 闪烁是否完成
    Enemy& data() { return m_data; }
    const Enemy& data() const { return m_data; }    //只读返回boss数据
    bool isAlive() const { return m_data.alive; }
    float radius() const { return m_radius; }
};

#endif