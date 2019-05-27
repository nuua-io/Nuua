#ifndef MODULE_HPP
#define MODULE_HPP

#include "../../Parser/include/rules.hpp"
#include "../../Parser/include/type.hpp"
#include "../../Parser/include/block.hpp"

// The Module class is used to analyze a module.
class Module
{
    // Stores the file name of that module.
    std::shared_ptr<const std::string> file;
    // Stores the code of that module.
    std::shared_ptr<std::vector<std::shared_ptr<Statement>>> code;
    // Stores the blocks used while analyzing the module.
    std::vector<std::shared_ptr<Block>> blocks;
    // Return variable if needs to be checked. Only 1 can exist since
    // it can only analyze 1 function at a time.
    std::shared_ptr<Type> return_type;
    // Analyzes the TLDs given a list of top level delcarations.
    void analyze_tld();
    // Analyzes the given top level declaration.
    void analyze_tld(std::shared_ptr<Statement> &tld, bool set_exported = false);
    // Analyzes the code.
    void analyze_code();
    // Analyzes the statement.
    void analyze_code(const std::shared_ptr<Statement> &rule, bool no_declare = false);
    // Analyzes the expression.
    void analyze_code(const std::shared_ptr<Expression> &rule, const bool allowed_noreturn_call = false);
    // Analyzes the block.
    std::shared_ptr<Block> analyze_code(
        const std::vector<std::shared_ptr<Statement>> &code,
        const std::vector<std::shared_ptr<Declaration>> &initializers = std::vector<std::shared_ptr<Declaration>>(),
        const std::shared_ptr<Node> &initializer_node = std::shared_ptr<Node>()
    );
    // Declares a variable to the most top level block.
    void declare(const std::shared_ptr<Declaration> &dec, const std::shared_ptr<Node> &node = std::shared_ptr<Node>());
    public:
        // Stores the main block of that module.
        std::shared_ptr<Block> main_block = std::make_shared<Block>();
        // Main module constructor
        Module(std::shared_ptr<const std::string> &file)
            : file(file) {}
        // Analyzes the module and adds it's entry to the modules symbol table.
        std::shared_ptr<Block> analyze(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, bool require_main = false);
};

// Module symbol table.
extern std::unordered_map<std::string, Module> modules;

#endif
