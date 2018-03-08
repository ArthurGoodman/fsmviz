#include "GraphicsObject.hpp"

namespace fsmviz {

GraphicsObject::GraphicsObject()
    : m_selected{false}
    , m_pos{}
    , m_velocity{}
{
}

GraphicsObject::GraphicsObject(const QVector2D &pos)
    : m_selected{false}
    , m_pos{pos}
    , m_velocity{}
{
}

GraphicsObject::~GraphicsObject()
{
}

bool GraphicsObject::contains(const QVector2D &p) const
{
    return (m_pos - p).length() <= getSize();
}

void GraphicsObject::select()
{
    m_selected = true;
}

void GraphicsObject::deselect()
{
    m_selected = false;
}

bool GraphicsObject::isSelected() const
{
    return m_selected;
}

QVector2D GraphicsObject::getPos() const
{
    return m_pos;
}

void GraphicsObject::setPos(const QVector2D &pos)
{
    m_pos = pos;
}

void GraphicsObject::move(const QVector2D &delta)
{
    m_pos += delta;
}

void GraphicsObject::applyForce(const QVector2D &force)
{
    m_velocity += force;
}

void GraphicsObject::tick(float dt)
{
    static constexpr float c_damping = 1.1;
    static constexpr float c_fps = 60;

    m_pos += m_velocity * dt * c_fps;
    m_velocity /= c_damping;
}

} // namespace fsmviz
