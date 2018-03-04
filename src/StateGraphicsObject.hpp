#pragma once

#include "GraphicsObject.hpp"

namespace fsmviz {

class StateGraphicsObject : public GraphicsObject
{
public: // methods
    StateGraphicsObject(const QPointF &pos);

    void render(QPainter &p, int pass) override;

    void toggleStarting();
    void toggleFinal();

private: // fields
    bool m_staring;
    bool m_final;
};

} // namespace fsmviz
