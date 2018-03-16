#include "Controller.hpp"
#include <fstream>
#include <functional>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"
#include "View.hpp"

namespace fsmviz {

Controller::Controller(
    gcp::GenericCommandProcessor &processor,
    qconsole::QConsole &console)
    : m_processor{processor}
    , m_console{console}
    , m_view{nullptr}
    , m_default_symbol{DefaultSymbol::Epsilon}
    , m_default_letter{'\0'}
    , m_command_from_key{false}
{
    setupCommands();
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
    for (GraphicsObjectPtr obj : m_objects)
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

void Controller::setupCommands()
{
    m_processor.registerErrorCallback(
        std::bind(&Controller::printError, this, std::placeholders::_1));

    m_processor.registerCommand("quit", [&]() { m_view->close(); });
    m_processor.registerCommand("exit", [&]() { m_view->close(); });

    m_processor.registerCommand(
        "bind", [&](const std::string &key, const std::string &command) {
            m_view->bind(key, [=]() {
                m_command_from_key = true;
                m_processor.process(command);
                m_command_from_key = false;
            });
        });

    m_processor.registerCommand(
        "unbind", [&](const std::string &str) { m_view->unbind(str); });

    m_processor.registerCommand("toggle_run", [&]() { m_view->toggleRun(); });

    m_processor.registerCommand("reset", [&]() { reset(); });

    m_processor.registerCommand(
        "antialias", [&]() { m_view->toggleAntialiasing(); });

    m_processor.registerCommand("delete", [&]() { deleteObject(); });

    m_processor.registerCommand(
        "toggle_fullscreen", [&]() { m_view->toggleFullscreen(); });

    m_processor.registerCommand(
        "show_fullscreen", [&]() { m_view->showFullScreen(); });
    m_processor.registerCommand("show_normal", [&]() { m_view->showNormal(); });

    m_processor.registerCommand("toggle_starting", [&]() { toggleStarting(); });
    m_processor.registerCommand("toggle_final", [&]() { toggleFinal(); });

    m_processor.registerCommand("run", [&]() { m_view->run(); });
    m_processor.registerCommand("stop", [&]() { m_view->stop(); });

    m_processor.registerCommand("clear", [&]() { clearConsole(); });
    m_processor.registerCommand("cls", [&]() { clearConsole(); });

    m_processor.registerCommand("edit", [&]() { m_view->edit(); });

    m_processor.registerCommand(
        "symbol", [&](const std::string &sym) { setDefaultSymbol(sym); });

    m_processor.registerCommand("print", [&]() { printFsm(createFsm()); });

    m_processor.registerCommand("rev", [&]() { loadFsm(createFsm().rev()); });
    m_processor.registerCommand("det", [&]() { loadFsm(createFsm().det()); });
    m_processor.registerCommand("min", [&]() { loadFsm(createFsm().min()); });

    m_processor.registerCommand("export", [&]() { exportGraphviz(); });
    m_processor.registerCommand("export", [&](const std::string &file_name) {
        exportGraphviz(file_name);
    });

    m_processor.registerCommand("render", [&]() { renderImage(); });
    m_processor.registerCommand("render", [&](const std::string &file_name) {
        renderImage(file_name);
    });
}

std::string Controller::getSaveFileName(const std::string &filter)
{
    return QFileDialog::getSaveFileName(m_view, "", "", filter.c_str())
        .toStdString();
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

void Controller::reset()
{
    m_objects.clear();
    m_states.clear();
    m_transitions.clear();

    m_view->reset();
}

template <class T, class U>
void remove(std::vector<T> &vector, const U &value)
{
    vector.erase(std::remove(
        std::begin(vector),
        std::end(vector),
        cast<typename U::element_type>(value)));
}

template <class T, class U>
void remove(std::vector<T> &vector, const std::vector<U> &values)
{
    vector.erase(
        std::remove_if(
            std::begin(vector),
            std::end(vector),
            [&](const T &value) {
                return std::find(
                           std::begin(values),
                           std::end(values),
                           cast<typename U::element_type>(value)) !=
                       values.end();
            }),
        std::end(vector));
}

void Controller::deleteObject()
{
    GraphicsObjectPtr selected_object = m_view->getSelectedObject();

    if (!selected_object)
    {
        return;
    }

    StateGraphicsObjectPtr state = cast<StateGraphicsObject>(selected_object);
    if (state)
    {
        for (std::size_t i = state->getId(); i < m_states.size(); i++)
        {
            m_states[i]->setId(m_states[i]->getId() - 1);
        }

        remove(m_objects, state);
        remove(m_states, state);

        auto transitions = state->getTransitions();

        for (TransitionGraphicsObjectPtr tr : transitions)
        {
            tr->getStart()->disconnect(tr);

            if (tr->getEnd() != tr->getStart())
            {
                tr->getEnd()->disconnect(tr);
            }
        }

        remove(m_objects, transitions);
        remove(m_transitions, transitions);
    }
    else
    {
        TransitionGraphicsObjectPtr transition =
            cast<TransitionGraphicsObject>(selected_object);
        if (transition && transition->getEnd())
        {
            transition->getStart()->disconnect(transition);
            transition->getEnd()->disconnect(transition);

            remove(m_objects, selected_object);
            remove(m_transitions, transition);
        }
    }

    m_view->deselect();
    updateConnectedComponents();
}

void Controller::toggleStarting()
{
    StateGraphicsObjectPtr state =
        cast<StateGraphicsObject>(m_view->getSelectedObject());
    if (state)
    {
        state->toggleStarting();
    }
}

void Controller::toggleFinal()
{
    StateGraphicsObjectPtr state =
        cast<StateGraphicsObject>(m_view->getSelectedObject());
    if (state)
    {
        state->toggleFinal();
    }
}

void Controller::clearConsole()
{
    m_console.clear();
    if (m_command_from_key)
    {
        m_console.insertPrompt();
    }
}

void Controller::printError(const std::string &message)
{
    if (!m_view->isConsoleVisible())
    {
        m_view->toggleConsole();
    }

    if (m_command_from_key)
    {
        m_console.removeBlock();
        m_console.insertBlock();
    }

    m_console << "error: " << message << "\n";

    if (m_command_from_key)
    {
        m_console.insertPrompt();
    }
}

void Controller::setDefaultSymbol(const std::string &sym)
{
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
        printError("invalid symbol");
    }
}

void Controller::printFsm(const fsm::Fsm &fsm)
{
    std::stringstream stream;
    stream << fsm;
    m_console << stream.str();
}

fsm::Fsm Controller::createFsm()
{
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
}

void Controller::loadFsm(const fsm::Fsm &fsm)
{
    reset();

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
}

void Controller::exportGraphviz()
{
    std::string file_name = getSaveFileName("Graphviz files (*.gv)");
    if (!file_name.empty())
    {
        exportGraphviz(file_name);
    }
}

void Controller::exportGraphviz(const std::string &file_name)
{
    if (file_name.empty())
    {
        printError("empty file name");
        return;
    }

    std::ofstream f(file_name);

    f << "digraph fsm {" << std::endl;
    f << "rankdir=LR;" << std::endl;
    f << "node[shape=doublecircle];" << std::endl;

    std::map<StateGraphicsObjectPtr, std::size_t> state_indices;

    std::size_t i = 0;
    for (StateGraphicsObjectPtr s : m_states)
    {
        if (s->isFinal())
        {
            f << "\"" << i << "\";" << std::endl;
        }

        state_indices[s] = i++;
    }

    f << "node[shape=circle];" << std::endl;

    for (TransitionGraphicsObjectPtr t : m_transitions)
    {
        char sym[] = {t->getSymbol(), '\0'};
        f << "\"" << state_indices[t->getStart()] << "\"->\""
          << state_indices[t->getEnd()] << "\"[label=\""
          << (sym[0] ? sym : "\u03b5") << "\"];" << std::endl;
    }

    f << "}" << std::endl;
}

void Controller::renderImage()
{
    std::string file_name = getSaveFileName("Images (*.bmp *.jpg *.png)");
    if (!file_name.empty())
    {
    }
    m_view->renderImage(file_name);
}

void Controller::renderImage(const std::string &file_name)
{
    if (file_name.empty())
    {
        printError("empty file name");
        return;
    }

    m_view->renderImage(file_name);
}

} // namespace fsmviz
