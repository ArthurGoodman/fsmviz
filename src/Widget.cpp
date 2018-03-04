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

    case Qt::Key_Delete:
        break;
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::lightGray);

    p.translate(m_translation);

    p.setRenderHint(QPainter::Antialiasing);

    ////////////////////////////////////////////////////////////////////////////

    ///@ do this separately form drawing

    for (GraphicsObject *obj1 : m_objects)
    {
        TransitionGraphicsObject *tr = obj1->as<TransitionGraphicsObject>();

        if (tr && tr->getEnd())
        {
        }

        for (GraphicsObject *obj2 : m_objects)
        {
            if (obj1 == obj2)
            {
                continue;
            }

            ///@ distance function
            double dx = obj1->getPos().x() - obj2->getPos().x();
            double dy = obj1->getPos().y() - obj2->getPos().y();
            double dist = std::sqrt(dx * dx + dy * dy);

            QVector2D v(obj2->getPos() - obj1->getPos());
            v.normalize();

            QPointF force = (v / dist / dist * 10).toPointF();

            if (!m_moving || obj2 != m_selected_object)
            {
                obj2->applyForce(force);
            }

            if (!m_moving || obj1 != m_selected_object)
            {
                obj1->applyForce(-force);
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
