/**
 * |---------------|
 * | Nuua Compiler |
 * |---------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "program.hpp"
#include "../../Analyzer/include/analyzer.hpp"

// Base compiler class for nuua.
class Compiler
{
    // Stores the program itself where everything is beeing compiled to.
    Program program;
    // Stores the current block.
    std::vector<Block *> blocks;
    // Stores the global frame information.
    FrameInfo global;
    // Sotres the current local information (it resets automatically).
    FrameInfo local;
    // Compiles a module.
    void compile_module(std::vector<Statement *> *code, Block *block);
    // Registers the top level declarations by assigning a register to them.
    // it registers all TLD of all modules in the same main frame info.
    void register_tld(std::vector<Statement *> *code, Block *block);
    // Compiles a list of statements
    // Defines a basic compilation for a Statement.
    void compile(Statement *rule);
    // Defines a basic compilation for an Expression. (Returns the register with the result).
    // If const_opcode is true and the expression is a constant expression, it will move it
    // to a new register. Otherwise, it will just add the constant and the constant index.
    // suggested_register will use that register as the result.
    register_t compile(Expression *rule, bool load_constant = true, register_t *suggested_register = nullptr);
    // Adds an opcode to the program.
    void add_opcodes(std::vector<opcode_t> opcodes);
    // Adds a constant to the constant pool of the current frame
    // and return it's position.
    size_t add_constant(Value value);
    // Creates a constant list or dictionary from the given list expression.
    // Take into consideration that the list MUST be checked if
    // it's constant using is_constant() function.
    Value constant_list(List *list);
    Value constant_dict(Dictionary *dict);
    // Determines if an expression is constant (is in the constant pool).
    bool is_constant(Expression *expression);
    // Sets a file flag at the current code location.
    const std::string *current_file;
    void set_file(const std::string *file);
    // Sets a line flag at the current code location.
    line_t current_line;
    void set_line(const line_t line);
    // Sets a column flag at the current code location.
    column_t current_column;
    void set_column(const column_t column);
    // Get a variable from the block stack and
    // return a pair containing the variable and a boolean
    // to indicate if it's global or not.
    std::pair<BlockVariableType *, bool> get_variable(std::string &name);
    public:
        // Compile an input source and returns the result program.
        Program compile(const char *file);
};

#endif
