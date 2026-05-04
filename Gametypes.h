#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <QColor>
#include <QtMath>

struct Bullet {
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    float radius = 4;
    QColor color = Qt::yellow;
    int type = 0;       //0:玩家, 1:普通直线, 2:追踪, 3:海胆, 4:回旋镖
    int damage = 1;
    bool alive = true;  
    float turnSpeed = 0.05f;    
    float growRate = 0; 
    int trackDur = 0;  

    void update(float targetX, float targetY) {         
        if (type == 2) {
            if (trackDur > 0) {
                trackDur--;
                float ta = atan2(targetY - y, targetX - x);
                float ca = atan2(vy, vx); 
                float diff = ta - ca;
                while (diff > static_cast<float>(M_PI)) diff -= 2.0f * static_cast<float>(M_PI);
                while (diff < -static_cast<float>(M_PI)) diff += 2.0f * static_cast<float>(M_PI);   
                float turn = qBound(-turnSpeed, diff, turnSpeed);   //限制函数
                float newA = ca + turn;
                float spd = sqrt(vx * vx + vy * vy);
                vx = cos(newA) * spd;
                vy = sin(newA) * spd; 
            }
            else {
                type = 1;
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
        x += vx;
        y += vy;
    }

    bool isOutOfScreen(int width, int height) const {
        return (x < -10 || x > width + 10 || y < -10 || y > height + 10);
    }
};

struct Enemy {
    float x = 0, y = 0;
    int hp = 3, maxHp = 3;  //后续调整数值及区分类型
    int type = 0;
    float baseX = 0;        //摇摆敌人初始X位置
    float moveTimer = 0;    //微小浮动
    float moveAmp = 60;     
    int shootTimer = 0;    
    int shootInterval = 40;     
    float angle = 0;            
    float angleStep = 0.15f;    
    bool alive = true;
    int stage = 0;             
    int stgTimer = 0;          
    int patIdx = 0;             //弹幕模式索引
};

struct Pickup {
    float x = 0, y = 0;
    int type = 0;
    int life = 300;     //后续调整数值
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

enum GameState { MENU, PLAY, PAUSED, BREAK, BOSS_WARN, OVER }; 

#endif