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
#include <parser.h>
#include <transpiler.h>
#include <definations.h>

std::map<std::string, DATATYPE> variableIndex;

std::string Transpiler::transpile() {
    std::stringstream str;
    
    // Output C header
    str << "#include <stdio.h>" << std::endl;
    str << "#include <stdlib.h>" << std::endl;
    str << "#include <string.h>" << std::endl;
    str << std::endl;
    
    // Process all AST nodes
    for (AST* node : nodes)
        str << factor(node) << std::endl;
    
    return str.str();
}

std::string Transpiler::factor(AST* body) {
    if (auto fn = dynamic_cast<FunctionNode*>(body)) {
        std::stringstream str;
        str << "void " << fn->name << "(";
        for (VariableNode* param : fn->param)
            str << getCDataType(param->datatype) << param->name;

        str << ") {" << std::endl;
        for (AST* body : fn->body)
            str << factor(body) << std::endl;

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
        str << ");";
        return str.str();
    }
    else if (auto var = dynamic_cast<VariableNode*>(body)) {
	    variableIndex.insert({var->name, var->datatype});
        return getCDataType(var->datatype) + ' ' + var->name + ';';
    }
    else if (auto asn = dynamic_cast<AssignNode*>(body)) {
        if (!variableIndex.contains(asn->name))
            bugDetected("Variable Is Using Without Declaration");
        else if (auto val = dynamic_cast<ValueNode*>(asn->node)) {
            try {
                std::stol(val->value);
                return asn->name + " = " + val->value + ';';
            } catch (std::runtime_error _) {
                return asn->name + " = \"" + val->value + "\";";
            }
        }
        else if (auto bon = dynamic_cast<BinOpNode*>(asn->node))
            return asn->name + " = " + factor(bon) + ';';
        else
            error("Illegal Value Used On " + asn->name, 0, 0);
    }
    else if (auto val = dynamic_cast<ValueNode*>(body))
	    return val->value;
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
    else if (auto featureNode = dynamic_cast<FeatureNode*>(body)) {
        // Feature nodes control compiler behavior - output as comments
        std::string state = featureNode->enabled ? "enable" : "disable";
        return "// Compiler feature: " + state + " " + featureNode->featureName;
    }
    else if (auto inlineCodeNode = dynamic_cast<InlineCodeNode*>(body)) {
        // Inline code nodes - output directly based on language type
        std::string code = inlineCodeNode->code;
        // Remove trailing space if present
        if (!code.empty() && code.back() == ' ') {
            code.pop_back();
        }
        
        switch (inlineCodeNode->language) {
            case TOKEN_C:
                return "extern \"C\" {\n" + code + "\n}\n";  // Direct C code output
            case TOKEN_CPP:
                return code;  // Direct C++ code output
            case TOKEN_ASM:
                return "__asm__(\"" + code + "\")";  // Wrap assembly in GCC inline asm
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
            str << "if (" << factor(ifWhileNode->condition) << ") {";
        } else if (ifWhileNode->sign == TOKEN_ELIF) {
            str << " else if (" << factor(ifWhileNode->condition) << ") {";
        } else if (ifWhileNode->sign == TOKEN_ELSE) {
            str << " else {";
        }
        return str.str();
    }
    else if (auto returnNode = dynamic_cast<ReturnNode*>(body)) {
        if (returnNode->node) {
            return "return " + factor(returnNode->node) + ";";
        }
        return "return;";
    }
    return "";
}

inline std::string Transpiler::getCDataType(DATATYPE dtype) {
    switch (dtype) {
        case STRING:
            return "char*";

        case NUMBER:
            return "long";

        case FLOAT:
            return "float";

        case DOUBLE:
            return "double";

        case VARIANT:
            return "var";

        default:
            return "long";
    }
}
