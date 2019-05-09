#include "../include/module.hpp"
#include "../../Logger/include/logger.hpp"

#define NODE(rule) (std::static_pointer_cast<Node>(rule))
#define ADD_LOG(rule, msg) (logger->add_entity(rule->file, rule->line, rule->column, msg))
#define ADD_NULL_LOG(file_ptr, msg) (logger->add_entity(file_ptr, 0, 0, msg))

// Stores the modules symbol table.
std::unordered_map<std::string, Module> modules;

// Determines the main function name.
static std::string main_fun = "main";

std::shared_ptr<Block> Module::analyze(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, bool require_main)
{
    // Set the AST source.
    this->code = code;
    // Analyze the TLDs.
    this->analyze_tld();
    if (require_main) {
        // Check if the main function exists
        BlockVariableType *var = this->main_block->get_variable(main_fun);
        if (!var || var->type->type != VALUE_FUN) {
            logger->add_entity(this->file, 0, 0, "The main block requires a '" + main_fun + "()' function as the entry point");
            exit(logger->crash());
        }
    }
    // Analyze the code.
    this->analyze_code();
    // Add it to the modules symbol table.
    modules.insert({{ std::string(*this->file), *this }});
    // Return the main block.
    return this->main_block;
}

void Module::analyze_tld(std::shared_ptr<Statement> &tld, bool set_exported)
{
    // List of analyzed files to avoid unesseary work.
    switch (tld->rule) {
        case RULE_USE: {
            std::shared_ptr<Use> use = std::static_pointer_cast<Use>(tld);
            if (modules.find(std::string(*use->module)) == modules.end()) {
                use->block = Module(use->module).analyze(use->code);
            } else {
                use->block = modules.at(std::string(*use->module)).main_block;
            }
            // if (blocks.find(use->module) == blocks.end()) use->block = this->analyze_tld(use->code);
            // else use->block = blocks[use->module];
            // Check if the imports are exported first.
            for (std::string &target : use->targets) {
                BlockVariableType *var = use->block->get_variable(target);
                if (!var) {
                    // Trying to import something that is not declared on that module.
                    ADD_NULL_LOG(use->module, "Importing an unknown target. Make sure your target '" + target + "' is defined in " + *use->module);
                    exit(logger->crash());
                }
                if (!use->block->is_exported(target)) {
                    // Trying to import something that's not exported.
                    ADD_LOG(var->node, "Importing an unexported target. Make sure your export '" + target + "' in " + *use->module);
                    exit(logger->crash());
                }
                // All good. Add the variable to the local block.
                this->main_block->set_variable(target, { var->type, var->node });
            }
            break;
        }
        case RULE_EXPORT: {
            // It still needs to analyze the statement.
            this->analyze_tld(std::static_pointer_cast<Export>(tld)->statement, true);
            break;
        }
        case RULE_CLASS: {
            break;
        }
        case RULE_FUNCTION: {
            // It needs to declare that function to the block.
            std::shared_ptr<Function> fun = std::static_pointer_cast<Function>(tld);
            // Set the variable to the scoped block.
            this->main_block->set_variable(fun->name, { std::make_shared<Type>(fun), NODE(fun), set_exported });
            break;
        }
        default: {
            ADD_LOG(tld, "Invalid TLD rule to analyze");
            exit(logger->crash());
        }
    }
}

void Module::analyze_code(const std::shared_ptr<Expression> &rule)
{
    switch (rule->rule) {
        case RULE_INTEGER:
        case RULE_FLOAT:
        case RULE_STRING:
        case RULE_BOOLEAN: { break; }
        // case RULE_NONE: { break; }
        case RULE_LIST: {
            std::shared_ptr<List> list = std::static_pointer_cast<List>(rule);
            if (list->value.size() == 0) {
                ADD_LOG(list, "You can't setup an empty list. You should declare a variable with it's type and leave the initializer empty");
                exit(logger->crash());
            }
            Type type = Type(list->value[0], &this->blocks);
            if (type.type == VALUE_NO_TYPE) {
                ADD_LOG(list, "You can't setup a list with no-value values. Your list item/s contains no value");
                exit(logger->crash());
            }
            for (size_t i = 1; i < list->value.size(); i++) {
                if (!Type(list->value[i], &this->blocks).same_as(type)) {
                    ADD_LOG(list, "Lists must have the same type. This list can only contain '" + type.to_string() + "' based on the first element");
                    exit(logger->crash());
                }
            }
            list->type = std::make_shared<Type>(rule, &this->blocks);
            break;
        }
        case RULE_DICTIONARY: {
            std::shared_ptr<Dictionary> dict = std::static_pointer_cast<Dictionary>(rule);
            if (dict->value.size() == 0) {
                ADD_LOG(dict, "You can't setup an empty dictionary. You should declare a variable with it's type and leave the initializer empty");
                exit(logger->crash());
            }
            Type type = Type(dict->value[dict->key_order[0]], &this->blocks);
            if (type.type == VALUE_NO_TYPE) {
                ADD_LOG(dict, "You can't setup a dictionary with no-value values. Your dictionary value/s contains no value");
                exit(logger->crash());
            }
            for (size_t i = 1; i < dict->key_order.size(); i++) {
                if (!Type(dict->value[dict->key_order[i]], &this->blocks).same_as(type)) {
                    ADD_LOG(dict->value[dict->key_order[i]], "Dictionaries must have the same type. This dictionary can only contain '" + type.to_string() + "' based on the first element");
                    exit(logger->crash());
                }
            }
            dict->type = std::make_shared<Type>(rule, &this->blocks);
            break;
        }
        case RULE_GROUP: {
            this->analyze_code(std::static_pointer_cast<Group>(rule)->expression);
            break;
        }
        case RULE_CAST: {
            std::shared_ptr<Cast> cast = std::static_pointer_cast<Cast>(rule);
            this->analyze_code(cast->expression);
            Type type = Type(cast->expression, &this->blocks);
            if (!type.cast(cast->type, &cast->cast_type)) {
                ADD_LOG(cast, "Casting from '" + type.to_string() + "' to '" + cast->type->to_string() + "' is not valid");
                exit(logger->crash());
            }
            break;
        }
        case RULE_UNARY: {
            std::shared_ptr<Unary> unary = std::static_pointer_cast<Unary>(rule);
            this->analyze_code(unary->right);
            Type type = Type(unary->right, &this->blocks);
            if (!type.unary(unary->op, nullptr, &unary->type)) {
                // The binary operation cannot be performed.
                ADD_LOG(unary, "Unary expression " + unary->op.to_type_string() + "'" + type.to_string() + "' does not match any known unary operation.");
                exit(logger->crash());
            }
            break;
        }
        case RULE_BINARY: {
            std::shared_ptr<Binary> binary = std::static_pointer_cast<Binary>(rule);
            this->analyze_code(binary->left);
            this->analyze_code(binary->right);
            Type left_type = Type(binary->left, &this->blocks);
            Type right_type = Type(binary->right, &this->blocks);
            if (!left_type.binary(binary->op, &right_type, nullptr, &binary->type)) {
                // The binary operation cannot be performed.
                ADD_LOG(binary, "Binary expression '" + left_type.to_string() + "' " + binary->op.to_type_string() + " '" + right_type.to_string() + "' does not match any known binary operation.");
                exit(logger->crash());
            }
            break;
        }
        case RULE_VARIABLE: {
            // Check if the variable has been declared.
            std::shared_ptr<Variable> var = std::static_pointer_cast<Variable>(rule);
            for (size_t i = this->blocks.size() - 1; i >= 0; i--) {
                BlockVariableType *v = this->blocks[i]->get_variable(var->name);
                if (v) { 
                    // Variable found!
                    // Declare last use.
                    v->last_use = rule;
                    goto rule_variable_ok;
                }
            }
            ADD_LOG(var, "Undeclared variable '" + var->name + "'");
            exit(logger->crash());
            // All ok
            rule_variable_ok:
            break;
        }
        case RULE_ASSIGN: {
            std::shared_ptr<Assign> assign = std::static_pointer_cast<Assign>(rule);
            this->analyze_code(assign->value);
            this->analyze_code(assign->target);
            // Make sure the types match.
            Type vtype = Type(assign->value, &this->blocks);
            Type ttype = Type(assign->target, &this->blocks);
            if (!ttype.same_as(vtype)) {
                ADD_LOG(assign, "Assignment type missmatch. Expected '" + ttype.to_string() + "' but got '" + vtype.to_string() + "'");
                exit(logger->crash());
            }
            break;
        }
        case RULE_LOGICAL: {
            std::shared_ptr<Logical> logical = std::static_pointer_cast<Logical>(rule);
            this->analyze_code(logical->left);
            this->analyze_code(logical->right);
            Type left_type = Type(logical->left, &this->blocks);
            if (left_type.type != VALUE_BOOL ) {
                ADD_LOG(logical->left, "Expected a boolean in the 'logical' left side. Got '" + left_type.to_string() + "'");
                exit(logger->crash());
            }
            Type right_type = Type(logical->right, &this->blocks);
            if (right_type.type != VALUE_BOOL ) {
                ADD_LOG(logical->right, "Expected a boolean in the 'logical' right side. Got '" + right_type.to_string() + "'");
                exit(logger->crash());
            }
            break;
        }
        case RULE_CALL: {
            std::shared_ptr<Call> call = std::static_pointer_cast<Call>(rule);
            this->analyze_code(call->target);
            Type type = Type(call->target, &this->blocks);
            // Check if it's callable.
            if (type.type != VALUE_FUN) {
                ADD_LOG(call, "The call target is not callable. Got '" + type.to_string() + "'");
                exit(EXIT_FAILURE);
            }
            // Check the arguments' type.
            for (size_t i = 0; i < call->arguments.size(); i++) {
                this->analyze_code(call->arguments[i]);
                Type arg = Type(call->arguments[i], &this->blocks);
                if (!arg.same_as(type.parameters[i])) {
                    ADD_LOG(
                        call->arguments[i],
                        "Invalid type in function call. Argument "
                            + std::to_string(i + 1)
                            + " must be "
                            + type.parameters[i]->to_string()
                            + ", got "
                            + arg.to_string()
                    );
                    exit(logger->crash());
                }
            }
            break;
        }
        case RULE_ACCESS: {
            std::shared_ptr<Access> access = std::static_pointer_cast<Access>(rule);
            this->analyze_code(access->index);
            this->analyze_code(access->target);
            Type itype = Type(access->index, &this->blocks);
            Type ttype = Type(access->target, &this->blocks);
            switch (ttype.type) {
                case VALUE_LIST: {
                    // The variable is a list. The index must be an integer.
                    if (itype.type != VALUE_INT) {
                        ADD_LOG(access->index, "List access index must be an integer. Got '" + itype.to_string() + "'");
                        exit(logger->crash());
                    }
                    // All checks passed. We may set the index type and break the switch.
                    access->integer_index = true;
                    break;
                }
                case VALUE_DICT: {
                    // The variable is a list. The index must be an integer.
                    if (itype.type != VALUE_STRING) {
                        ADD_LOG(access->index, "Dictionary access index must be a string. Got '" + itype.to_string() + "'");
                        exit(logger->crash());
                    }
                    // All checks passed. We may set the index type and break the switch.
                    access->integer_index = false;
                    break;
                }
                default: {
                    ADD_LOG(access, "You can't access an inner element on type '" + ttype.to_string() + "'");
                    exit(logger->crash());
                }
            }
            break;
        }
        case RULE_SLICE: {
            std::shared_ptr<Slice> slice = std::static_pointer_cast<Slice>(rule);
            if (slice->start) {
                this->analyze_code(slice->start);
                Type t = Type(slice->start, &this->blocks);
                if (t.type != VALUE_INT) {
                    ADD_LOG(slice->start, "Slice start index must be an 'int'. Got '" + t.to_string() + "'");
                    exit(logger->crash());
                }
            }
            if (slice->end) {
                this->analyze_code(slice->end);
                Type t = Type(slice->end, &this->blocks);
                if (t.type != VALUE_INT) {
                    ADD_LOG(slice->end, "Slice end index must be an 'int'. Got '" + t.to_string() + "'");
                    exit(logger->crash());
                }
            }
            if (slice->step) {
                this->analyze_code(slice->step);
                Type t = Type(slice->step, &this->blocks);
                if (t.type != VALUE_INT) {
                    ADD_LOG(slice->step, "Slice step index must be an 'int'. Got '" + t.to_string() + "'");
                    exit(logger->crash());
                }
            }
            this->analyze_code(slice->target);
            Type t = Type(slice->target, &this->blocks);
            if (t.type != VALUE_LIST && t.type != VALUE_STRING) {
                ADD_LOG(slice->target, "The slice target must be either 'list' or 'string'. Got '" + t.to_string() + "'");
                exit(logger->crash());
            }
            slice->is_list = t.type == VALUE_LIST ? true : false;
            break;
        }
        case RULE_RANGE: {
            std::shared_ptr<Range> range = std::static_pointer_cast<Range>(rule);
            this->analyze_code(range->start);
            this->analyze_code(range->end);
            Type start_type = Type(range->start, &this->blocks);
            if (start_type.type != VALUE_INT) {
                ADD_LOG(range->start, "The range start index must be 'int'. Got '" + start_type.to_string() + "'");
                exit(logger->crash());
            }
            Type end_type = Type(range->end, &this->blocks);
            if (end_type.type != VALUE_INT) {
                ADD_LOG(range->start, "The range end index must be 'int'. Got '" + end_type.to_string() + "'");
                exit(logger->crash());
            }
            break;
        }
        default: {
            ADD_LOG(rule, "Invalid expression to analyze");
            exit(logger->crash());
        }
    }
}

void Module::analyze_tld()
{
    for (std::shared_ptr<Statement> &tld : *this->code) this->analyze_tld(tld, &this->main_block);
}

void Module::analyze_code()
{
    this->blocks.push_back(this->main_block);
    for (std::shared_ptr<Statement> &rule : *this->code) this->analyze_code(rule);
    this->blocks.pop_back();
}

std::shared_ptr<Block> Module::analyze_code(
    const std::vector<std::shared_ptr<Statement>> &code,
    const std::vector<std::shared_ptr<Declaration>> &initializers,
    const std::shared_ptr<Node> &initializer_node
)
{
    std::shared_ptr block = std::make_shared<Block>();
    this->blocks.push_back(block);
    if (initializers.size() > 0) {
        for (const std::shared_ptr<Declaration> &argument : initializers) {
            this->declare(argument, initializer_node);
        }
    }
    // printf("Should be only initializers.\n");
    // block.debug();
    for (const std::shared_ptr<Statement> &rule : code) {
        this->analyze_code(rule);
    }
    this->blocks.pop_back();
    return block;
}

void Module::analyze_code(const std::shared_ptr<Statement> &rule, bool no_declare)
{
    switch (rule->rule) {

        /* Top level declarations */
        case RULE_USE: { break; } // This will be recursively analyzed already...
        case RULE_EXPORT: {
            this->analyze_code(std::static_pointer_cast<Export>(rule)->statement, no_declare);
            break;
        }
        case RULE_CLASS: {
            break;
        }
        case RULE_FUNCTION: {
            std::shared_ptr<Function> fun = std::static_pointer_cast<Function>(rule);
            // Analyze the function parameters.
            for (std::shared_ptr<Declaration> &parameter : fun->parameters) this->analyze_code(std::static_pointer_cast<Statement>(parameter), true);
            // Check if there's a top level return on the function.
            if (fun->return_type) {
                this->return_type = std::make_shared<Type>(*fun->return_type);
                for (std::shared_ptr<Statement> &rule : fun->body) {
                    if (rule->rule == RULE_RETURN) goto continue_rule_function;
                }
                ADD_LOG(fun, "Expected at least 1 function top level return. Returns found inside conditionals or loops are not guaranted to happen.");
                exit(logger->crash());
            } else {
                for (std::shared_ptr<Statement> &rule : fun->body) {
                    if (rule->rule == RULE_RETURN) goto continue_rule_function;
                }
                // Add an ending return because the function didn't have any!
                fun->body.push_back(std::make_shared<Return>(fun->body.back()->file, fun->body.back()->line, fun->body.back()->column));
            }
            continue_rule_function:
            // Analyze the function body.
            fun->block = this->analyze_code(fun->body, fun->parameters);
            this->return_type = nullptr;
            break;
        }

        /* Normal statements */
        case RULE_PRINT: {
            this->analyze_code(std::static_pointer_cast<Print>(rule)->expression);
            break;
        }
        case RULE_EXPRESSION_STATEMENT: {
            this->analyze_code(std::static_pointer_cast<ExpressionStatement>(rule)->expression);
            break;
        }
        case RULE_DECLARATION: {
            std::shared_ptr<Declaration> dec = std::static_pointer_cast<Declaration>(rule);
            if (!dec->type) {
                // dec->initializer MUST be defined because of how parsing works.
                this->analyze_code(dec->initializer);
                // Get the type of the initializer,
                dec->type = std::make_shared<Type>(dec->initializer, &this->blocks);
                if (dec->type->type == VALUE_NO_TYPE) {
                    ADD_LOG(dec->initializer, "You can't declare a variable with no-value values. Your declaration initializer contains no value");
                    exit(logger->crash());
                }
                if (!no_declare) this->declare(dec);
                break;
            }
            if (!no_declare) this->declare(dec);
            if (dec->initializer) {
                this->analyze_code(dec->initializer);
                // Get the type of the initializer,
                Type type = Type(dec->initializer, &this->blocks);
                // Check the types to know if it can be initialized.
                if (!dec->type->same_as(type)) {
                    ADD_LOG(dec,
                        "Incompatible types: Expected '"
                        + dec->type->to_string()
                        + "', got '"
                        + type.to_string()
                        + "'"
                    );
                    exit(logger->crash());
                }
            }
            break;
        }
        case RULE_RETURN: {
            std::shared_ptr<Return> ret = std::static_pointer_cast<Return>(rule);
            if (!ret->value) break;
            this->analyze_code(ret->value);
            if (!this->return_type) {
                ADD_LOG(ret, "Non-situational return. There is no return type expected.");
                exit(logger->crash());
            }
            Type type = Type(ret->value, &this->blocks);
            if (!type.same_as(*this->return_type)) {
                ADD_LOG(ret,
                    "Return type does not match with function type. Expected '"
                    + this->return_type->to_string()
                    + "', got '"
                    + type.to_string()
                    + "'"
                );
                exit(logger->crash());
            }
            break;
        }
        case RULE_IF: {
            std::shared_ptr<If> rif = std::static_pointer_cast<If>(rule);
            this->analyze_code(rif->condition);
            Type type = Type(rif->condition, &this->blocks);
            if (type.type != VALUE_BOOL) {
                ADD_LOG(rif->condition, "Expected a boolean in the 'if' condition. Got '" + type.to_string() + "'");
                exit(logger->crash());
            }
            rif->then_block = this->analyze_code(rif->then_branch);
            rif->else_block = this->analyze_code(rif->else_branch);
            break;
        }
        case RULE_WHILE: {
            std::shared_ptr<While> rwhile = std::static_pointer_cast<While>(rule);
            this->analyze_code(rwhile->condition);
            Type type = Type(rwhile->condition, &this->blocks);
            if (type.type != VALUE_BOOL) {
                ADD_LOG(rwhile->condition, "Expected a boolean in the 'while' condition. Got '" + type.to_string() + "'");
                exit(logger->crash());
            }
            rwhile->block = this->analyze_code(rwhile->body);
            break;
        }
        case RULE_FOR: {
            std::shared_ptr<For> rfor = std::static_pointer_cast<For>(rule);
            this->analyze_code(rfor->iterator);
            std::vector<std::shared_ptr<Declaration>> decs;
            // Declare the value of each iteration.
            std::shared_ptr<Type> vtype = std::make_shared<Type>(rfor->iterator, &this->blocks); // Types (they are saved on the heap since they will be saved)
            // Check iterator.
            if (vtype->type != VALUE_LIST && vtype->type != VALUE_DICT) {
                ADD_LOG(rfor->iterator, "The 'for' iterator is not iterable. It must be either 'list' or 'dictionary' but got '" + vtype->to_string() + "'");
                exit(logger->crash());
            }
            decs.push_back(std::make_shared<Declaration>(
                rfor->file, rfor->line, rfor->column, rfor->variable, vtype, std::shared_ptr<Expression>())
            );
            // Check if it uses the index and declare it if so.
            if (rfor->index != "") {
                std::shared_ptr<Type> itype = std::make_shared<Type>(VALUE_INT); // Types (they are saved on the heap since they will be saved)
                decs.push_back(std::make_shared<Declaration>(
                    rfor->file, rfor->line, rfor->column, rfor->index, itype, std::shared_ptr<Expression>())
                );
            }
            // Analyze the for body.
            rfor->block = this->analyze_code(rfor->body, decs, NODE(rfor));
            break;
        }
        default: {
            ADD_LOG(rule, "Invalid statement to analyze");
            exit(logger->crash());
        }
    }
}

void Module::declare(const std::shared_ptr<Declaration> &dec, const std::shared_ptr<Node> &node)
{
    std::shared_ptr<Block> block = this->blocks.back();
    // Check if the variable exists already.
    if (block->get_variable(dec->name)) {
        ADD_LOG(dec, "The variable '" + dec->name + "' is already declared in this scope.");
        exit(logger->crash());
    }
    // Declare the variable to the current scope.
    block->set_variable(dec->name, { dec->type, node ? node : NODE(dec) });
}
