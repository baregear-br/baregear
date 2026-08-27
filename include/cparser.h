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

#ifndef CARSER_H
#define CARSER_H

#include <string>
#include <memory>

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclBase.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>

enum class CLanguageMode {
    C,
    CPP
};

class CParser {
private:
    std::string sourceCode;
    CLanguageMode languageMode;
    std::unique_ptr<clang::ASTContext> astContext;
    std::unique_ptr<clang::TranslationUnitDecl> translationUnit;
    bool hasErrors;

public:
    CParser(const std::string& code, CLanguageMode mode = CLanguageMode::CPP);
    ~CParser();

    bool parse();
    clang::ASTContext* getASTContext() const;
    clang::TranslationUnitDecl* getTranslationUnit() const;
    bool hasParseErrors() const;
    std::string getLastError() const;
};

#endif // CARSER_H
