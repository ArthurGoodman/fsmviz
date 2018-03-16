#pragma once

#include <vector>
#include <QtCore/QObject>
#include "GraphicsObject.hpp"
#include "fsm/fsm.hpp"
#include "gcp/GenericCommandProcessor.hpp"
#include "qconsole/QConsole.hpp"

namespace fsmviz {

class View;

class Controller final : public QObject
{
    Q_OBJECT

public: // methods
    explicit Controller(
        gcp::GenericCommandProcessor &processor,
        qconsole::QConsole &console);

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
    void setupCommands();

    std::string getSaveFileName(const std::string &filter);

    void updateConnectedComponents();

    void reset();

    void deleteObject();
    void toggleStarting();
    void toggleFinal();

    void clearConsole();
    void printError(const std::string &message);

    void setDefaultSymbol(const std::string &sym);

    void printFsm(const fsm::Fsm &fsm);
    fsm::Fsm createFsm();
    void loadFsm(const fsm::Fsm &fsm);

    void exportGraphviz();
    void exportGraphviz(const std::string &file_name);

    void renderImage();
    void renderImage(const std::string &file_name);

private: // fields
    gcp::GenericCommandProcessor &m_processor;
    qconsole::QConsole &m_console;

    View *m_view;

    std::vector<GraphicsObjectPtr> m_objects;
    std::vector<StateGraphicsObjectPtr> m_states;
    std::vector<TransitionGraphicsObjectPtr> m_transitions;

    DefaultSymbol m_default_symbol;
    char m_default_letter;

    bool m_command_from_key; ///@todo Refactor?
};

} // namespace fsmviz
