#ifndef SKILLICONWIDGET_H
#define SKILLICONWIDGET_H

#include <QPixmap>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QWidget>
#include <functional>
#include <utility>

class QMouseEvent;
class QPaintEvent;
class QTimer;

class SkillIconWidget : public QWidget
{
public:
    static constexpr int IconSize = 128;
    static constexpr int DefaultMargin = 60;

    explicit SkillIconWidget(QWidget *parent = nullptr);
    ~SkillIconWidget() override = default;

    void setFrames(const QVector<QString> &resourcePaths);
    void setCooldownState(qreal remainingMs, qreal totalMs);
    void setClickHandler(std::function<void()> handler);
    void setDragStartedHandler(std::function<void()> handler);
    void setDragMovedHandler(std::function<void(const QPoint &)> handler);
    void setDragReleasedHandler(std::function<void(const QPoint &)> handler);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void onTimeout();
    void restartPlayback();

private:
    QTimer *m_timer = nullptr;
    QVector<QString> m_frameResourcePaths;
    QVector<QPixmap> m_frames;
    int m_frameIndex = 0;
    bool m_playing = false;
    QPoint m_pressGlobalPos;
    bool m_dragging = false;
    qreal m_cooldownRemainingMs = 0.0;
    qreal m_cooldownTotalMs = 0.0;
    std::function<void()> m_clickHandler;
    std::function<void()> m_dragStartedHandler;
    std::function<void(const QPoint &)> m_dragMovedHandler;
    std::function<void(const QPoint &)> m_dragReleasedHandler;
};

#endif // SKILLICONWIDGET_H
