#pragma once

#include "StateGraphicsObject.hpp"

namespace fsmviz {

class TransitionGraphicsObject : public GraphicsObject
{
public: // methods
    TransitionGraphicsObject(
        StateGraphicsObjectPtr start,
        const QVector2D &pos);

    void render(QPainter &p, int pass) override;

    double getSize() const override;

    void setEnd(StateGraphicsObjectPtr end);

    StateGraphicsObjectPtr getStart() const;
    StateGraphicsObjectPtr getEnd() const;

private: // methods
    void drawArrow(QPainter &p, const QVector2D &pos, QVector2D dir);

private: // fields
    StateGraphicsObjectPtr m_start;
    StateGraphicsObjectPtr m_end;
};

} // namespace fsmviz
