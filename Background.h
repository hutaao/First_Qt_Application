#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QPixmap>

class Background {
public:
    Background();
    void load(const QString& path);
    void update(float speed, int screenH);
    void draw(QPainter& p, int screenW, int screenH) const;

private:
    QPixmap m_px;
    float m_y1 = 0;
    float m_y2 = 0;
};

#endif