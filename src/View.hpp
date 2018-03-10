#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <QtWidgets/QWidget>
#include "GraphicsObject.hpp"
#include "gcp/GenericCommandProcessor.hpp"
#include "qconsole/QConsole.hpp"

namespace fsmviz {

class View : public QWidget
{
    Q_OBJECT

public: // methods
    View();

    virtual ~View();

    void bind(std::string str, const std::function<void()> &handler);
    void unbind(const std::string &str);

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

    enum class DefaultSymbol
    {
        Epsilon,
        Random,
        Letter,
    };

private: // methods
    void interact(GraphicsObjectPtr a, GraphicsObjectPtr b, bool attract);

    void applyForces();
    void tick();

    void updateConnectedComponents();

    void toggleConsole();

private slots:
    void resizeConsole();

private: // fields
    std::vector<GraphicsObjectPtr> m_objects;
    std::vector<StateGraphicsObjectPtr> m_states;
    std::vector<TransitionGraphicsObjectPtr> m_transitions;

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

    gcp::GenericCommandProcessor m_processor;

    bool m_shortcuts_enabled;

    DefaultSymbol m_default_symbol;
    char m_default_letter;
};

} // namespace fsmviz
