/*
 * baregear - A programming language compiler
 * Copyright (C) 2026 First Person
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <vector>
#include <sstream>
#include <set>
#include <parser.h>
#include <transpiler.h>
#include <definations.h>

std::map<std::string, DATATYPE> variableIndex;

std::string Transpiler::transpile() {
    // Output CPP header
    hstr << "#include <iostream>" << std::endl;
    sstr << std::endl;

    std::stringstream mainStmts;
    for (AST* node : nodes) {
        if (dynamic_cast<FunctionNode*>(node) ||
            dynamic_cast<DefineNode*>(node) ||
            dynamic_cast<InlineCodeNode*>(node))
            sstr << factor(node) << std::endl;
        else
            mainStmts << "    " << statement(node) << std::endl;
    }
    hstr << sstr.str();
    hstr << str.str();
    hstr << "int main() {" << std::endl;
    hstr << mainStmts.str();
    hstr << "    return 0;" << std::endl;
    hstr << "}" << std::endl;
    nodes.clear();
    return hstr.str();
}

std::string Transpiler::statement(AST* node) {
    std::string s = factor(node);
    if (dynamic_cast<CallNode*>(node) ||
        dynamic_cast<BinOpNode*>(node) ||
        dynamic_cast<ValueNode*>(node)) {
        return s + ';';
    }
    if (auto var = dynamic_cast<VariableNode*>(node)) {
        if (!var->isDecl)
            return s + ';';
    }
    return s;
}

std::string Transpiler::factor(AST* body) {
    if (auto fn = dynamic_cast<FunctionNode*>(body)) {
        std::stringstream str;
        str << "void " << fn->name << "(";
        for (size_t i = 0; i < fn->param.size(); i++) {
            if (auto varParam = dynamic_cast<VariableNode*>(fn->param[i])) {
                if (i > 0) str << ", ";
                str << getCDataType(varParam->datatype) << ' ' << varParam->name;
                variableIndex.insert({varParam->name, varParam->datatype});
            }
        }

        str << ") {" << std::endl;
        for (AST* body : fn->body)
            str << statement(body) << std::endl;

        str << "}" << std::endl;
        return str.str();
    }
    else if (auto call = dynamic_cast<CallNode*>(body)) {
        std::stringstream str;
        str << call->name << "(";
        for (size_t i = 0; i < call->param.size(); i++) {
            if (i > 0) str << ", ";
            str << factor(call->param[i]);
        }
        str << ")";
        return str.str();
    }
    else if (auto val = dynamic_cast<ValueNode*>(body)) {
        // Check if the value is a string (contains non-numeric characters)
        try {
            std::stol(val->value);
            return val->value; // It's a number
        } catch (std::invalid_argument&) {
            // It's a string, add quotes
            return "\"" + val->value + "\"";
        } catch (std::out_of_range&) {
            // It's a string, add quotes
            return "\"" + val->value + "\"";
        }
    }
    else if (auto var = dynamic_cast<VariableNode*>(body)) {
        if (var->isDecl) {
            if (!variableIndex.contains(var->name)) {
                variableIndex.insert({var->name, var->datatype});
                return getCDataType(var->datatype) + ' ' + var->name + ';';
            }
            return "";
        }
        if (variableIndex.contains(var->name))
            return variableIndex.find(var->name)->second == VARIANT
                       ? "std::get<>(" + var->name + ')'
                       : var->name;
        return var->name;
    }
    else if (auto asn = dynamic_cast<AssignNode*>(body)) {
        if (!variableIndex.contains(asn->name))
            variableIndex.insert({asn->name, VARIANT});
        if (auto val = dynamic_cast<ValueNode*>(asn->node)) {
            try {
                std::stol(val->value);
                return asn->name + " = " + val->value + ';';
            } catch (const std::exception&) {
                return asn->name + " = \"" + val->value + "\";";
            }
        }
        else if (auto bon = dynamic_cast<BinOpNode*>(asn->node))
            return asn->name + " = " + factor(bon) + ';';
        else if (auto call = dynamic_cast<CallNode*>(asn->node))
            return asn->name + " = " + factor(call);
        else if (auto var = dynamic_cast<VariableNode*>(asn->node))
            return asn->name + " = " + var->name + ';';
        else
            error("Illegal Value Used On " + asn->name, 0, 0);
    }
    else if (auto bon = dynamic_cast<BinOpNode*>(body)) {
        std::string op;
        switch (bon->op) {
            case TOKEN_PLUS:
                op = "+";
                break;
            case TOKEN_MINUS:
                op = "-";
                break;
            case TOKEN_MULTIPLY:
                op = "*";
                break;
            case TOKEN_DIVIDE:
                op = "/";
                break;
            case TOKEN_EQUAL:
                op = "==";
                break;
            case TOKEN_GREATER:
                op = ">";
                break;
            case TOKEN_SHORTER:
                op = "<";
                break;
            case TOKEN_GREATER_EQUAL:
                op = ">=";
                break;
            case TOKEN_SHORTER_EQUAL:
                op = "<=";
                break;
            case TOKEN_AND:
                op = "&&";
                break;
            case TOKEN_OR:
                op = "||";
                break;
            case TOKEN_XOR:
                op = "^";
                break;
            case TOKEN_NOT:
                op = "!";
                break;
            default:
                op = "+";
                break;
        }
        return "(" + factor(bon->left) + " " + op + " " + factor(bon->right) + ")";
    }
    else if (auto defineNode = dynamic_cast<DefineNode*>(body)) {
        // Define nodes for constants/macros - output as #define
        return "#define " + defineNode->name + " " + defineNode->value;
    }
    else if (auto inlineCodeNode = dynamic_cast<InlineCodeNode*>(body)) {
        // Inline code nodes - output directly based on language type
        std::string code = inlineCodeNode->code;
        // Remove trailing space if present
        if (!code.empty() && code.back() == ' ')
            code.pop_back();
        
        switch (inlineCodeNode->language) {
            case TOKEN_C: {
                // Substitute references to baregear variables inside the inline
                // C++ code with their C++ access form (mirrors the Clang AST
                // "uses minus declarations" analysis):
                //   - variant-typed variables are accessed via std::get<>(name)
                //   - concrete-typed variables keep their plain name
                // Names that the inline code itself declares (preceded by a C++
                // type keyword) are subtracted and left untouched.
                static const std::set<std::string> cppTypes = {
                    "int", "float", "double", "short", "long", "string",
                    "auto", "char", "bool", "unsigned", "const", "std::string"
                };
                std::vector<std::string> toks;
                std::istringstream iss(code);
                std::string tok;
                while (iss >> tok)
                    toks.push_back(tok);
                std::stringstream out;
                for (size_t i = 0; i < toks.size(); i++) {
                    if (i > 0) out << ' ';
                    std::string word = toks[i];
                    auto it = variableIndex.find(word);
                    if (it != variableIndex.end()) {
                        bool isDecl = i > 0 && cppTypes.contains(toks[i - 1]);
                        if (!isDecl && it->second == VARIANT)
                            out << "std::get<>(" << word << ')';
                        else
                            out << word;
                    } else {
                        out << word;
                    }
                }
                return out.str();
            }
            case TOKEN_ASM: {
                // Escape double quotes and backslashes so assembly survives the
                // C++ string literal untouched
                std::string escaped;
                for (char ch : code) {
                    if (ch == '"') {
                        escaped += "\\\"";
                    } else if (ch == '\\') {
                        escaped += "\\\\";
                    } else {
                        escaped += ch;
                    }
                }
                // LLVM/Clang-compatible basic asm: `volatile` prevents the optimizer
                // from deleting or reordering the statement
                return "__asm__ volatile(\"" + escaped + "\")";
            }
            default:
                return code;
        }
    }
    else if (auto endNode = dynamic_cast<EndNode*>(body)) {
        // End nodes mark block endings - could be used for cleanup or ignored
        return "";  // No output needed for end markers
    }
    else if (auto ifWhileNode = dynamic_cast<IfWhileNode*>(body)) {
        std::stringstream str;
        if (ifWhileNode->sign == TOKEN_IF) {
            str << "if (" << factor(ifWhileNode->condition) << ") {" << std::endl;
            for(AST* stmt : ifWhileNode->body)
                str << "    " << statement(stmt) << std::endl;
            str << "}" << std::endl;
        } else if (ifWhileNode->sign == TOKEN_ELIF) {
            str << " else if (" << factor(ifWhileNode->condition) << ") {" << std::endl;
            for(AST* stmt : ifWhileNode->body)
                str << "    " << statement(stmt) << std::endl;
            str << "}" << std::endl;
        } else if (ifWhileNode->sign == TOKEN_ELSE) {
            str << " else {" << std::endl;
            for(AST* stmt : ifWhileNode->body)
                str << "    " << statement(stmt) << std::endl;
            str << "}" << std::endl;
        } else if (ifWhileNode->sign == TOKEN_WHILE) {
            str << "while (" << factor(ifWhileNode->condition) << ") {" << std::endl;
            for(AST* stmt : ifWhileNode->body)
                str << "    " << statement(stmt) << std::endl;
            str << "}" << std::endl;
            return str.str();
        }
        return str.str();
    }
    else if (auto returnNode = dynamic_cast<ReturnNode*>(body)) {
        if (returnNode->node)
            return "return " + factor(returnNode->node) + ";";
        return "return;";
    }
    else if (auto switchNode = dynamic_cast<SwitchNode*>(body)) {
        std::stringstream str;
        str << "switch (" << factor(switchNode->condition) << ") {" << std::endl;
        for (AST* c : switchNode->cases) {
            if (auto caseNode = dynamic_cast<CaseNode*>(c)) {
                if (caseNode->isDefault)
                    str << "default:" << std::endl;
                else
                    str << "case " << factor(caseNode->value) << ":" << std::endl;
                for (AST* stmt : caseNode->body)
                    str << "    " << statement(stmt) << std::endl;
                str << "    break;" << std::endl;
            }
        }
        str << "}" << std::endl;
        return str.str();
    }
    return "";
}

inline std::string Transpiler::getCDataType(DATATYPE dtype) {
    switch (dtype) {
        case STRING:
            return "std::string";

        case INT:
            return "int";

        case SHORT:
            return "short";

        case NUMBER:
            if (!isVarDTYPEUsed) {
                hstr << "#include <variant>";
                isVarDTYPEUsed = true;
            }

            return "std::variant<int, float, double, short, long>";

        case FLOAT:
            return "float";

        case DOUBLE:
            return "double";

        case VARIANT:
            if (!isVarDTYPEUsed) {
                hstr << "#include <variant>" << std::endl;
                isVarDTYPEUsed = true;
            }

            return "std::variant<std::string, int, float, double, short, long>";

        default:
            return "std::variant<std::string, int, float, double, short, long>";
    }
}
