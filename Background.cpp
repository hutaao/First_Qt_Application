#include "Background.h"
#include <QPainter>

Background::Background() {}

void Background::load(const QString& path) {
    m_px.load(path);
    m_y1 = 0;
    m_y2 = -m_px.height();
}

void Background::update(float speed, int screenH) {
    m_y1 += speed;
    m_y2 += speed;

    if (m_y1 >= screenH) m_y1 = m_y2 - m_px.height();
    if (m_y2 >= screenH) m_y2 = m_y1 - m_px.height();
}

void Background::draw(QPainter& p, int screenW, int screenH) const {
    if (m_px.isNull()) return;

    if (m_y1 < screenH && m_y1 > -m_px.height())
        p.drawPixmap(0, (int)m_y1, m_px);
    if (m_y2 < screenH && m_y2 > -m_px.height())
        p.drawPixmap(0, (int)m_y2, m_px);
}