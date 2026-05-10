#ifndef PLAYER_H
#define PLAYER_H

#include "GameTypes.h"
#include <functional>
#include <QSet>
#include <QPainter>
#include <QPixmap>

class Player {
    float m_x = 0, m_y = 0;
    float m_speed = 10.0f;
    int m_hp = 20, m_maxHp = 20;
    int m_shootTimer = 0;      
    int m_shootInterval = 5;    
    int m_invTimer = 0;         
    int m_dmg = 2;

    Skill m_bomb;  // 炸弹技能
    int m_bombStock = 0;  // 炸弹存储数量
    int m_shieldTimer = 0;
    int m_hits = 0;          // 受击次数
    int m_pickupsCollected = 0;  // 道具拾取次数
    Skill m_fire;  // 火力增强效果计时
    std::function<void(const Bullet&)> m_addBullet;     //子弹添加回调函数

    QPixmap m_pixmap; 
    float m_radius = 20; 
public:
    Player();
    void reset(float startX, float startY); 
    void update(const QSet<int>& keys, int w, int h, int frameCnt);  //帧计数
    void draw(QPainter& p) const;   

    void setBulletAdder(std::function<void(const Bullet&)> adder);  //子弹添加回调
    void tryShoot();   

    void useSkill(int id); 
    void updateSkills();

    void hit();         
    void getPickup(int type);    

    float x() const { return m_x; }
    float y() const { return m_y; }
    int hp() const { return m_hp; }
    int maxHp() const { return m_maxHp; }
    int dmg() const { return m_dmg; }
    int invTimer() const { return m_invTimer; }     //无敌剩余帧数
    int radius() const { return (int)m_radius; }
    bool isInv() const { return m_invTimer > 0; }

    Skill& bombSkill() { return m_bomb; }
    Skill& fireSkill() { return m_fire; }
    int bombStock() const { return m_bombStock; }  // 炸弹剩余数量
    int shieldTimer() const { return m_shieldTimer; }
    int hits() const { return m_hits; }
    int pickupsCollected() const { return m_pickupsCollected; }

    void flyOff();
    bool offScreen() const;
};

#endif