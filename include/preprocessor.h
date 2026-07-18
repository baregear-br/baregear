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
#include <lexer.h>
#include <parser.h>

class Preprocessor {
private:
    int idx;
    std::vector<AST*> nodes;
    bool inConditionalBlock = false;
    bool conditionalBlockActive = true;
    
    void processIfWhileNode(IfWhileNode* node);
    void optimizeBinOpNode(BinOpNode* node);
    AST* evaluateConstantExpression(AST* node);
    bool isConstant(AST* node);
public:
    Preprocessor(std::vector<AST*> n) : nodes(std::move(n)), idx(0) { }
    std::vector<AST*> preprocess();
};