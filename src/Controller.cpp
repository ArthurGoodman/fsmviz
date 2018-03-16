#include "Controller.hpp"
#include <fstream>
#include <functional>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"
#include "View.hpp"
#include "fsm/fsm.hpp"

namespace fsmviz {

Controller::Controller(gcp::GenericCommandProcessor &processor)
    : m_processor{processor}
    , m_default_symbol{DefaultSymbol::Epsilon}
    , m_default_letter{'\0'}
{
    ///@todo Move commands into separate methods

    ///@ temporary workaround
    static bool from_key = false;

    m_processor.registerErrorCallback([&](const std::string &message) {
        if (from_key)
        {
            m_view->getConsole().insertBlock();
        }

        m_view->getConsole() << "error: " << message << "\n";

        if (from_key)
        {
            m_view->getConsole().insertPrompt();
        }
    });

    m_processor.registerCommand("quit", [&]() { m_view->close(); });
    m_processor.registerCommand("exit", [&]() { m_view->close(); });

    m_processor.registerCommand(
        "bind", [&](const std::string &key, const std::string &command) {
            m_view->bind(key, [=]() {
                from_key = true;
                m_processor.process(command);
                from_key = false;
            });
        });

    ///@todo fix for std::bind
    m_processor.registerCommand(
        "unbind",
        std::function<void(const std::string &)>(
            std::bind(&View::unbind, m_view, std::placeholders::_1)));

    m_processor.registerCommand("toggle_run", [&]() { m_view->toggleRun(); });

    m_processor.registerCommand("reset", [&]() {
        m_objects.clear();
        m_states.clear();
        m_transitions.clear();

        m_view->reset();
    });

    m_processor.registerCommand(
        "antialias", [&]() { m_view->toggleAntialiasing(); });

    m_processor.registerCommand("delete", [&]() {
        GraphicsObjectPtr selected_object = m_view->getSelectedObject();

        if (selected_object)
        {
            StateGraphicsObjectPtr state =
                cast<StateGraphicsObject>(selected_object);
            if (state)
            {
                ///@ boilerplate
                m_objects.erase(std::remove(
                    std::begin(m_objects), std::end(m_objects), state));
                m_states.erase(std::remove(
                    std::begin(m_states), std::end(m_states), state));

                auto transitions = state->getTransitions();

                for (TransitionGraphicsObjectPtr tr : transitions)
                {
                    tr->getStart()->disconnect(tr);

                    if (tr->getEnd() != tr->getStart())
                    {
                        tr->getEnd()->disconnect(tr);
                    }
                }

                ///@ boilerplate
                m_objects.erase(
                    std::remove_if(
                        std::begin(m_objects),
                        std::end(m_objects),
                        [&](GraphicsObjectPtr obj) {
                            return std::find(
                                       std::begin(transitions),
                                       std::end(transitions),
                                       obj) != transitions.end();
                        }),
                    std::end(m_objects));

                m_transitions.erase(
                    std::remove_if(
                        std::begin(m_transitions),
                        std::end(m_transitions),
                        [&](TransitionGraphicsObjectPtr obj) {
                            return std::find(
                                       std::begin(transitions),
                                       std::end(transitions),
                                       obj) != transitions.end();
                        }),
                    std::end(m_transitions));
            }
            else
            {
                TransitionGraphicsObjectPtr transition =
                    cast<TransitionGraphicsObject>(selected_object);
                if (transition && transition->getEnd())
                {
                    transition->getStart()->disconnect(transition);
                    transition->getEnd()->disconnect(transition);

                    ///@ boilerplate
                    m_objects.erase(std::remove(
                        std::begin(m_objects),
                        std::end(m_objects),
                        selected_object));
                    m_transitions.erase(std::remove(
                        std::begin(m_transitions),
                        std::end(m_transitions),
                        transition));
                }
            }

            m_view->deselect();
            updateConnectedComponents();
        }
    });

    m_processor.registerCommand(
        "toggle_fullscreen", [&]() { m_view->toggleFullscreen(); });

    m_processor.registerCommand(
        "show_fullscreen", [&]() { m_view->showFullScreen(); });
    m_processor.registerCommand("show_normal", [&]() { m_view->showNormal(); });

    m_processor.registerCommand("toggle_starting", [&]() {
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_view->getSelectedObject());
        if (state)
        {
            state->toggleStarting();
        }
    });

    m_processor.registerCommand("toggle_final", [&]() {
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_view->getSelectedObject());
        if (state)
        {
            state->toggleFinal();
        }
    });

    m_processor.registerCommand("run", [&]() { m_view->run(); });
    m_processor.registerCommand("stop", [&]() { m_view->stop(); });

    m_processor.registerCommand("clear", [&]() {
        m_view->getConsole().clear();
        if (from_key)
        {
            m_view->getConsole().insertPrompt();
        }
    });

    ///@ boilerplate
    m_processor.registerCommand("cls", [&]() {
        m_view->getConsole().clear();
        if (from_key)
        {
            m_view->getConsole().insertPrompt();
        }
    });

    m_processor.registerCommand("edit", [&]() { m_view->edit(); });

    m_processor.registerCommand("symbol", [&](const std::string &sym) {
        if (sym == "epsilon")
        {
            m_default_symbol = DefaultSymbol::Epsilon;
        }
        else if (sym == "random")
        {
            m_default_symbol = DefaultSymbol::Random;
        }
        else if (sym.size() == 1 && std::isalnum(sym[0]))
        {
            m_default_symbol = DefaultSymbol::Letter;
            m_default_letter = sym[0];
        }
        else
        {
            m_view->getConsole() << "error: invalid symbol\n";
        }
    });

    auto printFsm = [&](const fsm::Fsm &fsm) {
        std::stringstream stream;
        stream << fsm;
        m_view->getConsole() << stream.str();
    };

    auto writeFsm = [&]() {
        fsm::Fsm fsm(m_states.size());

        std::map<StateGraphicsObjectPtr, std::size_t> state_indices;

        std::size_t i = 0;
        for (StateGraphicsObjectPtr s : m_states)
        {
            if (s->isStarting())
            {
                fsm.setStarting(i);
            }

            if (s->isFinal())
            {
                fsm.setFinal(i);
            }

            state_indices[s] = i++;
        }

        for (TransitionGraphicsObjectPtr t : m_transitions)
        {
            fsm.connect(
                state_indices[t->getStart()],
                state_indices[t->getEnd()],
                t->getSymbol());
        }

        return fsm;
    };

    auto readFsm = [&](const fsm::Fsm &fsm) {
        m_processor.process("reset");

        auto transitions = fsm.getTransitions();
        auto starting_states = fsm.getStartingStates();
        auto final_states = fsm.getFinalStates();

        auto pos = []() {
            return QVector2D(
                static_cast<float>(std::rand()) / RAND_MAX,
                static_cast<float>(std::rand()) / RAND_MAX);
        };

        for (fsm::Fsm::state_t s = 0; s < transitions.size(); s++)
        {
            StateGraphicsObjectPtr state{
                new StateGraphicsObject(pos(), m_states.size())};
            m_objects.emplace_back(state);
            m_states.emplace_back(state);

            if (starting_states.find(s) != starting_states.end())
            {
                state->toggleStarting();
            }

            if (final_states.find(s) != final_states.end())
            {
                state->toggleFinal();
            }
        }

        for (fsm::Fsm::state_t s1 = 0; s1 < transitions.size(); s1++)
        {
            for (fsm::Fsm::state_t s2 = 0; s2 < transitions.size(); s2++)
            {
                for (fsm::Fsm::symbol_t a : transitions[s1][s2])
                {
                    TransitionGraphicsObjectPtr transition{
                        new TransitionGraphicsObject(m_states[s1], pos())};

                    transition->setEnd(m_states[s2]);
                    transition->setSymbol(a);

                    m_states[s1]->connect(transition);
                    m_states[s2]->connect(transition);

                    m_objects.emplace_back(transition);
                    m_transitions.emplace_back(transition);
                }
            }
        }

        updateConnectedComponents();
    };

    m_processor.registerCommand("print", [=]() { printFsm(writeFsm()); });

    m_processor.registerCommand("rev", [=]() { readFsm(writeFsm().rev()); });
    m_processor.registerCommand("det", [=]() { readFsm(writeFsm().det()); });
    m_processor.registerCommand("min", [=]() { readFsm(writeFsm().min()); });

    m_processor.registerCommand("export", [&](const std::string &file_name) {
        std::ofstream f(file_name);

        f << "digraph fsm {" << std::endl;
        f << "rankdir=LR;" << std::endl;
        f << "node[shape=doublecircle];";

        std::map<StateGraphicsObjectPtr, std::size_t> state_indices;

        std::size_t i = 0;
        for (StateGraphicsObjectPtr s : m_states)
        {
            if (s->isFinal())
            {
                f << "\"" << i << "\";";
            }

            state_indices[s] = i++;
        }

        f << std::endl;
        f << "node[shape=circle];" << std::endl;

        for (TransitionGraphicsObjectPtr t : m_transitions)
        {
            char sym[] = {t->getSymbol(), '\0'};
            f << "\"" << state_indices[t->getStart()] << "\"->\""
              << state_indices[t->getEnd()] << "\"[label=\""
              << (t->getSymbol() ? sym : "\u03b5") << "\"];" << std::endl;
        }

        f << "}" << std::endl;
    });

    m_processor.registerCommand("render", [&](const std::string &file_name) {
        if (file_name.empty())
        {
            m_view->getConsole() << "error: empty file name\n";
        }
        m_view->renderImage(file_name);
    });

    m_processor.registerCommand("render", [&]() {
        std::string file_name = QFileDialog::getSaveFileName(
                                    m_view, "", "", "Images(*.png *.jpg *.bmp)")
                                    .toStdString();
        m_view->renderImage(file_name);
    });
}

void Controller::setView(View *view)
{
    m_view = view;
}

const std::vector<GraphicsObjectPtr> &Controller::getObjects() const
{
    return m_objects;
}

StateGraphicsObjectPtr Controller::createState(const QVector2D &pos)
{
    StateGraphicsObjectPtr state{new StateGraphicsObject(pos, m_states.size())};
    m_objects.emplace_back(state);
    m_states.emplace_back(state);

    updateConnectedComponents();

    return state;
}

TransitionGraphicsObjectPtr Controller::createTransition(
    StateGraphicsObjectPtr state,
    const QVector2D &pos)
{
    TransitionGraphicsObjectPtr transition{
        new TransitionGraphicsObject(state, pos)};

    state->connect(transition);

    switch (m_default_symbol)
    {
    case DefaultSymbol::Epsilon:
        break;

    case DefaultSymbol::Random:
        transition->setSymbol('a' + std::rand() % ('z' - 'a' + 1));
        break;

    case DefaultSymbol::Letter:
        transition->setSymbol(m_default_letter);
        break;
    }

    m_objects.emplace_back(transition);
    m_transitions.emplace_back(transition);

    updateConnectedComponents();

    return transition;
}

void Controller::connectTransition(
    TransitionGraphicsObjectPtr transition,
    StateGraphicsObjectPtr end)
{
    transition->setEnd(end);
    end->connect(transition);

    updateConnectedComponents();
}

GraphicsObjectPtr Controller::objectAt(const QVector2D &pos) const
{
    for (GraphicsObjectPtr obj : m_states)
    {
        if (obj->contains(pos))
        {
            return obj;
        }
    }

    return nullptr;
}

StateGraphicsObjectPtr Controller::stateAt(const QVector2D &pos) const
{
    for (StateGraphicsObjectPtr state : m_states)
    {
        if (state->contains(pos))
        {
            return state;
        }
    }

    return nullptr;
}

void Controller::updateConnectedComponents()
{
    std::function<void(StateGraphicsObjectPtr, int)> visitState =
        [&](StateGraphicsObjectPtr state, int tag) {
            if (state->getFlag())
            {
                return;
            }

            state->setFlag(true);
            state->setTag(tag);

            auto transitions = state->getTransitions();

            for (TransitionGraphicsObjectPtr transition : transitions)
            {
                transition->setTag(tag);

                if (transition->getStart() != state)
                {
                    visitState(transition->getStart(), tag);
                }
                else if (transition->getEnd())
                {
                    visitState(transition->getEnd(), tag);
                }
            }
        };

    for (StateGraphicsObjectPtr state : m_states)
    {
        state->setFlag(false);
    }

    int tag = 0;

    for (StateGraphicsObjectPtr state : m_states)
    {
        visitState(state, tag++);
    }
}

} // namespace fsmviz
