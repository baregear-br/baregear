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
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <lexer.h>
#include <definations.h>

#define isAl(x) (x >= 'A' && x <= 'Z' || x >= 'a' && x <= 'z')
#define isNum(x) (x >= '1' && x <= '9')

std::vector<TOKEN> Lexer::lex() {
    std::vector<TOKEN> tokens;
    
    while (!isAtEnd()) {
        char32_t c = code[idx];
        TOKEN tkn;
        tkn.row = row;
        tkn.col = col;
        tkn.value = c;

        switch (c) {
            case '(':
            case '[':
            case '{':
                tkn.type = TOKEN_LPAREN;
                break;

            case ')':
            case ']':
            case '}':
                tkn.type = TOKEN_RPAREN;
                break;

            case '+':
                tkn.type = TOKEN_PLUS;
                break;

            case '-':
                if (code[idx + 1] == '-' && code[idx + 2] == '-') {
                    while (peek() != '-' || code[idx + 1] != '-'  || code[idx + 2] != '-') {
                        if (isAtEnd()) {
                            error("Multiline comment is not closed using '---'.", row, col);
                            break;
                        }
                        advance();
                    }
                }
                tkn.type = TOKEN_MINUS;
                break;

            case '*':
                tkn.type = TOKEN_MULTIPLY;
                break;

            case '/':
                tkn.type = TOKEN_DIVIDE;
                break;

            case '=':
                advance();
                if (peek() == '=')
                    tkn.type = TOKEN_EQUAL;
                else
                    tkn.type = TOKEN_ASSIGNMENT;
                break;

            case '>':
                advance();
                if (peek() == '=')
                    tkn.type = TOKEN_GREATER_EQUAL;
                else
                    tkn.type = TOKEN_GREATER;
                break;

            case '<':
                advance();
                if (peek() == '=')
                    tkn.type = TOKEN_SHORTER_EQUAL;
                else
                    tkn.type = TOKEN_SHORTER;
                break;

            case '#': {
                advance();
                std::string text = "#";
                if (isAl(code[idx])) {
                    while (isAl(code[idx]) || isNum(code[idx])) {
                        text += code[idx];
                        advance();
                    }
                    if (keywords.contains(text))
                        tkn.type = keywords.find(text)->second;
                    else {
                        while (code[idx - 1] != '\n')
                            advance();
                        continue;
                    }
                } else {
                    while (code[idx - 1] != '\n')
                        advance();
                    continue;
                }
                break;
            }

            case ':':
                tkn.type = TOKEN_COLON;
                break;

            case ',':
                tkn.type = TOKEN_COMMA;
                break;

            case ' ':
            case '\t':
            case '\r':
                advance();
                continue;
                
            case '\n':
                col = 1;
                row++;
                idx++;
                continue;
                
            case ';':
                tkn.type = TOKEN_SEMICOLON;
                break;
                
            case '&':
                tkn.type = TOKEN_AND;
                break;
                
            case '|':
                tkn.type = TOKEN_OR;
                break;
                
            case '^':
                tkn.type = TOKEN_XOR;
                break;
                
            case '!':
                tkn.type = TOKEN_NOT;
                break;

            case '\'':
            case '"': {
                advance();
                std::string text;
                while (peek() != '"') {
                    if (isAtEnd()) {
                        error("String literal is not closed with " + c + '.', row, col);
                        break;
                    }
                    text += code[idx];
                    advance();
                }
                advance(); // skip closing quote
                tkn.value = text;
                tkn.type = TOKEN_STRING;
                break;
            }

            default:
                std::string text;
                if (isAl(c)) {
                    text += c;
                    advance();
                    while (isAl(code[idx]) || isNum(code[idx])) {
                        text += code[idx];
                        advance();
                    }
                    tkn.value = text;
                    if (keywords.contains(text))
                        tkn.type = keywords.find(text)->second;
                    else
                        tkn.type = TOKEN_IDENTIFIER;
                } else if (isNum(c)) {
                    text += c;
                    advance();
                    while (isNum(code[idx])) {
                        text += code[idx];
                        advance();
                    }
                    tkn.value = text;
                    tkn.type = TOKEN_NUMBER;
                } else {
                    advance();
                    continue;
                }
                break;
        }
        
        tokens.push_back(tkn);
        advance();
    }
    
    return tokens;
}

bool Lexer::isAtEnd() {
    return idx > code.size();
}

void Lexer::advance() {
    if (getLine(row).size() > col) {
        col = 1;
        row++;
    }
    else
        col++;
    idx++;
}

char32_t Lexer::peek() {
    if (isAtEnd()) return '\0';
    return code[idx];
}