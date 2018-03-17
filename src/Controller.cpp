#include "Controller.hpp"
#include <cstring>
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

const std::vector<StateGraphicsObjectPtr> &Controller::getStates() const
{
    return m_states;
}

const std::vector<TransitionGraphicsObjectPtr> &Controller::getTransitions()
    const
{
    return m_transitions;
}

StateGraphicsObjectPtr Controller::createState(
    const QVector2D &pos,
    bool is_starting,
    bool is_final,
    bool update_connected_components)
{
    StateGraphicsObjectPtr state{new StateGraphicsObject(pos, m_states.size())};
    m_objects.emplace_back(state);
    m_states.emplace_back(state);

    if (is_starting)
    {
        state->toggleStarting();
    }

    if (is_final)
    {
        state->toggleFinal();
    }

    if (update_connected_components)
    {
        updateConnectedComponents();
    }

    return state;
}

TransitionGraphicsObjectPtr Controller::createTransition(
    StateGraphicsObjectPtr start,
    const QVector2D &pos,
    bool update_connected_components)
{
    TransitionGraphicsObjectPtr transition{
        new TransitionGraphicsObject(start, pos)};

    start->connect(transition);

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

    if (update_connected_components)
    {
        updateConnectedComponents();
    }

    return transition;
}

TransitionGraphicsObjectPtr Controller::createTransition(
    StateGraphicsObjectPtr start,
    StateGraphicsObjectPtr end,
    char symbol,
    const QVector2D &pos,
    bool update_connected_components)
{
    TransitionGraphicsObjectPtr transition{
        new TransitionGraphicsObject(start, QVector2D())};

    transition->setEnd(end);
    transition->setPos(pos);
    transition->setSymbol(symbol);

    start->connect(transition);
    end->connect(transition);

    m_objects.emplace_back(transition);
    m_transitions.emplace_back(transition);

    if (update_connected_components)
    {
        updateConnectedComponents();
    }

    return transition;
}

void Controller::connectTransition(
    TransitionGraphicsObjectPtr transition,
    StateGraphicsObjectPtr end,
    bool update_connected_components)
{
    transition->setEnd(end);
    end->connect(transition);

    if (update_connected_components)
    {
        updateConnectedComponents();
    }
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
    ///@todo Implement batch processing

    m_processor.registerErrorCallback(
        [&](const std::string &message) { print("error: " + message); });

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

    m_processor.registerCommand("print", [&]() { printFsm(buildFsm()); });

    m_processor.registerCommand("rev", [&]() { loadFsm(buildFsm().rev()); });
    m_processor.registerCommand("det", [&]() { loadFsm(buildFsm().det()); });
    m_processor.registerCommand("min", [&]() { loadFsm(buildFsm().min()); });

    m_processor.registerCommand("export", [&]() { exportGraphviz(); });
    m_processor.registerCommand("export", [&](const std::string &file_name) {
        exportGraphviz(file_name);
    });

    m_processor.registerCommand("render", [&]() { renderImage(); });
    m_processor.registerCommand("render", [&](const std::string &file_name) {
        renderImage(file_name);
    });

    m_processor.registerCommand(
        "echo", [&](const std::string &str) { print(str); });

    m_processor.registerCommand("save", [&]() { save(); });
    m_processor.registerCommand(
        "save", [&](const std::string &file_name) { save(file_name); });

    m_processor.registerCommand("open", [&]() { open(); });
    m_processor.registerCommand(
        "open", [&](const std::string &file_name) { open(file_name); });
}

std::string Controller::getSaveFileName(const std::string &filter)
{
    return QFileDialog::getSaveFileName(m_view, "", "", filter.c_str())
        .toStdString();
}

std::string Controller::getOpenFileName(const std::string &filter)
{
    return QFileDialog::getOpenFileName(m_view, "", "", filter.c_str())
        .toStdString();
}

void visitState(StateGraphicsObjectPtr state, int tag)
{
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
}

void Controller::updateConnectedComponents()
{
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

void Controller::print(const std::string &message)
{
    if (!m_view->isConsoleVisible())
    {
        m_view->toggleConsole();
    }

    if (m_command_from_key)
    {
        m_console.eraseBlock();
    }

    m_console << message << "\n";

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
        print("error: invalid symbol");
    }
}

void Controller::printFsm(const fsm::Fsm &fsm)
{
    std::stringstream stream;
    stream << fsm;
    m_console << stream.str();
}

fsm::Fsm Controller::buildFsm()
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
    std::string file_name =
        getSaveFileName("Graphviz files (*.gv);;All files (*)");
    if (!file_name.empty())
    {
        exportGraphviz(file_name);
    }
}

void Controller::exportGraphviz(const std::string &file_name)
{
    if (file_name.empty())
    {
        print("error: empty file name");
        return;
    }

    std::ofstream file(file_name);

    if (!file)
    {
        print("error: couldn't open file");
        return;
    }

    file << "digraph fsm {" << std::endl;
    file << "rankdir=LR;" << std::endl;

    file << "node[shape=doublecircle];" << std::endl;

    std::map<StateGraphicsObjectPtr, std::size_t> state_indices;

    std::size_t i = 0;
    for (StateGraphicsObjectPtr s : m_states)
    {
        if (s->isFinal())
        {
            file << "\"" << i << "\";" << std::endl;
        }

        state_indices[s] = i++;
    }

    file << "node[shape=circle];" << std::endl;

    for (TransitionGraphicsObjectPtr t : m_transitions)
    {
        char sym[] = {t->getSymbol(), '\0'};
        file << "\"" << state_indices[t->getStart()] << "\"->\""
             << state_indices[t->getEnd()] << "\"[label=\""
             << (sym[0] ? sym : "\u03b5") << "\"];" << std::endl;
    }

    file << "}" << std::endl;
}

void Controller::renderImage()
{
    std::string file_name =
        getSaveFileName("Images (*.bmp *.jpg *.png);;All files (*)");
    if (!file_name.empty())
    {
        renderImage(file_name);
    }
}

void Controller::renderImage(const std::string &file_name)
{
    if (file_name.empty())
    {
        print("error: empty file name");
        return;
    }

    m_view->renderImage(file_name);
}

void Controller::save()
{
    std::string file_name = getSaveFileName("Fsm files (*.fsm);;All files (*)");
    if (!file_name.empty())
    {
        save(file_name);
    }
}

template <class T>
void write(std::ostream &stream, const T &value)
{
    stream.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <class T>
void read(std::istream &stream, T &value)
{
    stream.read(reinterpret_cast<char *>(&value), sizeof(T));
}

#pragma pack(push, 1)
struct FsmHeader
{
    char magic_number[3];
    std::uint64_t states;
    std::uint64_t transitions;
    float x_offset;
    float y_offset;
    float scale;
};

struct StateRecord
{
    std::uint64_t id;
    char is_starting;
    char is_final;
    float x;
    float y;
};

struct TransitionRecord
{
    std::uint64_t start;
    std::uint64_t end;
    char symbol;
    float x;
    float y;
};
#pragma pack(pop)

void Controller::save(const std::string &file_name)
{
    if (file_name.empty())
    {
        print("error: empty file name");
        return;
    }

    std::ofstream file(file_name, std::ios::binary);

    if (!file)
    {
        print("error: couldn't open file");
        return;
    }

    QPointF translation = m_view->getTranslation();
    float scale = m_view->getScale();

    FsmHeader header;

    std::memcpy(header.magic_number, "FSM", 3);
    header.states = m_states.size();
    header.transitions = m_transitions.size();
    header.x_offset = translation.x();
    header.y_offset = translation.y();
    header.scale = scale;

    write(file, header);

    std::map<StateGraphicsObjectPtr, std::size_t> state_indices;

    std::size_t i = 0;
    for (StateGraphicsObjectPtr s : m_states)
    {
        StateRecord rec;

        rec.id = i;
        rec.is_starting = s->isStarting();
        rec.is_final = s->isFinal();
        rec.x = s->getPos().x();
        rec.y = s->getPos().y();

        write(file, rec);

        state_indices[s] = i++;
    }

    for (TransitionGraphicsObjectPtr t : m_transitions)
    {
        TransitionRecord rec;

        rec.start = state_indices[t->getStart()];
        rec.end = state_indices[t->getEnd()];
        rec.symbol = t->getSymbol();
        rec.x = t->getPos().x();
        rec.y = t->getPos().y();

        write(file, rec);
    }
}

void Controller::open()
{
    std::string file_name = getOpenFileName("Fsm files (*.fsm);;All files (*)");
    if (!file_name.empty())
    {
        open(file_name);
    }
}

void Controller::open(const std::string &file_name)
{
    if (file_name.empty())
    {
        print("error: empty file name");
        return;
    }

    std::ifstream file(file_name, std::ios::binary);

    if (!file)
    {
        print("error: couldn't open file");
        return;
    }

    reset();

    FsmHeader header;
    read(file, header);

    if (std::strncmp(header.magic_number, "FSM", 3))
    {
        print("error: file corrupted");
        return;
    }

    m_view->setTranslation(QPointF(header.x_offset, header.y_offset));
    m_view->setScale(header.scale);

    for (std::size_t i = 0; i < header.states; i++)
    {
        StateRecord rec;
        read(file, rec);

        createState(
            QVector2D(rec.x, rec.y), rec.is_starting, rec.is_final, false);
    }

    for (std::size_t i = 0; i < header.transitions; i++)
    {
        TransitionRecord rec;
        read(file, rec);

        createTransition(
            m_states[rec.start],
            m_states[rec.end],
            rec.symbol,
            QVector2D(rec.x, rec.y),
            false);
    }

    updateConnectedComponents();
}

} // namespace fsmviz
