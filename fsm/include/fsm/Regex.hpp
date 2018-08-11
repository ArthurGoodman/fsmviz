#pragma once

#include <memory>
#include <string>

namespace fsm {

class Fsm;
class RegexImpl;

class Regex final
{
public: // methods
    Regex(const std::string &pattern);
    bool match(const std::string &str);

    static Fsm buildFsm(const std::string &pattern);

private: // fields
    std::unique_ptr<RegexImpl> m_impl;
};

} // namespace fsm
