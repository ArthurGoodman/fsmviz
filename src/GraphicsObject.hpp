#pragma once

#include <QtGui/QPainter>

namespace fsmviz {

class GraphicsObject
{
public: // methods
    GraphicsObject();
    GraphicsObject(const QPointF &pos);

    virtual ~GraphicsObject();

    virtual void render(QPainter &p, int pass) = 0;

    virtual bool contains(const QPointF &p) const;

    void select();
    void deselect();

    QPointF getPos() const;
    void setPos(const QPointF &pos);

    void move(const QPointF &delta);

    double getSize() const;

    template <class T>
    T *as();

    void applyForce(const QPointF &force);
    void tick(); ///@ take dt into account

protected: // fields
    bool m_selected;
    double m_size;

    QPointF m_pos;
    QPointF m_velocity;
};

template <class T>
T *GraphicsObject::as()
{
    return dynamic_cast<T *>(this);
}

} // namespace fsmviz
