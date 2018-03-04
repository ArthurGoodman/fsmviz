#include "StateGraphicsObject.hpp"

namespace fsmviz {

StateGraphicsObject::StateGraphicsObject(const QPointF &pos)
    : GraphicsObject{pos}
    , m_staring{false}
    , m_final{false}
{
    ///@ improve size mechanism
    m_size = 20;
}

void StateGraphicsObject::render(QPainter &p, int pass)
{
    if (pass != 1)
    {
        return;
    }

    ///@ organize colors
    static const QColor c_default_color = QColor(255, 255, 100);
    static const QColor c_selected_color = QColor(255, 100, 100);

    static constexpr int c_delta_size = 4;

    QPen pen(Qt::black, 1.5);
    p.setPen(pen);

    QPainterPath path;
    path.addEllipse(m_pos, m_size, m_size);

    p.fillPath(path, m_selected ? c_selected_color : c_default_color);
    p.strokePath(path, pen);

    if (m_staring)
    {
        p.drawEllipse(m_pos, m_size + c_delta_size, m_size + c_delta_size);
    }

    if (m_final)
    {
        p.drawEllipse(m_pos, m_size - c_delta_size, m_size - c_delta_size);
    }
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
