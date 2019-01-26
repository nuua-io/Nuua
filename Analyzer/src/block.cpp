#include "../include/block.hpp"

BlockVariableType *Block::get_variable(std::string &name)
{
    for (auto &[key, value] : this->variables) {
        if (name == key) return &value;
    }

    return nullptr;
}
