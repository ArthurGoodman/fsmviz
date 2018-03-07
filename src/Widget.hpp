#pragma once

#include <QtWidgets/QWidget>
#include "GraphicsObject.hpp"

namespace fsmviz {

class Widget : public QWidget
{
    Q_OBJECT

public: // methods
    Widget();

    virtual ~Widget();

protected: // methods
    void timerEvent(QTimerEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private: // fields
    std::vector<GraphicsObject *> m_objects;
    GraphicsObject *m_selected_object;

    bool m_translating;
    bool m_moving;
    bool m_run;

    QPoint m_last_pos;
    QPointF m_translation;
    double m_scale;
};

} // namespace fsmviz
