#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <QColor>
#include <QtMath>
#include <QImage>
#include <QPixmap>

struct Bullet {
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    float radius = 4;
    QColor color = Qt::yellow;
    int type = 0;       //0:玩家, 1:普通直线, 2:追踪, 3:海胆, 4:回旋镖, 5:斜向弹, 6:尖刺海胆, 7:Boss尾弹, 8:粉尘弹
    bool isBoss = false; // 是否来自Boss
    int damage = 1;
    bool alive = true;  
    float turnSpeed = 0.05f;    
    float growRate = 0; 
    int trackDur = 0;  

    void update(float targetX, float targetY) {         
        if (type <= 1 || type == 5 || type >= 7) {
        }
        else if (type == 2) {
            if (trackDur > 0) {
                trackDur--;
                float ta = atan2(targetY - y, targetX - x);
                float ca = atan2(vy, vx); 
                float diff = ta - ca;
                while (diff > static_cast<float>(M_PI)) diff -= 2.0f * static_cast<float>(M_PI);
                while (diff < -static_cast<float>(M_PI)) diff += 2.0f * static_cast<float>(M_PI);   
                float turn = qBound(-turnSpeed, diff, turnSpeed);
                float newA = ca + turn;
                float spd = sqrt(vx * vx + vy * vy);
                vx = cos(newA) * spd;
                vy = sin(newA) * spd; 
            }
            else {
            }
        }
        else if (type == 3) {   
            radius += growRate;
            if (radius > 18.0f) radius = 18.0f;
        }
        else if (type == 4) {   
            float ca = atan2(vy, vx);
            ca += turnSpeed;   
            float spd = sqrt(vx * vx + vy * vy);
            vx = cos(ca) * spd;
            vy = sin(ca) * spd;
        }
        else if (type == 6) {
            if (trackDur > 0) {
                trackDur--;
                radius += growRate;
            }
        }
        x += vx;
        y += vy;
    }

    bool isOutOfScreen(int width, int height) const {
        return (x < -10 || x > width + 10 || y < -10 || y > height + 10);
    }
};

struct Enemy {
    float x = 0, y = 0;
    int hp = 3, maxHp = 3;
    int type = 0;
    float baseX = 0;        //刷新X位置
    float baseY = 0;        //Y浮动基准位置
    float moveTimer = 0;    //微小浮动计时
    float floatPhase = 0;   //浮动随机相位
    float moveAmp = 60;     
    int shootTimer = 0;    
    int shootInterval = 40;
    int attackPhase = 0;    // 0=攻击期, 1=休息期
    int phaseTimer = 0;     // 当前阶段剩余帧数（60帧=1秒）     
    float angle = 0;            
    float angleStep = 0.15f;    
    bool alive = true;
    int stage = 0;             
    int stgTimer = 0;          
    int patIdx = 0;             //弹幕模式索引
    int randMoveTimer = 0;      //随机运动方向切换倒计时
    float randVX = 0, randVY = 0;  //当前随机漂移速度
};

struct Pickup {
    float x = 0, y = 0;
    float vx = 0, vy = 0;  // 飘动速度
    int type = 0;
    bool alive = true;
};

struct Particle {      
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    int life = 20, maxLife = 20;    //后续调整数值
    float radius = 3;
    QColor color = Qt::white;
    bool alive = true;
};

struct Skill {         //1:清屏,2:护盾,3:火力增强
    int maxCD = 0;
    int cd = 0;
    int active = 0;     
};

enum GameState { MENU, PLAY, PAUSED, BREAK, BOSS_WARN, VICTORY_FLY, OVER }; 

inline float contentRadius(const QPixmap& px) {         // 通过Alpha通道扫描图片真实内容边界，计算碰撞半径（外接圆）
    if (px.isNull()) return 20.0f;
    QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = img.width(), minY = img.height();
    int maxX = 0, maxY = 0;
    bool hasContent = false;
    for (int y = 0; y < img.height(); ++y) {
        const QRgb* row = (const QRgb*)img.constScanLine(y);
        for (int x = 0; x < img.width(); ++x) {
            if (qAlpha(row[x]) > 64) {
                hasContent = true;
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }
    if (!hasContent) return 20.0f;
    float hw = (maxX - minX) / 2.0f;
    float hh = (maxY - minY) / 2.0f;
    return sqrtf(hw * hw + hh * hh);
}

#endif