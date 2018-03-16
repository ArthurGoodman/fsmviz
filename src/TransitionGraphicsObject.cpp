#include "TransitionGraphicsObject.hpp"
#include <cmath>
#include <QtGui/QtGui>

namespace fsmviz {

TransitionGraphicsObject::TransitionGraphicsObject(
    StateGraphicsObjectPtr start,
    const QVector2D &pos)
    : GraphicsObject{pos}
    , m_start{start}
    , m_end{nullptr}
    , m_symbol{'\0'}
    , m_editing{false}
{
}

void TransitionGraphicsObject::render(QPainter &p, int pass)
{
    static const QColor c_default_color = QColor(100, 220, 100);
    static const QColor c_selected_color = QColor(255, 100, 100);
    static const QColor c_edit_color = QColor(100, 100, 255);

    QPen pen(Qt::black, 2);
    p.setPen(pen);

    if (pass == 0)
    {
        if (m_end)
        {
            QPainterPath path;
            path.moveTo(m_start->getPos().toPointF());

            QVector2D center = (m_start->getPos() + m_end->getPos()) / 2;
            QVector2D delta = (m_end->getPos() - center) / 2;

            QVector2D p1, p2;

            p1 = m_pos - delta;
            p2 = m_pos + delta;

            if (m_start == m_end)
            {
                QVector2D center = (m_pos + m_end->getPos()) / 2;
                float r = (center - m_pos).length();
                path.addEllipse(center.toPointF(), r, r);
            }
            else
            {
                path.quadTo(p1.toPointF(), m_pos.toPointF());
                path.quadTo(p2.toPointF(), m_end->getPos().toPointF());
            }

            p.strokePath(path, pen);

            if (m_start == m_end)
            {
                QVector2D center = (m_pos + m_end->getPos()) / 2;
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
                t.rotate(35 * 25 / len);
                QVector2D dir_t = QVector2D(t.map(dir.toPointF()));

                drawArrow(p, m_end->getPos() + dir, -dir_t.normalized());
            }
            else
            {
                QVector2D t;
                float q = 0.9;
                t = (1 - q) * (1 - q) * m_pos + 2 * (1 - q) * q * p2 +
                    q * q * m_end->getPos();

                QVector2D n = (m_end->getPos() - t).normalized();
                drawArrow(p, m_end->getPos() - n * m_end->getSize(), n);
            }
        }
        else
        {
            p.drawLine(m_start->getPos().toPointF(), m_pos.toPointF());
        }
    }
    else if (pass == 2)
    {
        QPainterPath path;
        path.addEllipse(m_pos.toPointF(), getSize(), getSize());

        QColor color = m_selected ? c_selected_color : c_default_color;

        if (m_editing)
        {
            color = c_edit_color;
        }

        p.fillPath(path, color);
        p.strokePath(path, pen);

        if (!m_editing && m_end)
        {
            p.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            static constexpr int c_rect_size = 15;

            QPoint center = QPoint(c_rect_size, c_rect_size) / 2;
            QSize size(c_rect_size, c_rect_size);

            const char sym[] = {m_symbol, '\0'};

            p.drawText(
                QRect(m_pos.toPoint() - center, size),
                Qt::AlignCenter,
                QString(sym[0] ? sym : "\u03b5"));
        }
    }
} // namespace fsmviz

double TransitionGraphicsObject::getSize() const
{
    return 10;
}

void TransitionGraphicsObject::setEnd(StateGraphicsObjectPtr end)
{
    m_end = end;
    m_pos = (m_start->getPos() + m_end->getPos()) / 2;
}

void TransitionGraphicsObject::setSymbol(char symbol)
{
    m_symbol = symbol;
    m_editing = false;
}

StateGraphicsObjectPtr TransitionGraphicsObject::getStart() const
{
    return m_start;
}

StateGraphicsObjectPtr TransitionGraphicsObject::getEnd() const
{
    return m_end;
}

char TransitionGraphicsObject::getSymbol() const
{
    return m_symbol;
}

void TransitionGraphicsObject::startEditing()
{
    m_editing = true;
}

void TransitionGraphicsObject::drawArrow(
    QPainter &p,
    const QVector2D &pos,
    QVector2D dir)
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

} // namespace fsmviz
