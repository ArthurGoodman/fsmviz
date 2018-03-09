#pragma once

#include <vector>
#include "GraphicsObject.hpp"

namespace fsmviz {

class StateGraphicsObject : public GraphicsObject
{
public: // methods
    StateGraphicsObject(const QVector2D &pos);

    void render(QPainter &p, int pass) override;

    double getSize() const override;

    void toggleStarting();
    void toggleFinal();

    void connect(GraphicsObjectPtr transition);
    void disconnect(GraphicsObjectPtr transition);
    std::vector<GraphicsObjectPtr> getTransitions() const;

private: // fields
    std::vector<GraphicsObjectPtr> m_transitions;

    bool m_staring;
    bool m_final;
};

using StateGraphicsObjectPtr = std::shared_ptr<StateGraphicsObject>;

} // namespace fsmviz
