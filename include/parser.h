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

#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <variant>
#include <map>
#include <lexer.h>
#include <vector>

enum CLASS_NODE_TYPE {
    PUBLIC,
    PROTECTED,
    PRIVATE
};

enum DATATYPE {
    INT,
    FLOAT,
    DOUBLE,
    NUMBER,
    STRING,
    LIST,
    VARIANT
};

struct AST {
    virtual ~AST() = default;
};

struct ValueNode : AST {
    std::string value;
    ValueNode(std::string v) : value(std::move(v)) {}
};

struct BinOpNode : AST {
    AST* left;
    TOKEN_TYPE op;
    AST* right;

    BinOpNode(AST* l, TOKEN_TYPE o, AST* r) : left(std::move(l)), op(std::move(o)),
                                              right(std::move(r)) { }
};

struct VariableNode : AST {
    std::string name;
    DATATYPE datatype;

    VariableNode(std::string n, DATATYPE d) : name(std::move(n)), datatype(std::move(d)) { }
};

struct CallNode : VariableNode {
    std::vector<VariableNode*> param;
    CallNode(std::string n, std::vector<VariableNode*> p) :
                                                           VariableNode(std::move(n), VARIANT),
                                                           param(std::move(p)) { }
};

struct FunctionNode : CallNode {
    std::vector<AST*> body;
    FunctionNode(std::string n, std::vector<VariableNode*> p, std::vector<AST*> b) :
                                                        CallNode(std::move(n), std::move(p)),
                                                        body(std::move(b)) { }
};

struct AssignNode : AST {
    std::string name;
    AST* node;

    AssignNode(std::string n, AST* v) : name(std::move(n)), node(std::move(v)) { }
};

struct ConditionNode : BinOpNode {
    ConditionNode(AST* l, TOKEN_TYPE o, AST* r) : BinOpNode(std::move(l), o, std::move(r)) { }
};

struct IfWhileNode : AST {
    AST* condition;
    TOKEN_TYPE sign;
    std::string prefix;

    IfWhileNode(AST* c, TOKEN_TYPE s) : condition(std::move(c)), sign(std::move(s)) { }
    IfWhileNode(AST* c, TOKEN_TYPE s, std::string p) :
                    condition(std::move(c)), sign(std::move(s)), prefix(std::move(p)) { }
};

struct DefineNode : AST {
    std::string name;
    std::string value;

    DefineNode(std::string n, std::string v) : name(std::move(n)), value(std::move(v)) { }
};

struct FeatureNode : AST {
    std::string featureName;
    bool enabled;

    FeatureNode(std::string name, bool enable) : featureName(std::move(name)), enabled(enable) { }
};

struct InlineCodeNode : AST {
    std::string code;
    TOKEN_TYPE language;

    InlineCodeNode(std::string c, TOKEN_TYPE lang) : code(std::move(c)), language(std::move(lang)) { }
};

struct EndNode : AST {
    TOKEN_TYPE endType;

    EndNode(TOKEN_TYPE type) : endType(std::move(type)) { }
};

struct StructNode : AST {
    std::vector<ValueNode> structure;
    StructNode(std::vector<ValueNode> Struct) : structure(std::move(Struct)) { }
};

struct ClassNode : AST {
    std::map<ValueNode, CLASS_NODE_TYPE> classStructure;
    ClassNode(std::map<ValueNode, CLASS_NODE_TYPE> ClassStruct) :
                             classStructure(std::move(ClassStruct)) { }
};

struct ReturnNode : AST {
    VariableNode* node;
    ReturnNode(VariableNode* n) : node(std::move(n)) { }
};

class Parser {
private:
    std::vector<TOKEN> tokens;
    int idx = 0;

    AST* term();
    AST* expr();
    AST* factor();
    std::vector<AST*> statement();
    inline bool isAtEnd() {
        return idx > tokens.size();
    }
public:
    Parser(std::vector<TOKEN> t) : tokens(std::move(t)) { }
    std::vector<AST*> parse();
    ~Parser();
};

#endif // PARSER_H
