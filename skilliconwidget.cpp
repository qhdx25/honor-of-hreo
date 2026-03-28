#include "skilliconwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

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
    painter.drawPixmap(drawX, drawY, pix);
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
    if (m_dragReleasedHandler) {
        m_dragReleasedHandler(dragOffset);
    }
    event->accept();
}
