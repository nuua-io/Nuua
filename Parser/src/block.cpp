#include "../include/block.hpp"
#include "../include/type.hpp"

BlockVariableType *Block::get_variable(std::string &name)
{
    return this->variables.find(name) == this->variables.end()
        ? nullptr
        : &this->variables[name];
}

void Block::set_variable(std::string name, BlockVariableType type)
{
    this->variables[name] = type;
}

bool Block::is_exported(std::string &name)
{
    if (BlockVariableType *var = this->get_variable(name)) {
        return var->exported;
    }

    return false;
}

BlockVariableType *Block::get_single_variable(std::string &name, std::vector<Block *> *blocks)
{
    for (size_t i = blocks->size() - 1; i >= 0; i--) {
        BlockVariableType *res = (*blocks)[i]->get_variable(name);
        if (res) return res;
    }

    return nullptr;
}

void Block::debug()
{
    printf("\nBlock variables: (%d)\n", this->variables.size());
    for (auto &[name, variable] : this->variables) {
        printf("%s: %s", name.c_str(), variable.type->to_string().c_str());
        if (variable.exported) printf(" (exported)\n");
        else printf("\n");
    }
}
