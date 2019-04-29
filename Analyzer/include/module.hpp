#ifndef MODULE_HPP
#define MODULE_HPP

#include "../../Parser/include/rules.hpp"
#include "../../Parser/include/type.hpp"
#include "../../Parser/include/block.hpp"

// The Module class is used to analyze a module.
class Module
{
    // Stores the file name of that module.
    const std::string *file;
    // Stores the code of that module.
    std::vector<Statement *> *code;
    // Stores the blocks used while analyzing the module.
    std::vector<Block *> blocks;
    // Return variable if needs to be checked. Only 1 can exist since
    // it can only analyze 1 function at a time.
    Type *return_type = nullptr;
    // Analyzes the TLDs given a list of top level delcarations.
    void analyze_tld();
    // Analyzes the given top level declaration.
    void analyze_tld(Statement *tld, Block *block, bool set_exported = false);
    // Analyzes the code.
    void analyze_code();
    // Analyzes the statement.
    void analyze_code(Statement *rule, bool no_declare = false);
    // Analyzes the expression.
    void analyze_code(Expression *rule);
    // Analyzes the block.
    Block analyze_code(std::vector<Statement *> *code, std::vector<Declaration *> *initializers = nullptr, Node *initializer_node = nullptr);
    // Declares a variable to the most top level block.
    void declare(Declaration *dec,  Node *node = nullptr);
    public:
        // Stores the main block of that module.
        Block main_block;
        // Main module constructor
        Module(const std::string *file)
            : file(file) {}
        // Analyzes the module and adds it's entry to the modules symbol table.
        Block analyze(std::vector<Statement *> *code, bool require_main = false);
};

// Module symbol table.
extern std::unordered_map<std::string, Module> modules;

#endif
