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
    void analyze_tld(const std::shared_ptr<Statement> &tld, const bool set_exported = false);
    // Analyzes the class top level declarations.
    // void analyze_class_tld(const std::shared_ptr<Class> &c);
    void analyze_class_tld(const std::shared_ptr<Statement> &tld, const std::shared_ptr<Block> &block);
    // Analyzes the code.
    void analyze_code();
    // Analyzes the statement.
    void analyze_code(const std::shared_ptr<Statement> &rule, bool no_declare = false);
    // Analyzes the expression.
    void analyze_code(
        const std::shared_ptr<Expression> &rule,
        const bool allowed_noreturn_call = false,
        // Single parameter to indicate if methods are allowed
        // and if they are, it stores if the function is a method or not.
        bool *allow_method_and_save_is_method = nullptr
    );
    // Analyzes the block.
    std::shared_ptr<Block> analyze_code(
        const std::vector<std::shared_ptr<Statement>> &code,
        const std::vector<std::shared_ptr<Declaration>> &initializers = std::vector<std::shared_ptr<Declaration>>(),
        const std::shared_ptr<Node> &initializer_node = std::shared_ptr<Node>()
    );
    // Analyzes a function.
    void analyze_function(
        const std::shared_ptr<FunctionValue> &fun,
        const std::vector<std::shared_ptr<Declaration>> &additionals = std::vector<std::shared_ptr<Declaration>>()
    );
    // Declares a variable to the most top level block.
    void declare(const std::shared_ptr<Declaration> &dec, const std::shared_ptr<Node> &node = std::shared_ptr<Node>());
    // Check if the given module have all the classes defined.
    void check_classes(const std::vector<std::string> &classes, const std::shared_ptr<Node> &fail_at);
    public:
        // Stores the main block of that module.
        std::shared_ptr<Block> main_block = std::make_shared<Block>();
        // Main module constructor
        Module(std::shared_ptr<const std::string> &file)
            : file(file) {}
        // Analyzes the module and adds it's entry to the modules symbol table.
        std::shared_ptr<Block> analyze(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const bool require_main = false);
};

// Module symbol table.
extern std::unordered_map<std::string, Module> modules;

#endif
