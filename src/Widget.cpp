#include "Widget.hpp"
#include <chrono>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"

namespace fsmviz {

Widget::Widget()
    : m_selected_object{nullptr}
    , m_translating{false}
    , m_moving{false}
    , m_run{false}
    , m_antialias{true}
    , m_scale{1}
{
    QSize screen_size = qApp->primaryScreen()->size();
    QPoint screen_center =
        QPoint(screen_size.width() / 2, screen_size.height() / 2);

    resize(screen_size * 0.75);
    move(screen_center - rect().center());

    setMouseTracking(true);

    startTimer(16);
}

Widget::~Widget()
{
}

void Widget::timerEvent(QTimerEvent *)
{
    applyForces();
    tick();
    repaint();
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if (!(e->button() & Qt::LeftButton) && !(e->button() & Qt::RightButton))
    {
        return;
    }

    if (m_selected_object)
    {
        m_selected_object->deselect();
        m_selected_object = nullptr;
    }

    QVector2D pos(e->pos() - m_translation);

    for (GraphicsObject *obj : m_objects)
    {
        if (obj->contains(pos))
        {
            obj->select();
            m_selected_object = obj;
            break;
        }
    }

    if (e->button() & Qt::LeftButton)
    {
        if (m_selected_object)
        {
            m_moving = true;
        }
        else
        {
            m_translating = true;
        }
    }
    else if (
        e->button() & Qt::RightButton &&
        !m_selected_object->as<TransitionGraphicsObject>())
    {
        StateGraphicsObject *state =
            m_selected_object->as<StateGraphicsObject>();
        if (state)
        {
            m_selected_object->deselect();
            m_selected_object = new TransitionGraphicsObject(state, pos);
            m_selected_object->select();
            m_objects.emplace_back(m_selected_object);

            m_moving = true;
        }
        else
        {
            m_selected_object = new StateGraphicsObject(pos);
            m_selected_object->select();
            m_objects.emplace_back(m_selected_object);
        }
    }

    m_last_pos = e->pos();
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
    TransitionGraphicsObject *transition =
        m_selected_object->as<TransitionGraphicsObject>();

    if (e->button() & Qt::RightButton && m_moving && transition)
    {
        QVector2D pos(e->pos() - m_translation);

        StateGraphicsObject *end = nullptr;

        for (GraphicsObject *obj : m_objects)
        {
            StateGraphicsObject *state = obj->as<StateGraphicsObject>();
            if (state && state->contains(pos))
            {
                end = state;
                break;
            }
        }

        if (m_selected_object)
        {
            m_selected_object->deselect();
            m_selected_object = nullptr;
        }

        transition->select();
        m_selected_object = transition;

        if (!end)
        {
            end = new StateGraphicsObject(pos);
            transition->deselect();
            end->select();
            m_selected_object = end;
            m_objects.emplace_back(end);
        }

        transition->setEnd(end);
    }

    m_translating = false;
    m_moving = false;
}

void Widget::mouseMoveEvent(QMouseEvent *e)
{
    QPointF delta = e->pos() - m_last_pos;

    if (m_translating)
    {
        m_translation += delta;
    }
    else if (m_moving)
    {
        m_selected_object->move(QVector2D(delta));
    }

    m_last_pos = e->pos();
}

void Widget::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0)
    {
        m_scale *= 1.1;
    }
    else
    {
        m_scale /= 1.1;
    }
}

void Widget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_F11:
        isFullScreen() ? showNormal() : showFullScreen();
        break;
    case Qt::Key_Escape:
        isFullScreen() ? showNormal() : qApp->quit();
        break;

    case Qt::Key_BracketLeft:
    {
        StateGraphicsObject *state =
            m_selected_object->as<StateGraphicsObject>();
        if (state)
        {
            state->toggleStarting();
        }
        break;
    }
    case Qt::Key_BracketRight:
    {
        StateGraphicsObject *state =
            m_selected_object->as<StateGraphicsObject>();
        if (state)
        {
            state->toggleFinal();
        }
        break;
    }

    case Qt::Key_Backspace:
        m_translation = QPointF();
        m_scale = 1;
        break;

    case Qt::Key_Space:
        m_run = !m_run;
        break;

    case Qt::Key_A:
        m_antialias = !m_antialias;
        break;

    case Qt::Key_Delete:
        break;
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::lightGray);

    p.translate(m_translation);

    if (m_antialias)
    {
        p.setRenderHint(QPainter::Antialiasing);
    }

    static constexpr int c_num_passes = 3;
    for (int pass = 0; pass < c_num_passes; pass++)
    {
        for (GraphicsObject *obj : m_objects)
        {
            obj->render(p, pass);
        }
    }
}

void Widget::interact(GraphicsObject *a, GraphicsObject *b, bool attract)
{
    static constexpr float c_edge_length = 25;
    const float anti_gravity = 100 * m_scale;

    QVector2D v(a->getPos() - b->getPos());
    float dist = v.length();
    v.normalize();

    float power;
    if (attract)
    {
        power = std::abs(dist / c_edge_length - 1);
    }
    else
    {
        power = anti_gravity / (dist > 0 ? dist : 1);
    }

    QVector2D force;

    if (attract)
    {
        force = v * power;

        if (dist < c_edge_length)
        {
            force *= -1;
        }
    }
    else
    {
        static constexpr float c_max_repulsion = 10;
        force = -v * std::min(power, c_max_repulsion);
    }

    if (!a->isSelected() || !m_moving)
    {
        a->applyForce(-force);
    }

    if (!b->isSelected() || !m_moving)
    {
        b->applyForce(force);
    }
}

void Widget::applyForces()
{
    if (!m_run)
    {
        return;
    }

    for (GraphicsObject *a : m_objects)
    {
        TransitionGraphicsObject *a_tr = a->as<TransitionGraphicsObject>();

        if (a_tr && a_tr->getEnd())
        {
            interact(a_tr, a_tr->getStart(), true);
            interact(a_tr, a_tr->getEnd(), true);
        }

        for (GraphicsObject *b : m_objects)
        {
            TransitionGraphicsObject *b_tr = b->as<TransitionGraphicsObject>();

            if (a >= b || (a_tr && !a_tr->getEnd()) ||
                (b_tr && !b_tr->getEnd()))
            {
                continue;
            }

            interact(b, a, false);
        }
    }
}

void Widget::tick()
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<float>;

    static TimePoint time;

    for (GraphicsObject *obj : m_objects)
    {
        obj->tick(Duration(Clock::now() - time).count());
    }

    time = Clock::now();
}

} // namespace fsmviz
