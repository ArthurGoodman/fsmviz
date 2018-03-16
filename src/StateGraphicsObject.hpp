#pragma once

#include <cstddef>
#include <vector>
#include "GraphicsObject.hpp"

namespace fsmviz {

class StateGraphicsObject : public GraphicsObject
{
public: // methods
    explicit StateGraphicsObject(const QVector2D &pos, std::size_t id);

    void render(QPainter &p, int pass) override;

    double getSize() const override;

    void toggleStarting();
    void toggleFinal();

    bool isStarting() const;
    bool isFinal() const;

    void connect(TransitionGraphicsObjectPtr transition);
    void disconnect(TransitionGraphicsObjectPtr transition);
    std::vector<TransitionGraphicsObjectPtr> getTransitions() const;

    bool getFlag() const;
    void setFlag(bool flag);

    std::size_t getId() const;
    void setId(std::size_t id);

private: // fields
    std::vector<TransitionGraphicsObjectPtr> m_transitions;

    bool m_staring;
    bool m_final;

    bool m_flag;

    std::size_t m_id;
};

} // namespace fsmviz
