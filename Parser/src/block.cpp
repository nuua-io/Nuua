#include "../include/block.hpp"
#include "../include/type.hpp"

BlockVariableType *Block::get_variable(const std::string &name)
{
    return this->variables.find(name) == this->variables.end()
        ? nullptr
        : &this->variables.at(name);
}

void Block::set_variable(const std::string &name, const BlockVariableType &var)
{
    this->variables[name] = std::move(var);
}

bool Block::is_exported(const std::string &name)
{
    if (BlockVariableType *var = this->get_variable(name)) {
        return var->exported;
    }

    return false;
}

bool Block::has(const std::string &name)
{
    return static_cast<bool>(this->get_variable(name));
}

BlockVariableType *Block::get_single_variable(const std::string &name, const std::vector<std::shared_ptr<Block>> *blocks)
{
    for (size_t i = blocks->size() - 1; i >= 0; i--) {
        BlockVariableType *res = (*blocks)[i]->get_variable(name);
        if (res) return res;
        else if (i == 0) return nullptr;
    }

    return nullptr; // Compiler warning... Totally useless.
}

void Block::debug() const
{
    printf("-----------\n-> Block variables: (%zu)\n", this->variables.size());
    for (const auto &[name, variable] : this->variables) {
        printf(
            "G-%05zu -> %s%s %s\n",
            variable.reg,
            name.c_str(),
            variable.exported ? "*:" : ":",
            variable.type->to_string().c_str()
        );
    }
    printf("\n-> Block classes: (%zu)\n", this->classes.size());
    for (const auto &[name, c] : this->classes) {
        printf(
            "%s -> %p\n",
            name.c_str(),
            c.block.get()
        );
    }
    printf("-----------\n");
}

BlockClassType *Block::get_class(const std::string &name)
{
    for (auto &[c, ct] : this->classes) {
        // printf("Checking: %s vs %s\n", c.substr(c.rfind(":") + 1).c_str(), name.c_str());
        if (c.substr(c.rfind(":") + 1) == name) return &ct;
    }
    return nullptr;
}

void Block::set_class(const std::string &name, const BlockClassType &c)
{
    this->classes[name] = std::move(c);
}

bool Block::is_exported_class(const std::string &name)
{
    if (BlockClassType *var = this->get_class(name)) {
        return var->exported;
    }

    return false;
}

bool Block::has_class(const std::string &name)
{
    return static_cast<bool>(this->get_class(name));
}
