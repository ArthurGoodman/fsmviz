#pragma once

#include <chrono>
#include <QtWidgets/QWidget>
#include "GraphicsObject.hpp"

namespace fsmviz {

class View : public QWidget
{
    Q_OBJECT

public: // methods
    View();

    virtual ~View();

protected: // methods
    void timerEvent(QTimerEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private: // types
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<float>;

private: // methods
    void interact(GraphicsObjectPtr a, GraphicsObjectPtr b, bool attract);

    void applyForces();
    void tick();

private: // fields
    std::vector<GraphicsObjectPtr> m_objects;
    GraphicsObjectPtr m_selected_object;

    bool m_translating;
    bool m_moving;
    bool m_run;
    bool m_antialias;

    QPoint m_last_pos;
    QPointF m_translation;
    float m_scale;

    TimePoint m_time;
};

} // namespace fsmviz
