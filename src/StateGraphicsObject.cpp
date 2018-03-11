#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"

namespace fsmviz {

StateGraphicsObject::StateGraphicsObject(const QVector2D &pos, std::size_t id)
    : GraphicsObject{pos}
    , m_staring{false}
    , m_final{false}
    , m_id{id}
{
}

void StateGraphicsObject::render(QPainter &p, int pass)
{
    if (pass != 1)
    {
        return;
    }

    static const QColor c_default_color = QColor(255, 255, 100);
    static const QColor c_selected_color = QColor(255, 100, 100);

    QPen pen(Qt::black, 2);
    p.setPen(pen);

    QPainterPath path;
    path.addEllipse(m_pos.toPointF(), getSize(), getSize());

    p.fillPath(path, m_selected ? c_selected_color : c_default_color);
    p.strokePath(path, pen);

    ///@ tmp

    p.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    static constexpr int c_rect_size = 75;

    QPoint center = QPoint(c_rect_size, c_rect_size) / 2;
    QSize size(c_rect_size, c_rect_size);

    p.drawText(
        QRect(m_pos.toPoint() - center, size),
        Qt::AlignCenter,
        QString::number(m_id));

    static constexpr int c_delta_size = 4;

    if (m_staring)
    {
        p.drawEllipse(
            m_pos.toPointF(),
            getSize() + c_delta_size,
            getSize() + c_delta_size);
    }

    if (m_final)
    {
        p.drawEllipse(
            m_pos.toPointF(),
            getSize() - c_delta_size,
            getSize() - c_delta_size);
    }
}

double StateGraphicsObject::getSize() const
{
    return 20;
}

void StateGraphicsObject::toggleStarting()
{
    m_staring = !m_staring;
}

void StateGraphicsObject::toggleFinal()
{
    m_final = !m_final;
}

bool StateGraphicsObject::isStarting() const
{
    return m_staring;
}

bool StateGraphicsObject::isFinal() const
{
    return m_final;
}

void StateGraphicsObject::connect(TransitionGraphicsObjectPtr transition)
{
    m_transitions.emplace_back(transition);
}

void StateGraphicsObject::disconnect(TransitionGraphicsObjectPtr transition)
{
    m_transitions.erase(std::remove(
        std::begin(m_transitions), std::end(m_transitions), transition));
}

std::vector<TransitionGraphicsObjectPtr> StateGraphicsObject::getTransitions()
    const
{
    return m_transitions;
}

bool StateGraphicsObject::getFlag() const
{
    return m_flag;
}

void StateGraphicsObject::setFlag(bool flag)
{
    m_flag = flag;
}

} // namespace fsmviz
