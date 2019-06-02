#include "../include/module.hpp"
#include "../../Logger/include/logger.hpp"

#define NODE(rule) (std::static_pointer_cast<Node>(rule))
#define ADD_LOG(rule, msg) (logger->add_entity(rule->file, rule->line, rule->column, msg))
#define ADD_NULL_LOG(file_ptr, msg) (logger->add_entity(file_ptr, 0, 0, msg))
#define MOD(file) (*file + ":")

// Stores the modules symbol table.
std::unordered_map<std::string, Module> modules;

// Determines the main function name.
static std::string main_fun = "main";

std::shared_ptr<Block> Module::analyze(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const bool require_main)
{
    // Set the AST source.
    this->code = code;
    // Analyze the TLDs.
    this->analyze_tld();
    if (require_main) {
        // Check if the main function exists
        BlockVariableType *var = this->main_block->get_variable(main_fun);
        if (!var || var->type->type != VALUE_FUN) {
            logger->add_entity(this->file, 0, 0, "The main module requires a '" + main_fun + "()' function as the entry point");
            exit(logger->crash());
        }
        // Check if the main function have the required argument of type [string]
        if (var->type->parameters.size() != 1LL) {
            logger->add_entity(this->file, 0, 0, "The main function requires a single parameter of type '[string]'.");
            exit(logger->crash());
        }
        if (var->type->parameters[0]->type != VALUE_LIST || var->type->parameters[0]->inner_type->type != VALUE_STRING) {
            logger->add_entity(var->node->file, var->node->line, var->node->column, "The main function requires a single parameter of type '[string]'.");
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

void Module::analyze_tld(const std::shared_ptr<Statement> &tld, const bool set_exported)
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
            if (use->targets.size() > 0) {
                // Import the requested exported fields.
                for (const std::string &target : use->targets) {
                    BlockVariableType *var = use->block->get_variable(target);
                    BlockClassType *c = use->block->get_class(MOD(use->module) + target);
                    if (!var && !c) {
                        // Trying to import something that is not declared on that module.
                        ADD_NULL_LOG(use->module, "Importing an unknown target. Make sure your target '" + target + "' is defined in " + *use->module);
                        exit(logger->crash());
                    }
                    if (var) {
                        // It's a function.
                        if (!var->exported) {
                            // Trying to import something that's not exported.
                            ADD_LOG(var->node, "Importing an unexported function. Make sure to export '" + target + "' in " + *use->module);
                            exit(logger->crash());
                        }
                        // All good. Add the variable to the local block.
                        this->main_block->set_variable(target, { var->type, var->node });
                    } else {
                        // It's a class.
                        if (!c->exported) {
                            // Trying to import something that's not exported.
                            ADD_LOG(c->node, "Importing an unexported class. Make sure to export '" + target + "' in " + *use->module);
                            exit(logger->crash());
                        }
                        // All good. Add the variable to the local block.
                        this->main_block->set_class(MOD(use->file) + target, { c->block, c->node });
                    }
                }
            } else {
                // Import all exported fields.
                for (const auto &[name, type] : use->block->variables) {
                    // Check if it's exported before importing.
                    if (type.exported) this->main_block->set_variable(name, { type.type, type.node });
                }
            }
            break;
        }
        case RULE_EXPORT: {
            // It still needs to analyze the statement.
            this->analyze_tld(std::static_pointer_cast<Export>(tld)->statement, true);
            break;
        }
        case RULE_CLASS: {
            const std::shared_ptr<Class> c = std::static_pointer_cast<Class>(tld);
            // Create the class block.
            c->block = std::make_shared<Block>();
            const std::string class_name = *c->file + ":" + c->name;
            if (this->main_block->has_class(class_name)) {
                ADD_LOG(c, "The class '" + c->name + "' was already declared in this module.");
                exit(logger->crash());
            }
            // Define that class type.
            this->main_block->set_class(
                class_name,
                { c->block, NODE(c), set_exported }
            );
            // Analyze the class body tld.
            for (const std::shared_ptr<Statement> &tld : c->body) this->analyze_class_tld(tld, c->block);
            break;
        }
        case RULE_FUNCTION: {
            // It needs to declare that function to the block.
            const std::shared_ptr<Function> f = std::static_pointer_cast<Function>(tld);
            const std::shared_ptr<FunctionValue> &fun = f->value;
            // Set the variable to the scoped block.
            // printf("Fun (exported: %d) -> %s\n", set_exported, fun->name.c_str());
            if (this->main_block->has(fun->name)) {
                ADD_LOG(fun, "'" + fun->name + "' was already declared. Function overloading is not currently implemented in this version of nuua.");
                exit(logger->crash());
            }
            this->main_block->set_variable(fun->name, { std::make_shared<Type>(f), NODE(fun), set_exported });
            break;
        }
        default: {
            ADD_LOG(tld, "Invalid TLD rule to analyze");
            exit(logger->crash());
        }
    }
}

void Module::analyze_class_tld(const std::shared_ptr<Statement> &tld, const std::shared_ptr<Block> &block)
{
    switch (tld->rule) {
        case RULE_DECLARATION: {
            std::shared_ptr<Declaration> dec = std::static_pointer_cast<Declaration>(tld);
            if (dec->initializer) {
                // A variable in a class can only be declared by type. No initializers allowed.
                ADD_LOG(dec, "Declarations in class members do not allow initializors.");
                exit(logger->crash());
            }
            if (!dec->type) {
                // A variable in a class can only be declared by type. No initializers allowed.
                ADD_LOG(dec, "Declarations in class members require an explicit type.");
                exit(logger->crash());
            }
            if (block->has(dec->name)) {
                ADD_LOG(dec, "The variable '" + dec->name + "' is already declared in this scope.");
                exit(logger->crash());
            }
            // Check if the type is correct.
            this->check_classes(dec->type->classes_used(MOD(dec->file)), NODE(dec));
            // Set the variable to the class block.
            block->set_variable(dec->name, { dec->type, NODE(dec) });
            break;
        }
        case RULE_FUNCTION: {
            // It needs to declare that function to the block.
            const std::shared_ptr<Function> f = std::static_pointer_cast<Function>(tld);
            const std::shared_ptr<FunctionValue> &fun = f->value;
            // Set the variable to the scoped block.
            // printf("Fun (exported: %d) -> %s\n", set_exported, fun->name.c_str());
            if (block->has(fun->name)) {
                ADD_LOG(fun, "'" + fun->name + "' was already declared. Function overloading is not currently implemented in this version of nuua.");
                exit(logger->crash());
            }
            block->set_variable(fun->name, { std::make_shared<Type>(f), NODE(fun) });
            break;
        }
        default: {
            ADD_LOG(tld, "Invalid class top level declaration to analyze");
            exit(logger->crash());
        }
    }
}

void Module::analyze_code(const std::shared_ptr<Expression> &rule, const bool allowed_noreturn_call)
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
            this->analyze_code(list->value[0]);
            Type type = Type(list->value[0], &this->blocks);
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
            this->analyze_code(dict->value[dict->key_order[0]]);
            Type type = Type(dict->value[dict->key_order[0]], &this->blocks);
            for (size_t i = 1; i < dict->key_order.size(); i++) {
                if (!Type(dict->value[dict->key_order[i]], &this->blocks).same_as(type)) {
                    ADD_LOG(dict->value[dict->key_order[i]], "Dictionaries must have the same type. This dictionary can only contain '" + type.to_string() + "' based on the first element");
                    exit(logger->crash());
                }
            }
            dict->type = std::make_shared<Type>(rule, &this->blocks);
            break;
        }
        case RULE_OBJECT: {
            std::shared_ptr<Object> object = std::static_pointer_cast<Object>(rule);
            this->check_classes({{ MOD(object->file) + object->name }}, NODE(object));
            // Get the class block.
            const std::shared_ptr<Block> &block = this->main_block->get_class(MOD(object->file) + object->name)->block;
            // Check the arguments to initialize the class.
            for (const auto &[key, arg] : object->arguments) {
                BlockVariableType *var;
                if (!(var = block->get_variable(key))) {
                    ADD_LOG(arg, "The class '" + object->name + "' does not have a '" + key + "' field.");
                    exit(logger->crash());
                }
                // Analyze the argument.
                this->analyze_code(arg);
                // Check if the types match.
                Type t = Type(arg, &this->blocks);
                if (!var->type->same_as(t)) {
                    ADD_LOG(arg, "Type missmatch. Expected '" + var->type->to_string() + "' but got '" + t.to_string() + "'");
                    exit(logger->crash());
                }
            }
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
                    break;
                } else if (i == 0) {
                    ADD_LOG(var, "Undeclared variable '" + var->name + "'");
                    exit(logger->crash());
                }
            }
            break;
        }
        case RULE_ASSIGN: {
            std::shared_ptr<Assign> assign = std::static_pointer_cast<Assign>(rule);
            // Analyze both parts.
            this->analyze_code(assign->value);
            this->analyze_code(assign->target);
            // Set the is_access field.
            assign->is_access = assign->target->rule == RULE_ACCESS;
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
            // Assign the has_return prop in the call.
            call->has_return = static_cast<bool>(type.inner_type); // If the inner type if set it means there's a return value.
            // Check if this call is allowed.
            if (!call->has_return && !allowed_noreturn_call) {
                // This call is not allowed.
                ADD_LOG(call, "Calling a function with no return value is only allowed as a block statement, it can't be used as an expression.");
                exit(logger->crash());
            }
            // Check the arguments' type.
            if (call->arguments.size() != type.parameters.size()) {
                ADD_LOG(
                    call,
                    "Function arguments does not match with the definition. Expected "
                    + std::to_string(type.parameters.size())
                    + " arguments, but got "
                    + std::to_string(call->arguments.size())
                );
                exit(logger->crash());
            }
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
                case VALUE_STRING: {
                    // The variable is a list. The index must be an integer.
                    if (itype.type != VALUE_INT) {
                        ADD_LOG(access->index, "String access index must be an integer. Got '" + itype.to_string() + "'");
                        exit(logger->crash());
                    }
                    // All checks passed. We may set the index type and break the switch.
                    access->type = ACCESS_STRING;
                    break;
                }
                case VALUE_LIST: {
                    // The variable is a list. The index must be an integer.
                    if (itype.type != VALUE_INT) {
                        ADD_LOG(access->index, "List access index must be an integer. Got '" + itype.to_string() + "'");
                        exit(logger->crash());
                    }
                    // All checks passed. We may set the index type and break the switch.
                    access->type = ACCESS_LIST;
                    break;
                }
                case VALUE_DICT: {
                    // The variable is a list. The index must be an integer.
                    if (itype.type != VALUE_STRING) {
                        ADD_LOG(access->index, "Dictionary access index must be a string. Got '" + itype.to_string() + "'");
                        exit(logger->crash());
                    }
                    // All checks passed. We may set the index type and break the switch.
                    access->type = ACCESS_DICT;
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
        case RULE_PROPERTY: {
            std::shared_ptr<Property> prop = std::static_pointer_cast<Property>(rule);
            // Analyze the type of the prop object.
            Type t = Type(prop->object, &this->blocks);
            if (t.type != VALUE_OBJECT) {
                ADD_LOG(prop->object, "Invalid property access. Tying to get a property of a non-object.");
                exit(logger->crash());
            }
            // Check if the class exists.
            this->check_classes({{ t.class_name }}, NODE(prop->object));
            // Get the class block.
            const std::shared_ptr<Block> &block = this->main_block->get_class(t.class_name)->block;
            // Check if the prop exists in the object class.
            if (!(block->get_variable(prop->name))) {
                ADD_LOG(prop->object, "The class '" + t.class_name + "' does not have a '" + prop->name + "' property.");
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
    for (std::shared_ptr<Statement> &tld : *this->code) this->analyze_tld(tld);
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
    // printf("SET BLOCK TO: %p\n", this->blocks.back().get());
    if (initializers.size() > 0) {
        for (const std::shared_ptr<Declaration> &argument : initializers) {
            this->declare(argument, initializer_node);
        }
    }
    // block.debug();
    for (const std::shared_ptr<Statement> &rule : code) {
        this->analyze_code(rule);
    }
    // printf("BLOCK DONE!\n");
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
            std::shared_ptr<FunctionValue> fun = std::static_pointer_cast<Function>(rule)->value;
            // Analyze the function parameters.
            for (const std::shared_ptr<Declaration> &parameter : fun->parameters) this->analyze_code(std::static_pointer_cast<Statement>(parameter), true);
            // Check if there's a top level return on the function.
            if (fun->return_type) {
                this->return_type = fun->return_type;
                for (const std::shared_ptr<Statement> &rule : fun->body) {
                    if (rule->rule == RULE_RETURN) goto continue_rule_function;
                }
                ADD_LOG(fun, "Expected at least 1 function top level return. Returns found inside conditionals or loops are not guaranted to happen.");
                exit(logger->crash());
            } else {
                for (const std::shared_ptr<Statement> &rule : fun->body) {
                    if (rule->rule == RULE_RETURN) goto continue_rule_function;
                }
                // Add an ending return because the function didn't have any!
                if (fun->body.size() == 0) {
                    fun->body.push_back(std::make_shared<Return>(fun->file, fun->line, fun->column));
                } else {
                    fun->body.push_back(std::make_shared<Return>(fun->body.back()->file, fun->body.back()->line, fun->body.back()->column));
                }
            }
            continue_rule_function:
            // Analyze the function body.
            fun->block = this->analyze_code(fun->body, fun->parameters);
            this->return_type.reset();
            break;
        }

        /* Normal statements */
        case RULE_PRINT: {
            this->analyze_code(std::static_pointer_cast<Print>(rule)->expression);
            break;
        }
        case RULE_EXPRESSION_STATEMENT: {
            this->analyze_code(std::static_pointer_cast<ExpressionStatement>(rule)->expression, true);
            break;
        }
        case RULE_DECLARATION: {
            std::shared_ptr<Declaration> dec = std::static_pointer_cast<Declaration>(rule);
            if (!dec->type) {
                // dec->initializer MUST be defined because of how parsing works.
                this->analyze_code(dec->initializer);
                // Get the type of the initializer,
                dec->type = std::make_shared<Type>(dec->initializer, &this->blocks);
                // Check if the type is correct.
                check_classes(dec->type->classes_used(MOD(dec->file)), NODE(dec));
                if (!no_declare) this->declare(dec);
                break;
            }
            check_classes(dec->type->classes_used(MOD(dec->file)), NODE(dec));
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
            if (!ret->value && !this->return_type) break;
            else if (ret->value && !this->return_type) {
                ADD_LOG(ret, "Return with value. There is no return type expected.");
                exit(logger->crash());
            } else if (!ret->value && this->return_type) {
                ADD_LOG(ret, "Return without value. There is a return type expected.");
                exit(logger->crash());
            }
            this->analyze_code(ret->value);
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
            if (vtype->type != VALUE_LIST && vtype->type != VALUE_DICT && vtype->type != VALUE_STRING) {
                ADD_LOG(rfor->iterator, "The 'for' iterator is not iterable. It must be either 'list', 'dictionary' or 'string' but got '" + vtype->to_string() + "'");
                exit(logger->crash());
            }
            rfor->type = vtype;
            decs.push_back(std::make_shared<Declaration>(
                rfor->file, rfor->line, rfor->column, rfor->variable,
                vtype->type == VALUE_STRING ? vtype : vtype->inner_type, std::shared_ptr<Expression>())
            );
            // Check if it uses the index and declare it if so.
            if (rfor->index != "") {
                std::shared_ptr<Type> itype = std::make_shared<Type>(vtype->type == VALUE_DICT ? VALUE_STRING : VALUE_INT); // Types (they are saved on the heap since they will be saved)
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
    if (block->has(dec->name)) {
        ADD_LOG(dec, "The variable '" + dec->name + "' is already declared in this scope.");
        exit(logger->crash());
    }
    // printf("Declared %s on block %p\n", dec->name.c_str(), block.get());
    // Declare the variable to the current scope.
    block->set_variable(dec->name, { dec->type, node ? node : NODE(dec) });
}

bool Module::check_classes(const std::vector<std::string> &classes, const std::shared_ptr<Node> &fail_at)
{
    for (const std::string &c : classes) {
        // Check if that class is defined here.
        if (!this->main_block->has_class(c)) {
            // The class is not defined in this module.
            ADD_LOG(fail_at, "The class '" + c.substr(c.rfind(':') + 1) + "' is an undeclared class type. Make sure to import all the necessary classes using 'use'.");
            exit(logger->crash());
        }
    }
}

#undef NODE
#undef ADD_LOG
#undef ADD_NULL_LOG
#undef MOD
