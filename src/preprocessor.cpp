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
#include <parser.h>
#include <preprocessor.h>
#include <definations.h>

std::vector<AST*> Preprocessor::preprocess() {
    std::vector<AST*> optimizedNodes;
    
    for (idx = 0; idx < nodes.size(); idx++) {
        AST* node = nodes[idx];
        
        // Skip nodes in inactive conditional blocks
        if (!conditionalBlockActive && inConditionalBlock) {
            if (auto* ifNode = dynamic_cast<IfWhileNode*>(node)) {
                if (ifNode->sign == TOKEN_IF || ifNode->sign == TOKEN_ELSE) {
                    processIfWhileNode(ifNode);
                }
            }
            continue;
        }


        // Process different node types
        if (auto* ifNode = dynamic_cast<IfWhileNode*>(node)) {
            processIfWhileNode(ifNode);
            if (conditionalBlockActive) {
                optimizedNodes.push_back(node);
            }
        } else if (auto* binOpNode = dynamic_cast<BinOpNode*>(node)) {
            optimizeBinOpNode(binOpNode);
            optimizedNodes.push_back(node);
        } else if (auto* defineNode = dynamic_cast<DefineNode*>(node)) {
            // Define nodes for constants/macros - pass through to transpiler
            optimizedNodes.push_back(node);
        } else if (auto* featureNode = dynamic_cast<FeatureNode*>(node)) {
            // Feature nodes control compiler behavior - keep them for transpiler
            optimizedNodes.push_back(node);
        } else if (auto* inlineCodeNode = dynamic_cast<InlineCodeNode*>(node)) {
            // Inline code nodes pass through to transpiler
            optimizedNodes.push_back(node);
        } else if (auto* endNode = dynamic_cast<EndNode*>(node)) {
            // End nodes mark block endings - pass through
            optimizedNodes.push_back(node);
        } else {
            optimizedNodes.push_back(node);
        }
    }
    
    return optimizedNodes;
}

void Preprocessor::processIfWhileNode(IfWhileNode* node) {
    if (node->sign == TOKEN_IF) {
        // Evaluate #if condition
        if (node->condition) {
            AST* result = evaluateConstantExpression(node->condition);
            if (result && isConstant(result)) {
                inConditionalBlock = true;
                conditionalBlockActive = true;
            } else {
                inConditionalBlock = true;
                conditionalBlockActive = false;
            }
        }
    } else if (node->sign == TOKEN_ELSE) {
        // #else - activate if no previous block was active
        if (inConditionalBlock && !conditionalBlockActive) {
            conditionalBlockActive = true;
        }
    }
}

void Preprocessor::optimizeBinOpNode(BinOpNode* node) {
    // Constant folding: evaluate constant expressions at compile time
    if (isConstant(node->left) && isConstant(node->right)) {
        AST* result = evaluateConstantExpression(node);
        if (result) {
            // Replace the BinOpNode with the constant result
            // Note: In a real implementation, you'd need proper memory management here
            // For now, we'll update the node to be a ValueNode by copying the result
            auto* resultVal = dynamic_cast<ValueNode*>(result);
            if (resultVal) {
                node->left = result;
                node->right = nullptr;
                node->op = TOKEN_NUMBER;
            }
        }
    }
}

AST* Preprocessor::evaluateConstantExpression(AST* node) {
    if (auto* valNode = dynamic_cast<ValueNode*>(node)) {
        return valNode;
    }
    
    if (auto* binOpNode = dynamic_cast<BinOpNode*>(node)) {
        AST* left = evaluateConstantExpression(binOpNode->left);
        AST* right = evaluateConstantExpression(binOpNode->right);
        
        if (left && right && isConstant(left) && isConstant(right)) {
            auto* leftVal = dynamic_cast<ValueNode*>(left);
            auto* rightVal = dynamic_cast<ValueNode*>(right);
            
            // Try to parse as double for arithmetic operations
            try {
                double leftNum = std::stod(leftVal->value);
                double rightNum = std::stod(rightVal->value);
                double result = 0.0;
                
                switch (binOpNode->op) {
                    case TOKEN_PLUS:
                        result = leftNum + rightNum;
                        break;
                    case TOKEN_MINUS:
                        result = leftNum - rightNum;
                        break;
                    case TOKEN_MULTIPLY:
                        result = leftNum * rightNum;
                        break;
                    case TOKEN_DIVIDE:
                        if (rightNum != 0) {
                            result = leftNum / rightNum;
                        } else {
                            return nullptr; // Division by zero
                        }
                        break;
                    case TOKEN_EQUAL:
                        return new ValueNode(leftNum == rightNum ? "1" : "0");
                    case TOKEN_GREATER:
                        return new ValueNode(leftNum > rightNum ? "1" : "0");
                    case TOKEN_SHORTER:
                        return new ValueNode(leftNum < rightNum ? "1" : "0");
                    case TOKEN_GREATER_EQUAL:
                        return new ValueNode(leftNum >= rightNum ? "1" : "0");
                    case TOKEN_SHORTER_EQUAL:
                        return new ValueNode(leftNum <= rightNum ? "1" : "0");
                    case TOKEN_AND:
                        return new ValueNode((leftNum != 0 && rightNum != 0) ? "1" : "0");
                    case TOKEN_OR:
                        return new ValueNode((leftNum != 0 || rightNum != 0) ? "1" : "0");
                    case TOKEN_XOR:
                        return new ValueNode(((leftNum != 0) != (rightNum != 0)) ? "1" : "0");
                    default:
                        return nullptr;
                }
                
                // Convert result back to string
                std::string resultStr = std::to_string(result);
                // Remove trailing zeros for cleaner output
                resultStr.erase(resultStr.find_last_not_of('0') + 1, std::string::npos);
                if (resultStr.back() == '.') {
                    resultStr.pop_back();
                }
                return new ValueNode(resultStr);
                
            } catch (const std::exception&) {
                // If parsing as numbers fails, try string concatenation for plus
                if (binOpNode->op == TOKEN_PLUS) {
                    return new ValueNode(leftVal->value + rightVal->value);
                }
                return nullptr;
            }
        }
    }
    
    return nullptr;
}

bool Preprocessor::isConstant(AST* node) {
    return dynamic_cast<ValueNode*>(node) != nullptr;
}