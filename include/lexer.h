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

#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

enum TOKEN_TYPE {
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_EQUAL,
    TOKEN_GREATER,
    TOKEN_SHORTER,
    TOKEN_GREATER_EQUAL,
    TOKEN_SHORTER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_XOR,
    TOKEN_NOT,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGNMENT,
    TOKEN_COMMA,

    // Extended Tokens
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_SHORT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ELIF,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT_CASE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_STRUCT,
    TOKEN_CLASS,
    TOKEN_CREATE,
    TOKEN_DESTROY,
    TOKEN_PUBLIC,
    TOKEN_PRIVATE,
    TOKEN_PROTECTED,
    TOKEN_NUMB,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_THROW,
    TOKEN_TEXT,
    TOKEN_DEFINE,
    TOKEN_ERROR,
    TOKEN_WARNING,
    TOKEN_C,
    TOKEN_CPP,
    TOKEN_ASM,
    TOKEN_FEATURE,
    TOKEN_NO_FEATURE,
    TOKEN_END,
    TOKEN_RETURN
};

inline std::unordered_map<std::string, TOKEN_TYPE> keywords {
        { "if",             TOKEN_IF },
        { "and",            TOKEN_AND},
        { "or",             TOKEN_OR},
        { "not",            TOKEN_NOT},
        { "xor",            TOKEN_XOR},
        { "equ",            TOKEN_EQUAL},
        { "else",           TOKEN_ELSE },
        { "elif",           TOKEN_ELIF },
        { "switch",         TOKEN_SWITCH },
        { "case",           TOKEN_CASE },
        { "default",        TOKEN_DEFAULT_CASE },
        { "for",            TOKEN_FOR },
        { "while",          TOKEN_WHILE },
        { "struct",         TOKEN_STRUCT },
        { "class",          TOKEN_CLASS },
        { "create",         TOKEN_CREATE },
        { "destroy",        TOKEN_DESTROY },
        { "public",         TOKEN_PUBLIC },
        { "private",        TOKEN_PRIVATE },
        { "protected",      TOKEN_PROTECTED },
        { "int",            TOKEN_INT },
        { "float",          TOKEN_FLOAT},
        { "double",         TOKEN_DOUBLE },
        { "short",          TOKEN_SHORT },
        { "numb",           TOKEN_NUMB },
        { "text",           TOKEN_TEXT },
        { "try",            TOKEN_TRY },
        { "catch",          TOKEN_CATCH },
        { "throw",          TOKEN_THROW },
        { "return",         TOKEN_RETURN },
        { "#if",            TOKEN_IF },
        { "#else",          TOKEN_ELSE },
        { "#define",        TOKEN_DEFINE },
        { "#error",         TOKEN_ERROR },
        { "#warning",       TOKEN_WARNING },
        { "#c",             TOKEN_C },
        { "#cpp",           TOKEN_CPP },
        { "#asm",           TOKEN_ASM },
        { "#feature",       TOKEN_FEATURE },
        { "#nofeature",     TOKEN_NO_FEATURE },
        { "#end",           TOKEN_END },
};

struct TOKEN {
    int row = 1, col = 1;
    std::string value;
    TOKEN_TYPE type;
};

class Lexer {
private:
    int row = 1, col = 1, idx = 0;
    std::string code;

    void advance();
    char32_t peek();
    bool isAtEnd();
public:
    Lexer(std::string c) : code(c) { }
    std::vector<TOKEN> lex();
    ~Lexer();
};

#endif // LEXER_H