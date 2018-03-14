#pragma once

#include <vector>
#include <QtCore/QObject>
#include "GraphicsObject.hpp"
#include "gcp/GenericCommandProcessor.hpp"
#include "qconsole/QConsole.hpp"

namespace fsmviz {

class View;

class Controller final : public QObject
{
    Q_OBJECT

public: // methods
    explicit Controller(gcp::GenericCommandProcessor &processor);

    ///@todo Sort methods
    void setView(View *view);

    const std::vector<GraphicsObjectPtr> &getObjects() const;

    StateGraphicsObjectPtr createState(const QVector2D &pos);

    TransitionGraphicsObjectPtr createTransition(
        StateGraphicsObjectPtr state,
        const QVector2D &pos);

    void connectTransition(
        TransitionGraphicsObjectPtr transition,
        StateGraphicsObjectPtr end);

    GraphicsObjectPtr objectAt(const QVector2D &pos) const;
    StateGraphicsObjectPtr stateAt(const QVector2D &pos) const;

private: // types
    enum class DefaultSymbol
    {
        Epsilon,
        Random,
        Letter,
    };

private: // methods
    void updateConnectedComponents();

private: // fields
    View *m_view;

    gcp::GenericCommandProcessor &m_processor;

    std::vector<GraphicsObjectPtr> m_objects;
    std::vector<StateGraphicsObjectPtr> m_states;
    std::vector<TransitionGraphicsObjectPtr> m_transitions;

    DefaultSymbol m_default_symbol;
    char m_default_letter;
};

} // namespace fsmviz
