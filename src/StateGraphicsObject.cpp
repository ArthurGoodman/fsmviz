#include "StateGraphicsObject.hpp"

namespace fsmviz {

StateGraphicsObject::StateGraphicsObject(const QVector2D &pos)
    : GraphicsObject{pos}
    , m_staring{false}
    , m_final{false}
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

} // namespace fsmviz
