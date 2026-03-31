#include "medicinepack.h"

#include "assetpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QString>

namespace {
QString assetPath(const QString &fileName)
{
    const QString packagedPath = QDir(QCoreApplication::applicationDirPath()).filePath("res/" + fileName);
    if (QFileInfo::exists(packagedPath)) {
        return packagedPath;
    }

    return QString::fromUtf8(kSourceAssetDir) + "/" + fileName;
}
}

MedicinePack::MedicinePack()
    : m_pixmap(assetPath("medicine.png"))
{
}

void MedicinePack::setCenter(const QPointF &center)
{
    m_center = center;
}

QPointF MedicinePack::center() const
{
    return m_center;
}

QRectF MedicinePack::boundingRect() const
{
    return QRectF(m_center.x() - m_size.width() / 2.0,
                  m_center.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

void MedicinePack::paint(QPainter &painter) const
{
    if (!m_active) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bodyRect = boundingRect();
    if (!m_pixmap.isNull()) {
        painter.drawPixmap(bodyRect.toRect(), m_pixmap);
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(232, 245, 232, 230));
        painter.drawRoundedRect(bodyRect, 18, 18);
        painter.setBrush(QColor(206, 48, 48, 235));
        painter.drawRect(QRectF(bodyRect.center().x() - 10.0, bodyRect.top() + 18.0, 20.0, bodyRect.height() - 36.0));
        painter.drawRect(QRectF(bodyRect.left() + 18.0, bodyRect.center().y() - 10.0, bodyRect.width() - 36.0, 20.0));
    }

    painter.restore();
}

void MedicinePack::spawn()
{
    m_active = true;
}

void MedicinePack::consume()
{
    m_active = false;
}

bool MedicinePack::isActive() const
{
    return m_active;
}
