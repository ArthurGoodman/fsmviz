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

///@ refactor this
void drawArrow(QPainter &p, const QVector2D &pos, QVector2D dir)
{
    static const float c_side = 10;
    dir *= c_side;

    QPainterPath path;

    QVector2D n(dir.y(), -dir.x());

    path.moveTo(pos.toPointF());
    path.lineTo((pos + n / 1.5 - dir).toPointF());
    path.lineTo((pos - n / 1.5 - dir).toPointF());
    path.lineTo(pos.toPointF());

    p.fillPath(path, Qt::black);
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

            ///@ fix QVector2D
            QVector2D center =
                QVector2D(m_start->getPos() + m_end->getPos()) / 2;
            QVector2D delta =
                QVector2D(m_end->getPos() - center.toPointF()) / 2;

            QVector2D p1, p2;

            ///@ fix QVector2D
            p1 = QVector2D(m_pos) - delta;
            p2 = QVector2D(m_pos) + delta;

            if (m_start == m_end)
            {
                QPointF center = (m_pos + m_end->getPos()) / 2;
                ///@ fix QVector2D
                QVector2D v = QVector2D(center - m_pos);
                float r = v.length();
                path.addEllipse(center, r, r);
            }
            else
            {
                path.quadTo(p1.toPointF(), m_pos);
                path.quadTo(p2.toPointF(), m_end->getPos());
            }

            p.strokePath(path, pen);

            if (m_start == m_end)
            {
                ///@ fix QVector2D

                QPointF center = (m_pos + m_end->getPos()) / 2;
                QVector2D v = QVector2D(m_pos - center);

                float r = v.length();
                float R = m_end->getSize();
                float d = QVector2D(m_end->getPos() - center).length();
                float q = d * d - r * r + R * R;
                float x = q / 2 / d;
                float a = std::sqrt(4 * d * d * R * R - std::pow(q, 2)) / d;

                float len = v.length();

                v.normalize();
                QVector2D dir = v * x + QVector2D(v.y(), -v.x()) * a / 2;

                QTransform t;
                t.rotate(35 * 25 / len); ///@ magic constant
                QVector2D dir_t = QVector2D(t.map(dir.toPointF()));

                drawArrow(
                    p,
                    QVector2D(m_end->getPos() + dir.toPointF()),
                    -dir_t.normalized());
            }
            else
            {
                ///@ fix QVector2D
                QVector2D end(m_end->getPos());
                QVector2D pos(m_pos);

                QVector2D t;
                float q = 0.9;
                t = (1 - q) * (1 - q) * pos + 2 * (1 - q) * q * p2 +
                    q * q * end;

                QVector2D n = (end - t).normalized();
                drawArrow(p, end - n * m_end->getSize(), n);
            }
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
