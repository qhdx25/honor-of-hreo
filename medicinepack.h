#ifndef MEDICINEPACK_H
#define MEDICINEPACK_H

#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QSize>

class QPainter;

class MedicinePack
{
public:
    MedicinePack();

    void setCenter(const QPointF &center);
    QPointF center() const;
    QRectF boundingRect() const;
    void paint(QPainter &painter) const;
    void spawn();
    void consume();
    bool isActive() const;

private:
    QPointF m_center;
    QPixmap m_pixmap;
    QSize m_size = QSize(116, 116);
    bool m_active = true;
};

#endif // MEDICINEPACK_H
