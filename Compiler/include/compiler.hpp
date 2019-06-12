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

#include "memory.hpp"
#include "program.hpp"
#include "../../Analyzer/include/analyzer.hpp"

// Base compiler class for nuua.
class Compiler
{
    // Stores the program itself where everything is beeing compiled to.
    std::shared_ptr<Program> program;
    // Stores the current block.
    std::vector<std::shared_ptr<Block>> blocks;
    // Stores the global frame information.
    FrameInfo global;
    // Sotres the current local information (it resets automatically).
    FrameInfo local;
    // Compiles a module.
    void compile_module(const std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const std::shared_ptr<Block> &block);
    // Registers the top level declarations by assigning a register to them.
    // it registers all TLD of all modules in the same main frame info.
    void register_tld(const std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const std::shared_ptr<Block> &block);
    // Compiles a list of statements
    // Defines a basic compilation for a Statement.
    void compile(const std::shared_ptr<Statement> &rule);
    // Defines a basic compilation for an Expression. (Returns the register with the result).
    // If const_opcode is true and the expression is a constant expression, it will move it
    // to a new register. Otherwise, it will just add the constant and the constant index.
    // suggested_register will use that register as the result.
    register_t compile(
        // The expression to compile
        const std::shared_ptr<Expression> &rule,
        // If the load constant opcode shall be included
        const bool load_constant = true,
        // If a register is suggested as the output instead of a new one.
        const register_t *suggested_register = nullptr,
        // If the inner expresion needs to assign rather than get the value.
        const std::shared_ptr<Expression> &assignment_value = std::shared_ptr<Expression>(),
        // It stores the object of the property in case is needed.
        register_t *object_reg = nullptr
    );
    Value compile_function(const std::shared_ptr<Function> &f);
    // Adds an opcode to the program.
    void add_opcodes(const std::vector<opcode_t> &opcodes);
    // Adds a constant to the constant pool of the current frame
    // and return it's position.
    size_t add_constant(const Value &value);
    // Creates a constant list or dictionary from the given list expression.
    // Take into consideration that the list MUST be checked if
    // it's constant using is_constant() function.
    void constant_list(const std::shared_ptr<List> &list, Value &dest);
    void constant_dict(const std::shared_ptr<Dictionary> &dict, Value &dest);
    // Determines if an expression is constant (is in the constant pool).
    bool is_constant(const std::shared_ptr<Expression> &expression);
    // Sets a file flag at the current code location.
    std::shared_ptr<const std::string> current_file;
    void set_file(const std::shared_ptr<const std::string> &file);
    // Sets a line flag at the current code location.
    line_t current_line = 0;
    void set_line(const line_t line);
    // Sets a column flag at the current code location.
    column_t current_column = 0;
    void set_column(const column_t column);
    // Get a variable from the block stack and
    // return a pair containing the variable and a boolean
    // to indicate if it's global or not.
    std::pair<BlockVariableType *, bool> get_variable(const std::string &name);
    BlockClassType *get_class(const std::string &name);
    public:
        // Compile an input source and returns the main global register.
        register_t compile(const char *file);
        Compiler(const std::shared_ptr<Program> &program)
            : program(program) { }
};

#endif
