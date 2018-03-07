#include "TransitionGraphicsObject.hpp"
#include <cmath>
#include <QtGui/QtGui>

namespace fsmviz {

TransitionGraphicsObject::TransitionGraphicsObject(
    StateGraphicsObject *start,
    const QPointF &pos)
    : GraphicsObject{pos}
    , m_start{start}
    , m_end{nullptr}
{
    ///@ improve size mechanism
    m_size = 10;
}

void TransitionGraphicsObject::render(QPainter &p, int pass)
{
    ///@ organize colors
    static const QColor c_default_color = QColor(100, 220, 100);
    static const QColor c_selected_color = QColor(255, 100, 100);

    QPen pen(Qt::black, 2);
    p.setPen(pen);

    if (pass == 0)
    {
        if (m_end)
        {
            ///@ mess

            QPainterPath path;
            path.moveTo(m_start->getPos());

            QPointF center = (m_start->getPos() + m_end->getPos()) / 2;
            QPointF delta = (m_end->getPos() - center) / 2;

            double x = m_pos.x();
            double y = m_pos.y();

            double end_x = m_end->getPos().x();
            double end_y = m_end->getPos().y();

            QPointF p1, p2;

            p1 = m_pos - delta;
            p2 = m_pos + delta;

            if (m_start == m_end)
            {
                QPointF center = (m_pos + m_end->getPos()) / 2;
                QPointF v = center - m_pos;
                double r = std::sqrt(v.x() * v.x() + v.y() * v.y());
                path.addEllipse(center, r, r);
            }
            else
            {
                path.quadTo(p1, m_pos);
                path.quadTo(p2, m_end->getPos());
            }

            p.strokePath(path, pen);

            ///@ fix arrow location

            double bx, by;
            double q = 0.99;

            bx = (1 - q) * (1 - q) * x + 2 * (1 - q) * q * p2.x() +
                 q * q * end_x;
            by = (1 - q) * (1 - q) * y + 2 * (1 - q) * q * p2.y() +
                 q * q * end_y;

            double len =
                std::sqrt(std::pow(end_x - bx, 2) + std::pow(end_y - by, 2));

            bx = m_end->getSize() * (end_x - bx) / len;
            by = m_end->getSize() * (end_y - by) / len;

            double cos = std::cos(M_PI / 15);
            double sin = std::sin(M_PI / 15);

            path = QPainterPath();
            path.moveTo(end_x - bx, end_y - by);
            path.lineTo(
                end_x - 1.5 * (bx * cos + by * -sin),
                end_y - 1.5 * (bx * sin + by * cos));
            path.lineTo(
                end_x - 1.5 * (bx * cos + by * sin),
                end_y - 1.5 * (bx * -sin + by * cos));

            p.fillPath(path, Qt::black);
        }
        else
        {
            p.drawLine(m_start->getPos(), m_pos);
        }
    }
    else if (pass == 2)
    {
        QPainterPath path;
        path.addEllipse(m_pos, m_size, m_size);

        p.fillPath(path, m_selected ? c_selected_color : c_default_color);
        p.strokePath(path, pen);
    }
}

void TransitionGraphicsObject::setEnd(StateGraphicsObject *end)
{
    m_end = end;
    m_pos = (m_start->getPos() + m_end->getPos()) / 2;
}

StateGraphicsObject *TransitionGraphicsObject::getStart() const
{
    return m_start;
}

StateGraphicsObject *TransitionGraphicsObject::getEnd() const
{
    return m_end;
}

} // namespace fsmviz
