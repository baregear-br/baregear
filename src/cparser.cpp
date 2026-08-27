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

#include <cparser.h>
#include <definations.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/raw_ostream.h>

// Global pointer to store the AST context for retrieval
static clang::ASTContext* g_astContext = nullptr;
static clang::TranslationUnitDecl* g_translationUnit = nullptr;
static std::string g_lastError;

class CParserASTConsumer : public clang::ASTConsumer {
public:
    void Initialize(clang::ASTContext& ctx) override {
        g_astContext = &ctx;
        g_translationUnit = ctx.getTranslationUnitDecl();
    }

    bool HandleTopLevelDecl(clang::DeclGroupRef DG) override {
        return true;
    }
};

class CParserFrontendAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, 
                                                           llvm::StringRef) override {
        return std::make_unique<CParserASTConsumer>();
    }

    void ExecuteAction() override {
        clang::ASTFrontendAction::ExecuteAction();
    }
};

class CParserDiagnosticConsumer : public clang::DiagnosticConsumer {
private:
    std::string errorMessages;

public:
    void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
                         const clang::Diagnostic& Info) override {
        llvm::SmallString<100> DiagMsg;
        Info.FormatDiagnostic(DiagMsg);
        errorMessages += DiagMsg.str().str() + "\n";
    }

    std::string getErrors() const { return errorMessages; }
    void clear() { errorMessages.clear(); }
};

CParser::CParser(const std::string& code, CLanguageMode mode)
    : sourceCode(code), languageMode(mode), hasErrors(false) {
}

CParser::~CParser() = default;

bool CParser::parse() {
    hasErrors = false;
    g_lastError.clear();
    g_astContext = nullptr;
    g_translationUnit = nullptr;

    // Configure compiler arguments based on language mode
    std::vector<std::string> args;
    
    if (languageMode == CLanguageMode::C) {
        args = {
            "cparser",
            "-std=c23",
            "-fsyntax-only",
            "-target", "x86_64-unknown-linux-gnu",
            "-I", "/usr/include"
        };
    } else {
        args = {
            "cparser",
            "-std=c++23",
            "-fsyntax-only",
            "-target", "x86_64-unknown-linux-gnu",
            "-I", "/usr/include"
        };
    }

    // Create frontend action
    auto action = std::make_unique<CParserFrontendAction>();
    
    // Run the tool on the code
    bool success = clang::tooling::runToolOnCodeWithArgs(
        std::move(action),
        sourceCode,
        args
    );

    if (!success || g_astContext == nullptr) {
        hasErrors = true;
        if (g_lastError.empty())
            g_lastError = "Failed to parse C/C++ code - unknown error";

        bugDetected(g_lastError.c_str());
        return false;
    }

    // Store the captured AST
    astContext.reset(g_astContext);
    translationUnit.reset(g_translationUnit);
    
    return true;
}

clang::ASTContext* CParser::getASTContext() const {
    return astContext.get();
}

clang::TranslationUnitDecl* CParser::getTranslationUnit() const {
    return translationUnit.get();
}

bool CParser::hasParseErrors() const {
    return hasErrors;
}

std::string CParser::getLastError() const {
    return g_lastError;
}
