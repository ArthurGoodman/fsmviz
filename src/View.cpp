#include "View.hpp"
#include <algorithm>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"

namespace fsmviz {

View::View()
    : m_selected_object{nullptr}
    , m_translating{false}
    , m_moving{false}
    , m_run{false}
    , m_antialias{true}
    , m_scale{1}
    , m_time{Clock::now()}
{
    QSize screen_size = qApp->primaryScreen()->size();
    QPoint screen_center =
        QPoint(screen_size.width() / 2, screen_size.height() / 2);

    resize(screen_size * 0.75);
    move(screen_center - rect().center());

    setMouseTracking(true);

    startTimer(16);
}

View::~View()
{
}

void View::timerEvent(QTimerEvent *)
{
    applyForces();
    tick();
    repaint();
}

void View::mousePressEvent(QMouseEvent *e)
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

    for (GraphicsObjectPtr obj : m_objects)
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
        !cast<TransitionGraphicsObject>(m_selected_object))
    {
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_selected_object);
        if (state)
        {
            m_selected_object->deselect();
            TransitionGraphicsObjectPtr transition{
                new TransitionGraphicsObject(state, pos)};
            m_selected_object = transition;
            state->connect(transition);
            m_selected_object->select();
            m_objects.emplace_back(transition);
            m_transitions.emplace_back(transition);
            updateConnectedComponents();

            m_moving = true;
        }
        else
        {
            StateGraphicsObjectPtr state{new StateGraphicsObject(pos)};
            m_selected_object = state;
            m_selected_object->select();
            m_objects.emplace_back(state);
            m_states.emplace_back(state);
            updateConnectedComponents();
        }
    }

    m_last_pos = e->pos();
}

void View::mouseReleaseEvent(QMouseEvent *e)
{
    TransitionGraphicsObjectPtr transition =
        cast<TransitionGraphicsObject>(m_selected_object);

    if (e->button() & Qt::RightButton && m_moving && transition)
    {
        QVector2D pos(e->pos() - m_translation);

        StateGraphicsObjectPtr end = nullptr;

        for (StateGraphicsObjectPtr state : m_states)
        {
            if (state->contains(pos))
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
            end.reset(new StateGraphicsObject(pos));
            transition->deselect();
            end->select();
            m_selected_object = end;
            m_objects.emplace_back(end);
            m_states.emplace_back(end);
        }

        transition->setEnd(end);
        end->connect(transition);

        updateConnectedComponents();
    }

    m_translating = false;
    m_moving = false;
}

void View::mouseMoveEvent(QMouseEvent *e)
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

void View::wheelEvent(QWheelEvent *e)
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

void View::keyPressEvent(QKeyEvent *e)
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
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_selected_object);
        if (state)
        {
            state->toggleStarting();
        }
        break;
    }
    case Qt::Key_BracketRight:
    {
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_selected_object);
        if (state)
        {
            state->toggleFinal();
        }
        break;
    }

    case Qt::Key_Backspace:
        m_translation = QPointF();
        m_scale = 1;
        m_objects.clear();
        m_states.clear();
        m_transitions.clear();
        break;

    case Qt::Key_Space:
        m_run = !m_run;
        break;

    case Qt::Key_A:
        m_antialias = !m_antialias;
        break;

    case Qt::Key_Delete:
        if (m_selected_object)
        {
            StateGraphicsObjectPtr state =
                cast<StateGraphicsObject>(m_selected_object);
            if (state)
            {
                m_objects.erase(std::remove(
                    std::begin(m_objects), std::end(m_objects), state));
                m_states.erase(std::remove(
                    std::begin(m_states), std::end(m_states), state));

                auto transitions = state->getTransitions();

                for (TransitionGraphicsObjectPtr tr : transitions)
                {
                    tr->getStart()->disconnect(tr);

                    if (tr->getEnd() != tr->getStart())
                    {
                        tr->getEnd()->disconnect(tr);
                    }
                }

                m_objects.erase(
                    std::remove_if(
                        std::begin(m_objects),
                        std::end(m_objects),
                        [&](GraphicsObjectPtr obj) {
                            return std::find(
                                       std::begin(transitions),
                                       std::end(transitions),
                                       obj) != transitions.end();
                        }),
                    std::end(m_objects));

                m_transitions.erase(
                    std::remove_if(
                        std::begin(m_transitions),
                        std::end(m_transitions),
                        [&](TransitionGraphicsObjectPtr obj) {
                            return std::find(
                                       std::begin(transitions),
                                       std::end(transitions),
                                       obj) != transitions.end();
                        }),
                    std::end(m_transitions));
            }
            else
            {
                TransitionGraphicsObjectPtr transition =
                    cast<TransitionGraphicsObject>(m_selected_object);
                if (transition && transition->getEnd())
                {
                    transition->getStart()->disconnect(transition);
                    transition->getEnd()->disconnect(transition);

                    m_objects.erase(std::remove(
                        std::begin(m_objects),
                        std::end(m_objects),
                        m_selected_object));
                    m_transitions.erase(std::remove(
                        std::begin(m_transitions),
                        std::end(m_transitions),
                        transition));
                }
            }

            m_selected_object = nullptr;
            updateConnectedComponents();
        }
        break;
    }
}

void View::paintEvent(QPaintEvent *)
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
        for (GraphicsObjectPtr obj : m_objects)
        {
            obj->render(p, pass);
        }
    }
}

void View::interact(GraphicsObjectPtr a, GraphicsObjectPtr b, bool attract)
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

void View::applyForces()
{
    if (!m_run)
    {
        return;
    }

    for (GraphicsObjectPtr a : m_objects)
    {
        TransitionGraphicsObjectPtr a_tr = cast<TransitionGraphicsObject>(a);

        if (a_tr && a_tr->getEnd())
        {
            interact(a_tr, a_tr->getStart(), true);
            interact(a_tr, a_tr->getEnd(), true);
        }

        for (GraphicsObjectPtr b : m_objects)
        {
            TransitionGraphicsObjectPtr b_tr =
                cast<TransitionGraphicsObject>(b);

            if (a >= b || (a_tr && !a_tr->getEnd()) ||
                (b_tr && !b_tr->getEnd()))
            {
                continue;
            }

            if (a->getTag() == b->getTag())
            {
                interact(b, a, false);
            }
        }
    }
}

void View::tick()
{
    for (GraphicsObjectPtr obj : m_objects)
    {
        obj->tick(Duration(Clock::now() - m_time).count());
    }

    m_time = Clock::now();
}

void visitState(StateGraphicsObjectPtr state, int tag)
{
    if (state->getFlag())
    {
        return;
    }

    state->setFlag(true);
    state->setTag(tag);

    auto transitions = state->getTransitions();

    for (TransitionGraphicsObjectPtr transition : transitions)
    {
        transition->setTag(tag);

        if (transition->getStart() != state)
        {
            visitState(transition->getStart(), tag);
        }
        else if (transition->getEnd())
        {
            visitState(transition->getEnd(), tag);
        }
    }
}

void View::updateConnectedComponents()
{
    for (StateGraphicsObjectPtr state : m_states)
    {
        state->setFlag(false);
    }

    int tag = 0;

    for (StateGraphicsObjectPtr state : m_states)
    {
        visitState(state, tag++);
    }
}

} // namespace fsmviz
