#pragma once
#include <string>
#include <stdexcept>
#include "lexer.hpp"
#include "newVector.hpp"
#include "ast.hpp"


class Parser {
public:
    Parser(const newVector<Token>& tokens)
        : tokens(tokens), index(0), ast(nullptr) {
    }

    // 公共接口，启动语法分析
    void parse();
    // 获取构建的AST
    ASTNode* getAST() const {
        return ast;
    }
private:
    newVector<Token> tokens;  // 词法分析器产生的标记序列
    size_t index;  // 当前处理的标记索引
    ASTNode* ast;  // 抽象语法树的根节点

    ASTNode* createASTNode(const std::string& type, const std::string& value);
    Token getCurrentToken() const;
    void connectChildren(ASTNode* parent, const std::vector<ASTNode*>& children);
    void consumeToken();
    void translationUnit();
    void externalDeclaration();
    void functionDefinition();
    void declaration();
    ASTNode* initDeclaratorList();
    ASTNode* initDeclarator();
    ASTNode* declarator();
    ASTNode* typeSpecifier();
    ASTNode* initializer();
    ASTNode* assignmentExpression();
    ASTNode* additiveExpression();
    ASTNode* multiplicativeExpression();
    ASTNode* primaryExpression();
    ASTNode* compoundStatement();
    ASTNode* statement();
    ASTNode* expressionStatement();
    ASTNode* expression();
};
