#include "skilliconwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <algorithm>
#include <cmath>

SkillIconWidget::SkillIconWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(IconSize, IconSize);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);

    m_timer = new QTimer(this);
    m_timer->setInterval(180);
    QObject::connect(m_timer, &QTimer::timeout, this, [this]() { onTimeout(); });
}

void SkillIconWidget::setFrames(const QVector<QString> &resourcePaths)
{
    m_frameResourcePaths = resourcePaths;
    m_frames.clear();
    m_frameIndex = 0;
    m_playing = false;
    m_timer->stop();

    for (const QString &path : m_frameResourcePaths) {
        QPixmap pix(path);
        if (pix.isNull()) {
            continue;
        }

        m_frames.push_back(pix.scaled(IconSize,
                                      IconSize,
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation));
    }

    update();
}

void SkillIconWidget::setCooldownState(qreal remainingMs, qreal totalMs)
{
    m_cooldownRemainingMs = std::max(0.0, remainingMs);
    m_cooldownTotalMs = std::max(0.0, totalMs);
    update();
}

void SkillIconWidget::setClickHandler(std::function<void()> handler)
{
    m_clickHandler = std::move(handler);
}

void SkillIconWidget::setDragStartedHandler(std::function<void()> handler)
{
    m_dragStartedHandler = std::move(handler);
}

void SkillIconWidget::setDragMovedHandler(std::function<void(const QPoint &)> handler)
{
    m_dragMovedHandler = std::move(handler);
}

void SkillIconWidget::setDragReleasedHandler(std::function<void(const QPoint &)> handler)
{
    m_dragReleasedHandler = std::move(handler);
}

void SkillIconWidget::restartPlayback()
{
    if (m_frames.isEmpty()) {
        return;
    }

    m_frameIndex = 0;
    m_playing = true;
    m_timer->start();
    update();
}

void SkillIconWidget::onTimeout()
{
    if (!m_playing || m_frames.isEmpty()) {
        m_timer->stop();
        return;
    }

    if (m_frameIndex >= m_frames.size() - 1) {
        m_timer->stop();
        m_playing = false;
        m_frameIndex = m_frames.size() - 1;
        update();
        return;
    }

    ++m_frameIndex;
    if (m_frameIndex >= m_frames.size() - 1) {
        m_frameIndex = m_frames.size() - 1;
        m_timer->stop();
        m_playing = false;
    }

    update();
}

void SkillIconWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_frames.isEmpty()) {
        return;
    }

    QPainter painter(this);
    const QPixmap &pix = m_frames[m_frameIndex];
    const int drawX = (width() - pix.width()) / 2;
    const int drawY = (height() - pix.height()) / 2;
    const QRect pixRect(drawX, drawY, pix.width(), pix.height());
    painter.drawPixmap(pixRect, pix);

    if (m_cooldownRemainingMs <= 0.0 || m_cooldownTotalMs <= 0.0) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal overlayDiameter = std::max<qreal>(0.0, std::min(pixRect.width(), pixRect.height()) - 10.0);
    const QPointF iconCenter = pixRect.center();
    const QRectF overlayRect(iconCenter.x() - overlayDiameter / 2.0,
                             iconCenter.y() - overlayDiameter / 2.0,
                             overlayDiameter,
                             overlayDiameter);
    const qreal cooldownRatio = std::clamp(m_cooldownRemainingMs / m_cooldownTotalMs, 0.0, 1.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(18, 24, 31, 130));
    painter.drawEllipse(overlayRect);

    painter.setBrush(QColor(10, 14, 18, 180));
    painter.drawPie(overlayRect, 90 * 16, -static_cast<int>(cooldownRatio * 360.0 * 16.0));

    const QRectF ringRect = overlayRect.adjusted(-3.0, -3.0, 3.0, 3.0);
    QPen ringBackgroundPen(QColor(255, 210, 120, 70));
    ringBackgroundPen.setWidth(6);
    painter.setPen(ringBackgroundPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(ringRect);

    QPen ringPen(QColor(255, 198, 92, 230));
    ringPen.setWidth(6);
    ringPen.setCapStyle(Qt::RoundCap);
    painter.setPen(ringPen);
    painter.drawArc(ringRect, 90 * 16, -static_cast<int>(cooldownRatio * 360.0 * 16.0));

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(16);
    painter.setFont(font);
    painter.setPen(QColor(255, 252, 244));
    const qreal seconds = m_cooldownRemainingMs / 1000.0;
    const QString label = seconds >= 1.0
                              ? QString::number(static_cast<int>(std::ceil(seconds)))
                              : QString::number(seconds, 'f', 1);
    painter.drawText(pixRect, Qt::AlignCenter, label);
}

void SkillIconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_pressGlobalPos = event->globalPos();
    m_dragging = true;
    grabMouse();
    if (m_dragStartedHandler) {
        m_dragStartedHandler();
    }
    event->accept();
}

void SkillIconWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (m_dragMovedHandler) {
        m_dragMovedHandler(event->globalPos() - m_pressGlobalPos);
    }
    event->accept();
}

void SkillIconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    const QPoint dragOffset = event->globalPos() - m_pressGlobalPos;
    m_dragging = false;
    releaseMouse();
    restartPlayback();
    if (m_clickHandler && dragOffset.manhattanLength() <= 14) {
        m_clickHandler();
    }
    if (m_dragReleasedHandler) {
        m_dragReleasedHandler(dragOffset);
    }
    event->accept();
}
