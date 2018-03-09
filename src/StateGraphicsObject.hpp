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

    void connect(TransitionGraphicsObjectPtr transition);
    void disconnect(TransitionGraphicsObjectPtr transition);
    std::vector<TransitionGraphicsObjectPtr> getTransitions() const;

private: // fields
    std::vector<TransitionGraphicsObjectPtr> m_transitions;

    bool m_staring;
    bool m_final;
};

} // namespace fsmviz
