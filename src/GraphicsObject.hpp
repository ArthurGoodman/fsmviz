#pragma once

#include <memory>
#include <QtGui/QtGui>

namespace fsmviz {

class GraphicsObject
{
public: // methods
    GraphicsObject();
    GraphicsObject(const QVector2D &pos);

    virtual ~GraphicsObject();

    virtual void render(QPainter &p, int pass) = 0;

    virtual double getSize() const = 0;

    virtual bool contains(const QVector2D &p) const;

    void select();
    void deselect();

    bool isSelected() const;

    QVector2D getPos() const;
    void setPos(const QVector2D &pos);

    void move(const QVector2D &delta);

    void applyForce(const QVector2D &force);
    void tick(float dt);

protected: // fields
    bool m_selected;

    QVector2D m_pos;
    QVector2D m_velocity;
};

using GraphicsObjectPtr = std::shared_ptr<GraphicsObject>;

template <class T>
std::shared_ptr<T> cast(GraphicsObjectPtr obj)
{
    return std::dynamic_pointer_cast<T>(obj);
}

} // namespace fsmviz
