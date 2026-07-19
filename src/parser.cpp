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
    return nodes;
}

std::vector<AST*> Parser::statement() {
    std::vector<AST*> nodes;
    TOKEN_TYPE current = TT(idx);

    if (current == TOKEN_DEFINE) {
        idx++;
        std::string defineName = T(idx).value;
        idx++;
        std::string defineValue = "";
        // Collect value until end of line or next macro
        while (!isAtEnd() && TT(idx) != TOKEN_IF && TT(idx) != TOKEN_ELSE &&
               TT(idx) != TOKEN_FEATURE && TT(idx) != TOKEN_NO_FEATURE &&
               TT(idx) != TOKEN_DEFINE && TT(idx) != TOKEN_C &&
               TT(idx) != TOKEN_CPP && TT(idx) != TOKEN_ASM &&
               TT(idx) != TOKEN_END && T(idx).row == T(idx - 1).row) {
            defineValue += T(idx).value + " ";
            idx++;
        }
        // Remove trailing space if present
        if (!defineValue.empty() && defineValue.back() == ' ') {
            defineValue.pop_back();
        }
        nodes.push_back(new DefineNode(defineName, defineValue));
    } else if (current == TOKEN_FEATURE) {
        idx++;
        std::string featureName = T(idx).value;
        idx++;
        nodes.push_back(new FeatureNode(featureName, true));
    } else if (current == TOKEN_NO_FEATURE) {
        idx++;
        std::string featureName = T(idx).value;
        idx++;
        nodes.push_back(new FeatureNode(featureName, false));
    } else if (current == TOKEN_C || current == TOKEN_CPP || current == TOKEN_ASM) {
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
                nodes.push_back(new EndNode(lang));
                idx++;
            }
        } else {
            // Single-line inline code
            while (!isAtEnd() && TT(idx) != TOKEN_IF && TT(idx) != TOKEN_ELSE && 
                   TT(idx) != TOKEN_FEATURE && TT(idx) != TOKEN_NO_FEATURE &&
                   TT(idx) != TOKEN_C && TT(idx) != TOKEN_CPP && TT(idx) != TOKEN_ASM &&
                   TT(idx) != TOKEN_END && T(idx).row == T(idx - 1).row) {
                code += T(idx).value + " ";
                idx++;
            }
        }
        nodes.push_back(new InlineCodeNode(code, lang));
    } else if (current == TOKEN_END) {
        idx++;
        nodes.push_back(new EndNode(TOKEN_END));
    } else if (current == TOKEN_IF) {
        idx++;
        AST* condition = expr();
        if (MATCH(idx, TOKEN_COLON)) {
            idx++;
            std::vector<AST*> ifBody;
            int startCol = T(idx).col;
            while (!isAtEnd() && T(idx).col > startCol) {
                ifBody.push_back(expr());
            }
            nodes.push_back(new IfWhileNode(condition, TOKEN_IF));
        }
    } else if (current == TOKEN_ELSE) {
        if (MATCH(idx + 1, TOKEN_IF)) {
            idx++;
            AST* condition = expr();
            if (MATCH(idx, TOKEN_COLON)) {
                idx++;
                std::vector<AST*> elseIfBody;
                int startCol = T(idx).col;
                while (!isAtEnd() && T(idx).col > startCol) {
                    elseIfBody.push_back(expr());
                }
                nodes.push_back(new IfWhileNode(condition, TOKEN_ELIF));
            }
        } else {
            idx++;
            if (MATCH(idx, TOKEN_COLON)) {
                idx++;
                std::vector<AST*> elseBody;
                int startCol = T(idx).col;
                while (!isAtEnd() && T(idx).col > startCol) {
                    elseBody.push_back(expr());
                }
                nodes.push_back(new IfWhileNode(nullptr, TOKEN_ELSE));
            }
        }
    } else if (current == TOKEN_ELIF) {
        idx++;
        AST* condition = expr();
        if (MATCH(idx, TOKEN_COLON)) {
            idx++;
            std::vector<AST*> elifBody;
            int startCol = T(idx).col;
            while (!isAtEnd() && T(idx).col > startCol) {
                elifBody.push_back(expr());
            }
            nodes.push_back(new IfWhileNode(condition, TOKEN_ELIF));
        }
    } else if (current == TOKEN_RETURN) {
        idx++;
        AST* returnValue = expr();
        VariableNode* returnVar = dynamic_cast<VariableNode*>(returnValue);
        if (!returnVar) {
            // If expr() didn't return a VariableNode, create one from the value
            if (auto* valNode = dynamic_cast<ValueNode*>(returnValue)) {
                returnVar = new VariableNode(valNode->value, VARIANT);
            } else {
                returnVar = new VariableNode("", VARIANT);
            }
        }
        nodes.push_back(new ReturnNode(returnVar));
        return nodes;
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
                    nodes.push_back(expr());
                    break;
                }
            } else {
                idx = saveIdx;
                nodes.push_back(expr());
                break;
            }
        }

        if (varNames.size() > 1) {
            AST* value = expr();
            for (const auto& varName : varNames) {
                nodes.push_back(new AssignNode(varName, value));
            }
        }
    } else {
        nodes.push_back(expr());
    }

    return nodes;
}

AST* Parser::expr() {
    AST* node = term();
    while (MATCH(idx, TOKEN_PLUS) || MATCH(idx, TOKEN_MINUS)) {
        TOKEN_TYPE op = TT(idx);
        idx++;
        node = new BinOpNode(node, op, term());
    }
    return node;
}

AST* Parser::term() {
    AST* node = factor();
    while (MATCH(idx, TOKEN_MULTIPLY) || MATCH(idx, TOKEN_DIVIDE)) {
        TOKEN_TYPE op = TT(idx);
        idx++;
        node = new BinOpNode(node, op, factor());
    }
    return node;
}

AST* Parser::factor() {
    TOKEN_TYPE op = TT(idx);
    if (op == TOKEN_STRING || op == TOKEN_NUMBER) {
        idx++;
        return new ValueNode(T(idx - 1).value);
    }
    else if (op == TOKEN_IDENTIFIER) {
        // Check if this is a function definition (followed by colon)
        int saveIdx = idx;
        idx++;
        if (MATCH(idx, TOKEN_LPAREN)) {
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
            
            // Convert AST* args to VariableNode* args for CallNode
            std::vector<VariableNode*> argNodes;
            for (auto* arg : args) {
                if (auto* varNode = dynamic_cast<VariableNode*>(arg)) {
                    argNodes.push_back(varNode);
                }
            }

            return new CallNode(funcName, argNodes);
        } else {
            // Check if this is a function call without parentheses: func arg1, arg2
            // Look ahead to see if next token is a valid argument (identifier, string, number)
            int nextIdx = idx;
            if (TT(nextIdx) == TOKEN_IDENTIFIER || TT(nextIdx) == TOKEN_STRING || TT(nextIdx) == TOKEN_NUMBER) {
                idx = saveIdx;
                std::string funcName = T(idx).value;
                idx++; // skip function name
                
                std::vector<AST*> args;
                // Collect arguments until end of line
                while (!isAtEnd() && TT(idx) != TOKEN_SEMICOLON && T(idx).row == T(saveIdx).row) {
                    if (TT(idx) == TOKEN_COMMA) {
                        idx++;
                        continue;
                    }
                    args.push_back(expr());
                }
                
                // Convert AST* args to VariableNode* args for CallNode
                std::vector<VariableNode*> argNodes;
                for (auto* arg : args) {
                    if (auto* varNode = dynamic_cast<VariableNode*>(arg)) {
                        argNodes.push_back(varNode);
                    } else if (auto* valNode = dynamic_cast<ValueNode*>(arg)) {
                        argNodes.push_back(new VariableNode(valNode->value, VARIANT));
                    }
                }
                
                return new CallNode(funcName, argNodes);
            }
        }
        if (MATCH(idx, TOKEN_COLON)) {
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
            return new FunctionNode(funcName, argNodes, body);
        } else {
            idx = saveIdx;
            idx++;
            return new VariableNode(T(idx - 1).value, VARIANT);
        }
    } else if (op == TOKEN_ASSIGNMENT) {
        const std::string name = T(idx - 1).value;
        idx++;
        return new AssignNode(name, factor());
    }
    else if (op == TOKEN_LPAREN) {
        const char openBracket = T(idx).value[0];
        char closeBracket;
        
        switch (openBracket) {
            case '(': closeBracket = ')'; break;
            case '[': closeBracket = ']'; break;
            case '{': closeBracket = '}'; break;
            default: closeBracket = ')';
        }
        
        idx++; // skip opening bracket
        AST* innerExpr = expr();
        
        if (!MATCH(idx, TOKEN_RPAREN)) {
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
    // Clean up any allocated resources if needed
}
