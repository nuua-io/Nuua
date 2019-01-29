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
            auto type = Type(dec->initializer, &this->blocks);

            // Check the types to know if it can be initialized.
            if (!Type(dec->type).same_as(&type)) {
                logger->error(
                    "Incompatible types: Need "
                    + Type(dec->type).to_string()
                    + ", got "
                    + type.to_string()
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
        case RULE_NONE: { break; }
        case RULE_LIST: {
            auto list = static_cast<List *>(rule);
            if (list->value.size() == 0) {
                logger->error("You can't setup an empty list. You should declare a variable with it's type and leave the initializer empty.");
                exit(EXIT_FAILURE);
            }
            auto type = Type(list->value[0], &this->blocks);
            for (size_t i = 1; i < list->value.size(); i++) {
                if (!Type(list->value[i], &this->blocks).same_as(&type)) {
                    logger->error("Multiple types in a list are not supported.");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
        case RULE_DICTIONARY: {
            auto dict = static_cast<Dictionary *>(rule);
            if (dict->value.size() == 0) {
                logger->error("You can't setup an empty dictionary. You should declare a variable with it's type and leave the initializer empty.");
                exit(EXIT_FAILURE);
            }
            auto type = Type(dict->value[dict->key_order[0]], &this->blocks);
            for (size_t i = 1; i < dict->key_order.size(); i++) {
                if (!Type(dict->value[dict->key_order[i]], &this->blocks).same_as(&type)) {
                    logger->error("Multiple types in a dictionary are not supported.");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
        case RULE_GROUP: {
            this->analyze(static_cast<Group *>(rule)->expression);
            break;
        }
        case RULE_CAST: {
            this->analyze(static_cast<Cast *>(rule)->expression);
            break;
        }
        case RULE_UNARY: {
            this->analyze(static_cast<Unary *>(rule)->right);
            break;
        }
        case RULE_BINARY: {
            auto binary = static_cast<Binary *>(rule);
            this->analyze(binary->left);
            this->analyze(binary->right);
            auto left_type = Type(binary->left, &this->blocks);
            auto right_type = Type(binary->right, &this->blocks);
            if (!left_type.same_as(&right_type)) {
                logger->error(
                    "Binary expression does not have the same type on both sides. "
                    + left_type.to_string() + " vs " + right_type.to_string()
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
            auto var = this->must_have(assign->name, assign->line);
            this->analyze(assign->value);
            // Make sure the types match.
            auto type = Type(assign->value, &this->blocks);
            if (!Type(var->type).same_as(&type)) {
                logger->error("Assignment type missmatch. Expected " + var->type + " but got " + type.to_string());
                exit(EXIT_FAILURE);
            }
            break;
        }
        case RULE_ASSIGN_ACCESS: {
            auto assign_access = static_cast<AssignAccess *>(rule);
            this->analyze(assign_access->value);
            auto var = this->must_have(assign_access->name, assign_access->line);
            this->analyze(assign_access->index);
            auto var_type = Type(var->type);
            if (var_type.type == VALUE_LIST) {
                // The variable is a list. The index must be an integer.
                auto t = Type(assign_access->index, &this->blocks);
                if (t.type != VALUE_INT) {
                    logger->error("List access index must be an integer. Found " + t.to_string());
                    exit(EXIT_FAILURE);
                }
                // All checks passed. We may set the index type and break the switch.
                assign_access->integer_index = true;
                break;
            } else if (var_type.type == VALUE_DICT) {
                // The variable is a dict. The index must be a string.
                auto t = Type(assign_access->index, &this->blocks);
                if (t.type != VALUE_STRING) {
                    logger->error("Dictionary access index must be a string. Found " + t.to_string());
                    exit(EXIT_FAILURE);
                }
                // All checks passed. We may set the index type and break the switch.
                assign_access->integer_index = false;
                break;
            }
            logger->error("You can't access an inner element on " + var_type.to_string());
            exit(EXIT_FAILURE);
            break;
        }
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            this->analyze(logical->left);
            this->analyze(logical->right);
            auto left_type = Type(logical->left, &this->blocks);
            auto right_type = Type(logical->right, &this->blocks);
            if (!left_type.same_as(&right_type)) {
                logger->error(
                    "Logical expression does not have the same type on both sides. "
                    + left_type.to_string() + " vs " + right_type.to_string()
                );
                exit(EXIT_FAILURE);
            }
            break;
        }
        case RULE_FUNCTION: {
            auto function = static_cast<Function *>(rule);
            this->analyze(function->body, function->arguments, function->return_type);
            break;
        }
        case RULE_CALL: {
            auto call = static_cast<Call *>(rule);
            auto var = this->must_have(call->callee, call->line);

            // Check if it's callable.
            if (Type(var->type).type != VALUE_FUN) {
                logger->error("The variable " + call->callee + " is not callable.");
                exit(EXIT_FAILURE);
            }

            // Check the arguments' type.
            for (size_t i = 0; i < call->arguments.size(); i++) {
                this->analyze(call->arguments[i]);
                auto arg = Type(call->arguments[i], &this->blocks).to_string();
                if (var->arguments[i] != arg) {
                    logger->error(
                        "Invalid type in function call. Argument "
                        + std::to_string(i + 1)
                        + " in "
                        + call->callee
                        + " must be "
                        + var->arguments[i]
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
            auto variable = this->must_have(access->name, access->line);
            this->analyze(access->index);
            auto var_type = Type(variable->type);
            if (var_type.type == VALUE_LIST) {
                // The variable is a list. The index must be an integer.
                auto t = Type(access->index, &this->blocks);
                if (t.type != VALUE_INT) {
                    logger->error("List access index must be an integer. Found " + t.to_string());
                    exit(EXIT_FAILURE);
                }
                // All checks passed. We may set the index type and break the switch.
                access->integer_index = true;
                break;
            } else if (var_type.type == VALUE_DICT) {
                // The variable is a dict. The index must be a string.
                auto t = Type(access->index, &this->blocks);
                if (t.type != VALUE_STRING) {
                    logger->error("Dictionary access index must be a string. Found " + t.to_string());
                    exit(EXIT_FAILURE);
                }
                // All checks passed. We may set the index type and break the switch.
                access->integer_index = false;
                break;
            }
            logger->error("You can't access an inner element on " + var_type.to_string());
            exit(EXIT_FAILURE);
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
                auto ret_type = Type(static_cast<Return *>(stmt)->value, &this->blocks);
                if (!Type(return_type).same_as(&ret_type)) {
                    logger->error("Function return value expects " + return_type + ", but got " + ret_type.to_string());
                    exit(EXIT_FAILURE);
                }
            }
        }
    } else for (auto stmt : block) this->analyze(stmt);

    // Pop back the block since it's already analyzed.
    this->blocks.pop_back();
}

BlockVariableType *Analyzer::must_have(std::string name, uint32_t line)
{
    for (int16_t i = this->blocks.size() - 1; i >= 0; i--) {
        auto var = this->blocks[i].get_variable(name);
        if (var != nullptr) return var;
    }

    logger->error("Undeclared variable", line);
    exit(EXIT_FAILURE);
}

void Analyzer::declare(std::string name, std::string type, Expression *initializer)
{
    auto block = &this->blocks.back();

    // Check if the variable exists already.
    for (int i = this->blocks.size() - 1; i >= 0; i--) {
        if (this->blocks[i].variables.find(name) != this->blocks[i].variables.end()) {
            logger->error("The variable '" + name + "' is already declared in this scope.");
            exit(EXIT_FAILURE);
        }
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
