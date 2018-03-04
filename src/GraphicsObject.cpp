#include "GraphicsObject.hpp"

namespace fsmviz {

GraphicsObject::GraphicsObject()
    : m_selected{false}
    , m_size{}
    , m_pos{}
    , m_velocity{}
{
}

GraphicsObject::GraphicsObject(const QPointF &pos)
    : m_selected{false}
    , m_size{}
    , m_pos{pos}
    , m_velocity{}
{
}

GraphicsObject::~GraphicsObject()
{
}

bool GraphicsObject::contains(const QPointF &p) const
{
    ///@ distance function
    double dx = m_pos.x() - p.x();
    double dy = m_pos.y() - p.y();
    return std::sqrt(dx * dx + dy * dy) <= m_size;
}

void GraphicsObject::select()
{
    m_selected = true;
}

void GraphicsObject::deselect()
{
    m_selected = false;
}

QPointF GraphicsObject::getPos() const
{
    return m_pos;
}

void GraphicsObject::setPos(const QPointF &pos)
{
    m_pos = pos;
}

void GraphicsObject::move(const QPointF &delta)
{
    m_pos += delta;
}

double GraphicsObject::getSize() const
{
    return m_size;
}

void GraphicsObject::applyForce(const QPointF &force)
{
    m_velocity += force;
}

void GraphicsObject::tick()
{
    m_pos += m_velocity;
    m_velocity /= 1.05; ///@ magic constant
}

} // namespace fsmviz
