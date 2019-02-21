#include "../include/block.hpp"

BlockVariableType *Block::get_variable(std::string &name)
{
    return this->variables.find(name) == this->variables.end()
        ? nullptr
        : &this->variables[name];
}
