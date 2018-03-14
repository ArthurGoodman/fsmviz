#pragma once

#include "StateGraphicsObject.hpp"

namespace fsmviz {

class TransitionGraphicsObject : public GraphicsObject
{
public: // methods
    explicit TransitionGraphicsObject(
        StateGraphicsObjectPtr start,
        const QVector2D &pos);

    void render(QPainter &p, int pass) override;

    double getSize() const override;

    void setEnd(StateGraphicsObjectPtr end);
    void setSymbol(char symbol);

    StateGraphicsObjectPtr getStart() const;
    StateGraphicsObjectPtr getEnd() const;
    char getSymbol() const;

private: // methods
    void drawArrow(QPainter &p, const QVector2D &pos, QVector2D dir);

private: // fields
    StateGraphicsObjectPtr m_start;
    StateGraphicsObjectPtr m_end;
    char m_symbol;
};

} // namespace fsmviz
