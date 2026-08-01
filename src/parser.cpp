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

#include <cstring>
#include <iostream>
#include <vector>
#include <variant>
#include <definations.h>
#include <lexer.h>
#include <parser.h>

#define T(i)            (tokens[i])
#define TT(i)           (tokens[i].type)
#define MATCH(i, t)     (tokens[i].type == t)

std::vector<AST*> Parser::parse() {
    std::vector<AST*> nodes;
    while (!isAtEnd()) {
        auto stmts = statement();
        nodes.insert(nodes.end(), stmts.begin(), stmts.end());
    }
    tokens.clear();
    lexs.clear();
    return nodes;
}

std::vector<AST*> Parser::statement() {
    std::vector<AST*> nodes;
    TOKEN_TYPE current = TT(idx);

    if (current == TOKEN_DEFINE) {
        int startRow = T(idx).row, startCol = T(idx).col;
        idx++;
        std::string defineName = T(idx).value;
        idx++;
        std::string defineValue = "";
        // Collect value until end of line or next macro
        while (!isAtEnd() && TT(idx) != TOKEN_IF && TT(idx) != TOKEN_ELSE &&
               TT(idx) != TOKEN_FEATURE && TT(idx) != TOKEN_NO_FEATURE &&
               TT(idx) != TOKEN_DEFINE && TT(idx) != TOKEN_C && TT(idx) != TOKEN_ASM &&
               TT(idx) != TOKEN_END && T(idx).row == T(idx - 1).row) {
            defineValue += T(idx).value + " ";
            idx++;
        }
        // Remove trailing space if present
        if (!defineValue.empty() && defineValue.back() == ' ') {
            defineValue.pop_back();
        }
        nodes.push_back(new DefineNode(defineName, defineValue, startRow, startCol));
    } else if (current == TOKEN_FEATURE) {
        int featRow = T(idx).row, featCol = T(idx).col;
        idx++;
        std::string featureName = T(idx).value;
        idx++;
        nodes.push_back(new FeatureNode(featureName, true, featRow, featCol));
    } else if (current == TOKEN_NO_FEATURE) {
        int noFeatRow = T(idx).row, noFeatCol = T(idx).col;
        idx++;
        std::string featureName = T(idx).value;
        idx++;
        nodes.push_back(new FeatureNode(featureName, false, noFeatRow, noFeatCol));
    } else if (current == TOKEN_C || current == TOKEN_ASM) {
        int codeRow = T(idx).row, codeCol = T(idx).col;
        TOKEN_TYPE lang = current;
        idx++;
        std::string code = "";
        int startCol = T(idx).col;
        
        // Check if it's a multi-line block (next token has greater column)
        if (T(idx).col > startCol) {
            // Multi-line block - collect until #end at same or lesser column
            while (!isAtEnd() && !(TT(idx) == TOKEN_END && T(idx).col <= startCol)) {
                code += T(idx).value + " ";
                idx++;
            }
            // Skip the #end token
            if (TT(idx) == TOKEN_END) {
                nodes.push_back(new EndNode(lang, codeRow, codeCol));
                idx++;
            }
        } else {
            // Single-line inline code
            while (!isAtEnd() && TT(idx) != TOKEN_IF && TT(idx) != TOKEN_ELSE && 
                   TT(idx) != TOKEN_FEATURE && TT(idx) != TOKEN_NO_FEATURE &&
                   TT(idx) != TOKEN_C && TT(idx) != TOKEN_ASM &&
                   TT(idx) != TOKEN_END && T(idx).row == T(idx - 1).row) {
                code += T(idx).value + " ";
                idx++;
            }
        }
        nodes.push_back(new InlineCodeNode(code, lang, codeRow, codeCol));
    } else if (current == TOKEN_END) {
        int endRow = T(idx).row, endCol = T(idx).col;
        idx++;
        nodes.push_back(new EndNode(TOKEN_END, endRow, endCol));
    } else if (current == TOKEN_IF) {
        int prevIDX = idx;
        int ifRow = T(idx).row, ifCol = T(idx).col;
        idx++;
        AST* condition = expr();
        if (MATCH(idx, TOKEN_COLON)) {
            idx++;
            std::vector<AST*> ifBody;
            while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).col > ifCol) {
                if (TT(idx) == TOKEN_IF || TT(idx) == TOKEN_ELSE || TT(idx) == TOKEN_ELIF) {
                    for (AST* s : statement())
                        ifBody.push_back(s);
                } else {
                    ifBody.push_back(expr());
                }
            }

            if (T(prevIDX).value[0] == '#')
                nodes.push_back(new IfWhileNode(condition, ifBody, TOKEN_IF, '#', ifRow, ifCol));
            else
                nodes.push_back(new IfWhileNode(condition, ifBody, TOKEN_IF, ifRow, ifCol));
        } else {
            error("Expected ':' after if condition", ifRow, ifCol);
        }
    } else if (current == TOKEN_ELSE) {
        int prevIDX = idx;
        int elseRow = T(idx).row, elseCol = T(idx).col;
        if (MATCH(idx + 1, TOKEN_IF)) {
            idx++;
            idx++;
            AST* condition = expr();
            if (MATCH(idx, TOKEN_COLON)) {
                idx++;
                std::vector<AST*> elseIfBody;
                while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).col > elseCol) {
                    if (TT(idx) == TOKEN_IF || TT(idx) == TOKEN_ELSE || TT(idx) == TOKEN_ELIF) {
                        for (AST* s : statement())
                            elseIfBody.push_back(s);
                    } else {
                        elseIfBody.push_back(expr());
                    }
                }
                if (T(prevIDX).value[0] == '#')
                    nodes.push_back(new IfWhileNode(condition, elseIfBody, TOKEN_ELIF, '#', elseRow, elseCol));
                else
                    nodes.push_back(new IfWhileNode(condition, elseIfBody, TOKEN_ELIF, elseRow, elseCol));
            } else {
                error("Expected ':' after else if condition", elseRow, elseCol);
            }
        } else {
            idx++;
            if (MATCH(idx, TOKEN_COLON)) {
                idx++;
                std::vector<AST*> elseBody;
                while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).col > elseCol) {
                    if (TT(idx) == TOKEN_IF || TT(idx) == TOKEN_ELSE || TT(idx) == TOKEN_ELIF) {
                        for (AST* s : statement())
                            elseBody.push_back(s);
                    } else {
                        elseBody.push_back(expr());
                    }
                }
                if (T(prevIDX).value[0] == '#')
                    nodes.push_back(new IfWhileNode(nullptr, elseBody, TOKEN_ELSE, '#', elseRow, elseCol));
                else
                    nodes.push_back(new IfWhileNode(nullptr, elseBody, TOKEN_ELSE, elseRow, elseCol));
            } else {
                error("Expected ':' after else", elseRow, elseCol);
            }
        }
    } else if (current == TOKEN_ELIF) {
        int prevIDX = idx;
        int elifRow = T(idx).row, elifCol = T(idx).col;
        idx++;
        AST* condition = expr();
        if (MATCH(idx, TOKEN_COLON)) {
            idx++;
            std::vector<AST*> elifBody;
            while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).col > elifCol) {
                if (TT(idx) == TOKEN_IF || TT(idx) == TOKEN_ELSE || TT(idx) == TOKEN_ELIF) {
                    for (AST* s : statement())
                        elifBody.push_back(s);
                } else {
                    elifBody.push_back(expr());
                }
            }
            if (T(prevIDX).value[0] == '#')
                nodes.push_back(new IfWhileNode(condition, elifBody, TOKEN_ELIF, '#', elifRow, elifCol));
            else
                nodes.push_back(new IfWhileNode(condition, elifBody, TOKEN_ELIF, elifRow, elifCol));
        } else {
            error("Expected ':' after elif condition", elifRow, elifCol);
        }
    } else if (current == TOKEN_WHILE) {
        int whileRow = T(idx).row, whileCol = T(idx).col;
        idx++;
        AST* condition = expr();
        if (MATCH(idx, TOKEN_COLON)) {
            idx++;
            std::vector<AST*> whileBody;
            while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).col > whileCol) {
                if (TT(idx) == TOKEN_IF || TT(idx) == TOKEN_ELSE || TT(idx) == TOKEN_ELIF) {
                    for (AST* s : statement())
                        whileBody.push_back(s);
                } else {
                    whileBody.push_back(expr());
                }
            }
            nodes.push_back(new IfWhileNode(condition, whileBody, TOKEN_WHILE, whileRow, whileCol));
        } else {
            error("Expected ':' after while condition", whileRow, whileCol);
        }
    } else if (current == TOKEN_RETURN) {
        int retRow = T(idx).row, retCol = T(idx).col;
        idx++;
        AST* returnValue = expr();
        VariableNode* returnVar = dynamic_cast<VariableNode*>(returnValue);
        if (!returnVar) {
            // If expr() didn't return a VariableNode, create one from the value
            if (auto* valNode = dynamic_cast<ValueNode*>(returnValue)) {
                returnVar = new VariableNode(valNode->value, VARIANT, false, retRow, retCol);
            } else {
                returnVar = new VariableNode("", VARIANT, false, retRow, retCol);
            }
        }
        nodes.push_back(new ReturnNode(returnVar, retRow, retCol));
        return nodes;
    } else if (current == TOKEN_VAR) {
        idx++;
        while (!isAtEnd() && TT(idx) == TOKEN_IDENTIFIER) {
            nodes.push_back(new VariableNode(T(idx).value, VARIANT, true, T(idx).row, T(idx).col));
            idx++;
            if (MATCH(idx, TOKEN_COMMA)) {
                idx++;
                continue;
            }
            break;
        }
    } else if (current == TOKEN_IDENTIFIER) {
        std::vector<std::string> varNames;
        int saveIdx = idx;

        while (true) {
            if (TT(idx) == TOKEN_IDENTIFIER) {
                varNames.push_back(T(idx).value);
                idx++;
                if (MATCH(idx, TOKEN_COMMA)) {
                    idx++;
                    continue;
                } else if (MATCH(idx, TOKEN_ASSIGNMENT)) {
                    idx++;
                    break;
                } else {
                    idx = saveIdx;
                    TOKEN_TYPE next = TT(idx + 1);
                    if (next == TOKEN_STRING || next == TOKEN_NUMBER ||
                        next == TOKEN_IDENTIFIER || next == TOKEN_LPAREN ||
                        next == TOKEN_EQUAL || next == TOKEN_GREATER ||
                        next == TOKEN_SHORTER || next == TOKEN_GREATER_EQUAL ||
                        next == TOKEN_SHORTER_EQUAL || next == TOKEN_AND ||
                        next == TOKEN_OR || next == TOKEN_XOR) {
                        nodes.push_back(expr());
                    } else {
                        for (const auto& varName : varNames) {
                            nodes.push_back(new VariableNode(varName, VARIANT, true, T(saveIdx).row, T(saveIdx).col));
                        }
                    }
                    break;
                }
            } else {
                idx = saveIdx;
                nodes.push_back(expr());
                break;
            }
        }

        if (!varNames.empty() && idx > 0 && TT(idx - 1) == TOKEN_ASSIGNMENT) {
            for (const auto& varName : varNames) {
                nodes.push_back(new VariableNode(varName, VARIANT, true, T(saveIdx).row, T(saveIdx).col));
            }
            AST* value = expr();
            for (const auto& varName : varNames) {
                nodes.push_back(new AssignNode(varName, value, T(saveIdx).row, T(saveIdx).col));
            }
        }
    } else {
        if (!isAtEnd())
            nodes.push_back(expr());
    }

    return nodes;
}

AST* Parser::expr() {
    AST* node = term();
    while (!isAtEnd() &&
           (MATCH(idx, TOKEN_PLUS) || MATCH(idx, TOKEN_MINUS) ||
           MATCH(idx, TOKEN_EQUAL) || MATCH(idx, TOKEN_GREATER) ||
           MATCH(idx, TOKEN_SHORTER) || MATCH(idx, TOKEN_GREATER_EQUAL) ||
           MATCH(idx, TOKEN_SHORTER_EQUAL) || MATCH(idx, TOKEN_AND) ||
           MATCH(idx, TOKEN_OR) || MATCH(idx, TOKEN_XOR) ||
           MATCH(idx, TOKEN_ASSIGNMENT))) {
        TOKEN_TYPE op = TT(idx);
        int opRow = T(idx).row, opCol = T(idx).col;
        idx++;
        if (op == TOKEN_ASSIGNMENT) {
            if (auto* var = dynamic_cast<VariableNode*>(node)) {
                node = new AssignNode(var->name, expr(), opRow, opCol);
                break;
            }
            continue;
        }
        node = new BinOpNode(node, op, term(), opRow, opCol);
    }
    return node;
}

AST* Parser::term() {
    AST* node = factor();
    while (!isAtEnd() && (MATCH(idx, TOKEN_MULTIPLY) || MATCH(idx, TOKEN_DIVIDE))) {
        TOKEN_TYPE op = TT(idx);
        int opRow = T(idx).row, opCol = T(idx).col;
        idx++;
        node = new BinOpNode(node, op, factor(), opRow, opCol);
    }
    return node;
}

AST* Parser::factor() {
    TOKEN_TYPE op = TT(idx);
    if (op == TOKEN_STRING || op == TOKEN_NUMBER) {
        idx++;
        return new ValueNode(T(idx - 1).value, T(idx - 1).row, T(idx - 1).col);
    }
    else if (op == TOKEN_IDENTIFIER) {
        // Check if this is a function definition (followed by colon)
        int saveIdx = idx;
        idx++;
        if (idx < tokens.size() && MATCH(idx, TOKEN_LPAREN)) {
            // Function call with arguments: func(arg1, arg2)
            idx = saveIdx;
            std::string funcName = T(idx).value;
            idx++; // skip function name
            idx++; // skip opening paren
            
            std::vector<AST*> args;
            while (!MATCH(idx, TOKEN_RPAREN) && !isAtEnd()) {
                args.push_back(expr());
                if (MATCH(idx, TOKEN_COMMA)) {
                    idx++;
                }
            }
            idx++; // skip closing paren

            return new CallNode(funcName, args, T(saveIdx).row, T(saveIdx).col);
        }
        else {
            // Check if this is a function call without parentheses: func arg1, arg2
            // Look ahead to see if next token is a valid argument (identifier, string, number)
            int nextIdx = idx;
            if (nextIdx < tokens.size() && (TT(nextIdx) == TOKEN_IDENTIFIER || TT(nextIdx) == TOKEN_STRING || TT(nextIdx) == TOKEN_NUMBER)) {
                idx = saveIdx;
                std::string funcName = T(idx).value;
                idx++; // skip function name
                
                std::vector<AST*> args;
                // Collect arguments until end of line, semicolon, dot, or colon mismatch
                while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && TT(idx) != TOKEN_DOT && T(idx).row == T(saveIdx).row) {
                    // Check for colon mismatch
                    if (TT(idx) == TOKEN_COLON && (idx + 1 >= tokens.size() || TT(idx + 1) != TOKEN_COLON)) {
                        break;
                    }
                    if (TT(idx) == TOKEN_COMMA) {
                        idx++;
                        continue;
                    }
                    args.push_back(expr());
                }
                
                return new CallNode(funcName, args, T(saveIdx).row, T(saveIdx).col);
            }
        }
        if (idx < tokens.size() && MATCH(idx, TOKEN_COLON)) {
            // Function definition: funcname(args):
            idx = saveIdx;
            std::string funcName = T(idx).value;
            idx++; // skip function name
            idx++; // skip colon
            
            // Parse function body (until end of line or same column)
            std::vector<AST*> body;
            int startCol = T(idx).col;
            while (!isAtEnd() && T(idx).col > startCol) {
                body.push_back(expr());
            }

            // Empty parameter list for function definition
            std::vector<VariableNode*> argNodes;
            return new FunctionNode(funcName, argNodes, body, T(saveIdx).row, T(saveIdx).col);
        } else {
            idx = saveIdx;
            idx++;
            return new VariableNode(T(idx - 1).value, VARIANT, false, T(idx - 1).row, T(idx - 1).col);
        }
    } else if (op == TOKEN_ASSIGNMENT) {
        int assignRow = T(idx - 1).row, assignCol = T(idx - 1).col;
        const std::string name = T(idx - 1).value;
        idx++;
        return new AssignNode(name, factor(), assignRow, assignCol);
    }
    else if (op == TOKEN_LPAREN) {
        char openBracket = '(';
        if (!T(idx).value.empty())
            openBracket = T(idx).value[0];
        char closeBracket;
        
        switch (openBracket) {
            case '(': closeBracket = ')'; break;
            case '[': closeBracket = ']'; break;
            case '{': closeBracket = '}'; break;
            default: break;
        }
        
        idx++; // skip opening bracket
        AST* innerExpr = expr();

        if (MATCH(idx, TOKEN_RPAREN) == false && (closeBracket == ')' || closeBracket == ']' || closeBracket == '}')) {
            error("Expected closing bracket '" + std::string(1, closeBracket) + "'", T(idx).row, T(idx).col);
            return nullptr;
        }
        
        idx++; // skip closing bracket
        return innerExpr;
    }
    
    idx++;
    return nullptr;
}

Parser::~Parser() {
    idx = 0;
    tokens.clear();
}
