#pragma once
#include <string>
#include <stdexcept>
#include "lexer.hpp"
#include "newVector.hpp"
#include "ast.hpp"
extern struct Token;

enum class DeclarationType
{
    Declaration, FunctionDefinition, ELSE
};

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
    void putBackToken();
    void translationUnit();
    void externalDeclaration();
    DeclarationType isDeclarationOrFunctionDefinition();
    ASTNode* functionDefinition();
    ASTNode* declaration();
    ASTNode* initDeclaratorList();
    ASTNode* initDeclarator();
    ASTNode* declarator();
    ASTNode* parameterList();
    ASTNode* parameterDeclaration();
    ASTNode* typeSpecifier();
    ASTNode* initializer();
    ASTNode* assignmentExpression();
    ASTNode* conditionalExpression();
    ASTNode* logicalOrExpression();
    ASTNode* logicalAndExpression();
    ASTNode* equalityExpression();
    ASTNode* relationalExpression();
    ASTNode* inclusiveOrExpression();
    ASTNode* exclusiveOrExpression();
    ASTNode* andExpression();
    ASTNode* shiftExpression();
    ASTNode* castExpression();
    ASTNode* unaryExpression();
    ASTNode* postfixExpression();
    ASTNode* argumentExpressionList();
    ASTNode* additiveExpression();
    ASTNode* multiplicativeExpression();
    ASTNode* primaryExpression();
    ASTNode* compoundStatement();
    ASTNode* statement();
    ASTNode* selectionStatement();
    ASTNode* iterationStatement();
    ASTNode* jumpStatement();
    ASTNode* expressionStatement();
    ASTNode* expression();
};
