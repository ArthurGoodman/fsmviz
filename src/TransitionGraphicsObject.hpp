#pragma once

#include "StateGraphicsObject.hpp"

namespace fsmviz {

class TransitionGraphicsObject : public GraphicsObject
{
public: // methods
    TransitionGraphicsObject(StateGraphicsObject *start, const QPointF &pos);

    void render(QPainter &p, int pass) override;

    void setEnd(StateGraphicsObject *end);

    StateGraphicsObject *getStart() const;
    StateGraphicsObject *getEnd() const;

private: // fields
    StateGraphicsObject *m_start;
    StateGraphicsObject *m_end;
};

} // namespace fsmviz
