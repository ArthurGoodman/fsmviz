#pragma once

#include "StateGraphicsObject.hpp"

namespace fsmviz {

class TransitionGraphicsObject : public GraphicsObject
{
public: // methods
    TransitionGraphicsObject(StateGraphicsObject *start, const QVector2D &pos);

    void render(QPainter &p, int pass) override;

    double getSize() const override;

    void setEnd(StateGraphicsObject *end);

    StateGraphicsObject *getStart() const;
    StateGraphicsObject *getEnd() const;

private: // methods
    void drawArrow(QPainter &p, const QVector2D &pos, QVector2D dir);

private: // fields
    StateGraphicsObject *m_start;
    StateGraphicsObject *m_end;
};

} // namespace fsmviz
