#include "Widget.hpp"
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"

#include <iostream>

namespace fsmviz {

Widget::Widget()
    : m_selected_object{nullptr}
    , m_translating{false}
    , m_moving{false}
    , m_run{false}
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

    QPointF pos = e->pos() - m_translation;

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
        QPointF pos = e->pos() - m_translation;

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
        m_selected_object->move(delta);
    }

    m_last_pos = e->pos();
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
        break;

    case Qt::Key_Space:
        m_run = !m_run;
        break;

    case Qt::Key_Delete:
        break;
    }
}

void interact(
    GraphicsObject *obj,
    const QPointF &p,
    ///@ add more arguments
    bool moving,
    bool attract,
    bool edge,
    bool special = false)
{
    static constexpr double edge_length = 150;
    static constexpr double anti_gravity = 50;

    if (obj->isSelected() && moving)
    {
        return;
    }

    QVector2D v(p - obj->getPos());
    v.normalize();

    double dx = obj->getPos().x() - p.x();
    double dy = obj->getPos().y() - p.y();
    double dist = std::sqrt(dx * dx + dy * dy);

    double power;
    if (special)
    {
        power = std::max(0.0, std::log(dist + 1));
    }
    else
    {
        if (edge)
        {
            power = std::abs(dist / edge_length - 1);
        }
        else
        {
            power = anti_gravity / (dist ? dist : 1);
        }
    }

    QPointF force = (v * power).toPointF();

    if (!attract)
    {
        force *= -1;
    }

    if (edge)
    {
        if (dist > edge_length)
        {
            obj->applyForce(force);
        }
        else
        {
            obj->applyForce(-force);
        }
    }
    else
    {
        obj->applyForce(force);
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::lightGray);

    p.translate(m_translation);

    p.setRenderHint(QPainter::Antialiasing);

    ////////////////////////////////////////////////////////////////////////////

    ///@ do this separately from drawing

    if (m_run)
    {
        for (GraphicsObject *a : m_objects)
        {
            TransitionGraphicsObject *tr = a->as<TransitionGraphicsObject>();

            if (tr && tr->getEnd())
            {
                QPointF center =
                    (tr->getStart()->getPos() + tr->getEnd()->getPos()) / 2;
                interact(tr, center, m_moving, true, false, true);

                interact(
                    tr->getStart(),
                    tr->getEnd()->getPos(),
                    m_moving,
                    true,
                    true);
                interact(
                    tr->getEnd(),
                    tr->getStart()->getPos(),
                    m_moving,
                    true,
                    true);
            }

            for (GraphicsObject *b : m_objects)
            {
                TransitionGraphicsObject *tr2 =
                    b->as<TransitionGraphicsObject>();

                if (a == b || tr || tr2)
                {
                    continue;
                }

                interact(a, b->getPos(), m_moving, false, false);
                interact(b, a->getPos(), m_moving, false, false);
            }
        }
    }

    for (GraphicsObject *obj : m_objects)
    {
        obj->tick();
    }

    ////////////////////////////////////////////////////////////////////////////

    for (int pass = 0; pass < 3; pass++)
    {
        for (GraphicsObject *obj : m_objects)
        {
            obj->render(p, pass);
        }
    }
}

} // namespace fsmviz
