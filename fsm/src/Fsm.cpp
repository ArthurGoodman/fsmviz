#include "fsm/Fsm.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace fsm {

Fsm::Fsm(
    std::size_t states,
    const std::set<state_t> &s,
    const std::set<state_t> &f)
    : m_transitions(states)
    , m_starting_states(s)
    , m_final_states(f)
{
    for (std::vector<std::set<symbol_t>> &t : m_transitions)
    {
        t.resize(states);
    }
}

Fsm::Fsm(
    const std::vector<std::vector<std::set<symbol_t>>> &t,
    const std::set<state_t> &s,
    const std::set<state_t> &f)
    : m_transitions(t)
    , m_starting_states(s)
    , m_final_states(f)
{
    buildAlphabet();
}

Fsm::Fsm(
    const std::set<symbol_t> &alphabet,
    const std::vector<std::vector<std::vector<state_t>>> &t,
    const std::set<state_t> &s,
    const std::set<state_t> &f)
    : m_alphabet(alphabet)
    , m_transitions(t.size())
    , m_starting_states(s)
    , m_final_states(f)
{
    for (auto &row : m_transitions)
    {
        row.resize(t.size());
    }

    for (state_t s1 = 0; s1 < t.size(); s1++)
    {
        auto it = alphabet.begin();
        for (std::size_t i = 0; i <= alphabet.size(); i++)
        {
            for (state_t s2 : t[s1][i])
            {
                symbol_t sym = i == alphabet.size() ? '\0' : *it;
                connect(s1, s2, sym);
            }

            if (i < alphabet.size())
            {
                ++it;
            }
        }
    }
}

void Fsm::connect(state_t s1, state_t s2, symbol_t a)
{
    m_transitions[s1][s2].insert(a);
    if (a)
    {
        m_alphabet.insert(a);
    }
}

void Fsm::setStarting(state_t state, bool value)
{
    if (value)
    {
        m_starting_states.insert(state);
    }
    else
    {
        m_starting_states.erase(state);
    }
}

void Fsm::setFinal(state_t state, bool value)
{
    if (value)
    {
        m_final_states.insert(state);
    }
    else
    {
        m_final_states.erase(state);
    }
}

std::vector<std::vector<std::set<Fsm::symbol_t>>> Fsm::getTransitions() const
{
    return m_transitions;
}

std::set<Fsm::state_t> Fsm::getStartingStates() const
{
    return m_starting_states;
}

std::set<Fsm::state_t> Fsm::getFinalStates() const
{
    return m_final_states;
}

Fsm Fsm::rev() const
{
    Fsm rfsm(m_transitions.size(), m_final_states, m_starting_states);

    for (state_t s1 = 0; s1 < m_transitions.size(); s1++)
    {
        for (state_t s2 = 0; s2 < m_transitions.size(); s2++)
        {
            for (symbol_t a : m_transitions[s1][s2])
            {
                rfsm.connect(s2, s1, a);
            }
        }
    }

    return rfsm;
}

Fsm Fsm::det() const
{
    const std::vector<std::set<state_t>> &closures = epsilonClosures();

    std::vector<std::set<state_t>> q;

    std::set<state_t> q0;

    for (state_t s : m_starting_states)
    {
        q0.insert(closures[s].begin(), closures[s].end());
    }

    q.push_back(q0);

    std::vector<std::vector<std::vector<state_t>>> t;

    while (t.size() < q.size())
    {
        std::vector<std::vector<state_t>> row;

        for (symbol_t a : m_alphabet)
        {
            std::set<state_t> ts;

            for (state_t i : q[t.size()])
            {
                for (state_t s = 0; s < m_transitions.size(); s++)
                {
                    if (m_transitions[i][s].find(a) !=
                        m_transitions[i][s].end())
                    {
                        ts.insert(closures[s].begin(), closures[s].end());
                    }
                }
            }

            if (ts.empty())
            {
                row.push_back({});
                continue;
            }

            state_t index;

            const auto &it = std::find(q.begin(), q.end(), ts);

            if (it == q.end())
            {
                index = q.size();
                q.push_back(ts);
            }
            else
            {
                index = it - q.begin();
            }

            row.push_back({index});
        }

        row.push_back({});

        t.push_back(row);
    }

    std::set<state_t> f;

    for (std::size_t i = 0; i < q.size(); i++)
    {
        for (state_t s : q[i])
        {
            if (m_final_states.find(s) != m_final_states.end())
            {
                f.insert(i);
            }
        }
    }

    return Fsm(m_alphabet, t, {0}, f);
}

Fsm Fsm::min() const
{
    return rev().det().rev().det();
}

std::ostream &operator<<(std::ostream &stream, const Fsm &fsm)
{
    for (Fsm::state_t s1 = 0; s1 < fsm.m_transitions.size(); s1++)
    {
        for (Fsm::state_t s2 = 0; s2 < fsm.m_transitions.size(); s2++)
        {
            for (Fsm::symbol_t a : fsm.m_transitions[s1][s2])
            {
                fsm.printState(stream, s1);

                if (a == '\0')
                {
                    stream << " --->> ";
                }
                else
                {
                    stream << " --" << a << "-> ";
                }

                fsm.printState(stream, s2);

                stream << std::endl;
            }
        }
    }

    return stream;
}

///@todo Remove unnecessary epsilon transitions
Fsm Fsm::concatenation(const std::vector<Fsm> &fsms)
{
    std::size_t states_num = 2;

    std::set<symbol_t> alphabet;

    for (const auto &fsm : fsms)
    {
        fsm.ensureAtomic();
        states_num += fsm.getTransitions().size();
        alphabet.insert(fsm.m_alphabet.begin(), fsm.m_alphabet.end());
    }

    Fsm res(states_num);
    res.m_alphabet = alphabet;

    std::size_t global_index = 1;

    state_t global_start = 0;
    state_t global_end = states_num - 1;

    res.setStarting(global_start);
    res.setFinal(global_end);

    state_t prev_end = global_start;

    for (const auto &fsm : fsms)
    {
        auto transitions = fsm.getTransitions();

        for (state_t i = 0; i < transitions.size(); i++)
        {
            for (state_t j = 0; j < transitions.size(); j++)
            {
                res.m_transitions[global_index + i][global_index + j] =
                    transitions[i][j];
            }
        }

        state_t start = *fsm.getStartingStates().begin();
        state_t end = *fsm.getFinalStates().begin();

        res.connect(prev_end, start + global_index, '\0');
        prev_end = end + global_index;

        global_index += transitions.size();
    }

    res.connect(prev_end, global_end, '\0');

    return res;
}

///@todo Boilerplate
Fsm Fsm::disjunction(const std::vector<Fsm> &fsms)
{
    std::size_t states_num = 2;

    std::set<symbol_t> alphabet;

    for (const auto &fsm : fsms)
    {
        fsm.ensureAtomic();
        states_num += fsm.getTransitions().size();
        alphabet.insert(fsm.m_alphabet.begin(), fsm.m_alphabet.end());
    }

    Fsm res(states_num);
    res.m_alphabet = alphabet;

    std::size_t global_index = 1;

    state_t global_start = 0;
    state_t global_end = states_num - 1;

    res.setStarting(global_start);
    res.setFinal(global_end);

    for (const auto &fsm : fsms)
    {
        auto transitions = fsm.getTransitions();

        for (state_t i = 0; i < transitions.size(); i++)
        {
            for (state_t j = 0; j < transitions.size(); j++)
            {
                res.m_transitions[global_index + i][global_index + j] =
                    transitions[i][j];
            }
        }

        state_t start = *fsm.getStartingStates().begin();
        state_t end = *fsm.getFinalStates().begin();

        res.connect(global_start, start + global_index, '\0');
        res.connect(end + global_index, global_end, '\0');

        global_index += transitions.size();
    }

    return res;
}

Fsm Fsm::option(const Fsm &fsm)
{
    fsm.ensureAtomic();

    state_t start = *fsm.getStartingStates().begin();
    state_t end = *fsm.getFinalStates().begin();

    Fsm res = fsm;

    res.connect(start, end, '\0');

    return res;
}

Fsm Fsm::iteration(const Fsm &fsm)
{
    fsm.ensureAtomic();

    state_t start = *fsm.getStartingStates().begin();
    state_t end = *fsm.getFinalStates().begin();

    Fsm res = fsm;

    res.connect(end, start, '\0');

    return res;
}

void Fsm::buildAlphabet()
{
    m_alphabet.clear();

    for (state_t s1 = 0; s1 < m_transitions.size(); s1++)
    {
        for (state_t s2 = 0; s2 < m_transitions.size(); s2++)
        {
            for (symbol_t a : m_transitions[s1][s2])
            {
                if (a)
                {
                    m_alphabet.insert(a);
                }
            }
        }
    }
}

void Fsm::printState(std::ostream &stream, state_t state) const
{
    if (m_starting_states.find(state) != m_starting_states.end())
    {
        stream << "*";
    }
    else
    {
        stream << " ";
    }

    stream << state;

    if (m_final_states.find(state) != m_final_states.end())
    {
        stream << "*";
    }
    else
    {
        stream << " ";
    }
}

std::vector<std::set<Fsm::state_t>> Fsm::epsilonClosures() const
{
    std::vector<std::set<state_t>> closures(m_transitions.size());
    std::vector<bool> flags(m_transitions.size(), false);

    for (state_t s = 0; s < m_transitions.size(); s++)
    {
        closures[s].insert(s);
    }

    for (state_t s = 0; s < m_transitions.size(); s++)
    {
        buildEpsilonClosures(s, closures, flags);
    }

    return closures;
}

void Fsm::buildEpsilonClosures(
    state_t state,
    std::vector<std::set<state_t>> &closures,
    std::vector<bool> &flags) const
{
    if (flags[state])
    {
        return;
    }

    flags[state] = true;

    for (state_t s = 0; s < m_transitions.size(); s++)
    {
        if (m_transitions[state][s].find('\0') != m_transitions[state][s].end())
        {
            buildEpsilonClosures(s, closures, flags);
            closures[state].insert(closures[s].begin(), closures[s].end());
        }
    }
}

///@todo Refactor this
void Fsm::ensureAtomic() const
{
    if (getStartingStates().size() != 1 && getFinalStates().size() != 1)
    {
        throw std::runtime_error("FSM is not atomic");
    }
}

} // namespace fsm
