#include "../include/analyzer.hpp"
#include "../include/type.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"
#include <algorithm>

void Analyzer::analyze(Statement *rule)
{
    switch (rule->rule) {
        case RULE_PRINT: {
            this->analyze(static_cast<Print *>(rule)->expression);
            break;
        }
        case RULE_EXPRESSION_STATEMENT: {
            this->analyze(static_cast<ExpressionStatement *>(rule)->expression);
            break;
        }
        case RULE_DECLARATION: {
            auto dec = static_cast<Declaration *>(rule);
            this->declare(dec->name, dec->type, dec->initializer);
            this->analyze(dec->initializer);
            // Get the type of the initializer,
            auto type = Type::get_string(dec->initializer);
            // Replace the variables type of the type with the current block.
            this->replace_types(type);

            // Check the types to know if it can be initialized.
            if (dec->type != type) {
                logger->error(
                    "Incompatible types: Need "
                    + dec->type
                    + ", got "
                    + type
                );
                exit(EXIT_FAILURE);
            }
            break;
        }
        case RULE_RETURN: {
            this->analyze(static_cast<Return *>(rule)->value);
            break;
        }
        case RULE_IF: {
            auto rif = static_cast<If *>(rule);
            this->analyze(rif->condition);
            this->analyze(rif->thenBranch);
            this->analyze(rif->elseBranch);
            break;
        }
        case RULE_WHILE: {
            auto rwhile = static_cast<While *>(rule);
            this->analyze(rwhile->condition);
            this->analyze(rwhile->body);
            break;
        }
        default: {
            logger->error("Invalid statement.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
}

void Analyzer::analyze(Expression *rule)
{
    switch (rule->rule) {
        case RULE_INTEGER:
        case RULE_FLOAT:
        case RULE_STRING:
        case RULE_BOOLEAN:
        case RULE_LIST:
        case RULE_DICTIONARY:
        case RULE_NONE: { break; }
        case RULE_GROUP: {
            this->analyze(static_cast<Group *>(rule)->expression);
            break;
        }
        case RULE_CAST: {
            auto cast = static_cast<Cast *>(rule);
            this->analyze(cast->expression);
            break;
        }
        case RULE_UNARY: {
            auto unary = static_cast<Unary *>(rule);
            this->analyze(unary->right);
            break;
        }
        case RULE_BINARY: {
            auto binary = static_cast<Binary *>(rule);
            this->analyze(binary->left);
            this->analyze(binary->right);
            auto left_type = Type::get_string(binary->left);
            auto right_type = Type::get_string(binary->right);
            this->replace_types(left_type);
            this->replace_types(right_type);
            if (left_type != right_type) {
                logger->error(
                    "Binary expression does not have the same type on both sides. "
                    + left_type + " vs " + right_type
                );
                exit(EXIT_FAILURE);
            }
            break;
        }
        case RULE_VARIABLE: {
            // Check if the variable has been declared.
            auto var = static_cast<Variable *>(rule);
            this->must_have(var->name, var->line);
            break;
        }
        case RULE_ASSIGN: {
            auto assign = static_cast<Assign *>(rule);
            this->must_have(assign->name, assign->line);
            this->analyze(assign->value);
            break;
        }
        case RULE_ASSIGN_ACCESS: {
            auto assign_access = static_cast<AssignAccess *>(rule);
            this->analyze(assign_access->value);
            this->must_have(assign_access->name, assign_access->line);
            this->analyze(assign_access->index);
            break;
        }
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            this->analyze(logical->left);
            this->analyze(logical->right);
            break;
        }
        case RULE_FUNCTION: {
            auto function = static_cast<Function *>(rule);
            this->analyze(function->body, function->arguments, function->return_type);
            break;
        }
        case RULE_CALL: {
            auto call = static_cast<Call *>(rule);
            this->must_have(call->callee, call->line);
            for (size_t i = 0; i < call->arguments.size(); i++) {
                this->analyze(call->arguments[i]);
                auto arg = Type::get_string(call->arguments[i]);
                this->replace_types(arg);
                if (this->blocks.back().variables[call->callee].arguments[i] != arg) {
                    logger->error(
                        "Invalid type in function call. Argument "
                        + std::to_string(i + 1)
                        + " in "
                        + call->callee
                        + " must be "
                        + this->blocks.back().variables[call->callee].arguments[i]
                        + ", got "
                        + arg
                    );
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
        case RULE_ACCESS: {
            auto access = static_cast<Access *>(rule);
            this->must_have(access->name, access->line);
            this->analyze(access->index);
            break;
        }
        default: {
            logger->error("Invalid expression.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
}

void Analyzer::analyze(std::vector<Statement *> block, std::vector<Statement *> arguments, std::string return_type)
{
    // Push a new block to the analyzed blocks.
    this->blocks.push_back(Block());

    // Push the arguments if any.
    // The parser makes sure it's a declaration.
    for (auto argument : arguments) {
        auto dec = static_cast<Declaration *>(argument);
        this->declare(dec->name, dec->type, dec->initializer);
    }

    // Analyze the block.
    if (return_type != "") {
        for (auto stmt : block) {
            this->analyze(stmt);
            // Check if it's a return statement.
            if (stmt->rule == RULE_RETURN) {
                auto ret_type = Type::get_string(static_cast<Return *>(stmt)->value);
                this->replace_types(ret_type);
                if (return_type != ret_type) {
                    logger->error("Function return value expects " + return_type + ", but got " + ret_type);
                    exit(EXIT_FAILURE);
                }
            }
        }
    } else for (auto stmt : block) this->analyze(stmt);

    // Pop back the block since it's already analyzed.
    this->blocks.pop_back();
}

void Analyzer::must_have(std::string name, uint32_t line)
{
    int i;
    for (
        i = this->blocks.size() - 1;
        i >= 0 && this->blocks[i].variables.find(name) == this->blocks[i].variables.end();
        i--
    );
    if (i == -1) {
        logger->error("Undeclared variable", line);
        exit(EXIT_FAILURE);
    }
}

void Analyzer::declare(std::string name, std::string type, Expression *initializer)
{
    auto block = &this->blocks.back();

    // Check if the variable exists already.
    if (block->variables.find(name) != block->variables.end()) {
        logger->error("The variable '" + name + "' is already declared in this block scope.");
        exit(EXIT_FAILURE);
    }

    // Set the return type if it's a function value.
    std::string return_type = "?";
    std::vector<std::string> arguments;
    if (initializer && initializer->rule == RULE_FUNCTION) {
        auto fun = static_cast<Function *>(initializer);
        return_type = fun->return_type;
        for (auto argument : fun->arguments) arguments.push_back(static_cast<Declaration *>(argument)->type);
    }

    // Declare the variable with the given type.
    block->variables[name] = BlockVariableType(type, arguments, return_type);
}

void Analyzer::replace_types(std::string &dest)
{
    std::vector<std::string> done;
    for (int64_t i = this->blocks.size() - 1; i >= 0; i--) {
        for (auto &[key, value] : this->blocks[i].variables) {
            if (std::find(done.begin(), done.end(), key) == done.end()) {
                // <var> - Replaces <var> with the type of var.
                this->find_replace(dest, "<" + key + ">", value.type);
                // <:var:> - Replaces <:var:> with the return type of var.
                this->find_replace(dest, "<:" + key + ":>", value.return_type);
                // <[var]> - Replaces <[var]> with the inner type of var.
                size_t inpos = value.type.find('[');
                if (inpos != std::string::npos) {
                    // There is an inner type
                    // Get the closing ]. It must
                    // be the last ] found since we found
                    // the first [.
                    this->find_replace(dest, "<[" + key + "]>", value.type.substr(inpos + 1, value.type.rfind(']') - inpos - 1));
                };
                done.push_back(key);
            }
        }
    }
    // <type=type> - Replaces <type=type> with type and checks if they match
    for (;;) {
        size_t pos = dest.find('=');
        if (pos == std::string::npos) break;
        // Find the initial <
        size_t current_pos, skip;
        for (
            current_pos = pos, skip = 0;
            skip == 0 && dest[current_pos] != '<';
            current_pos--
        ) {
            if (dest[current_pos] == '>') skip++;
            else if (dest[current_pos] == '<') skip--;
        }

        auto first_type = dest.substr(current_pos + 1, (pos - current_pos - 1));
        if (
            dest.length() - 1 < (pos - current_pos) * 2
            || first_type != dest.substr(pos + 1, (pos - current_pos - 1))
        ) {
            logger->info(dest);
            logger->error(
                "Incompatible types for binary expression. "
                + first_type + " vs " + dest.substr(pos + 1, (pos - current_pos - 1))
            );
            exit(EXIT_FAILURE);
        }

        // Replace the substring.
        dest.replace(current_pos, ((pos - current_pos) * 2) + 1, first_type);
    }
}

void Analyzer::find_replace(std::string &dest, const std::string &find, const std::string &replace)
{
    size_t pos = 0;
    while ((pos = dest.find(find, pos)) != std::string::npos) {
         dest.replace(pos, find.length(), replace);
         pos += replace.length();
    }
}

Analyzer *Analyzer::analyze(const char *source)
{
    this->code = Parser().parse(source);

    logger->info("Started analyzing...");

    this->analyze(this->code);

    logger->success("Analyzis complete...");

    return this;
}

Analyzer *Analyzer::optimize()
{
    /* For future use */

    return this;
}
