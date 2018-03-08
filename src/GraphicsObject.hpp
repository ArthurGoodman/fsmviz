#pragma once

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

    template <class T>
    T *as();

    void applyForce(const QVector2D &force);
    void tick(float dt);

protected: // fields
    bool m_selected;

    QVector2D m_pos;
    QVector2D m_velocity;
};

template <class T>
T *GraphicsObject::as()
{
    return dynamic_cast<T *>(this);
}

} // namespace fsmviz
