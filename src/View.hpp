#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <QtWidgets/QWidget>
#include "Controller.hpp"
#include "GraphicsObject.hpp"
#include "gcp/GenericCommandProcessor.hpp"
#include "qconsole/QConsole.hpp"

namespace fsmviz {

class View : public QWidget
{
    Q_OBJECT

public: // methods
    explicit View(
        gcp::GenericCommandProcessor &processor,
        Controller &controller);
    virtual ~View();

    void bind(std::string str, const std::function<void()> &handler);
    void unbind(const std::string &str);

    void renderImage(const std::string &file_name);

    ///@todo Reoreder methods
    GraphicsObjectPtr getSelectedObject() const;
    void deselect();

    void disableShortcuts();

    qconsole::QConsole &getConsole();

    void toggleFullscreen();
    void toggleAntialiasing();
    void toggleRun();

    void run();
    void stop();

    void reset();

    void edit();

protected: // methods
    void timerEvent(QTimerEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
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
    void render(QPainter &p, const QRect &rect, const QPointF &translation);

    void interact(GraphicsObjectPtr a, GraphicsObjectPtr b, bool attract);

    void applyForces();
    void tick();

    void toggleConsole();

private slots:
    void resizeConsole();

private: // fields
    gcp::GenericCommandProcessor &m_processor;
    Controller &m_controller;

    GraphicsObjectPtr m_selected_object;

    bool m_translating;
    bool m_moving;
    bool m_run;
    bool m_antialias;

    QPoint m_last_pos;
    QPointF m_translation;
    float m_scale;

    TimePoint m_time;

    qconsole::QConsole m_console;
    bool m_console_visible;

    std::map<QKeySequence, std::pair<QAction *, QMetaObject::Connection>>
        m_actions;

    bool m_editing;

    QVector2D m_min, m_max;
};

} // namespace fsmviz
