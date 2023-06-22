
#include "astParser.hpp"  // 语法分析器产生的头文件
#include "newVector.cpp"

// 公共接口，启动语法分析
void Parser::parse() {
    // 开始符号的递归调用
    translationUnit();

    // 判断是否成功解析了整个输入
    if (index == tokens.size()) {
        // 解析成功
        std::cout << "Parsing successful!" << std::endl;
    }
    else {
        // 解析失败，打印错误信息
        std::cout << "Parsing failed! Unexpected token: "
            << tokens[index].lexeme << std::endl;
    }
}

// 辅助函数，获取当前标记
Token Parser::getCurrentToken() const {
    return tokens[index];
}

// 辅助函数，移动到下一个标记
void Parser::consumeToken() {
    index++;
}

// 创建AST节点
ASTNode* Parser::createASTNode(const std::string& type, const std::string& value) {
    return new ASTNode(type, value);
}

// 连接子节点到父节点
void Parser::connectChildren(ASTNode* parent, const std::vector<ASTNode*>& children) {
    for (auto child : children) {
        parent->children.push_back(child);
    }
}

// 产生式规则：translation_unit -> external_declaration
void Parser::translationUnit() {
    externalDeclaration();
}

// 产生式规则：external_declaration -> function_definition | declaration
void Parser::externalDeclaration() {
    if (getCurrentToken().type == TokenType::Keywords && getCurrentToken().lexeme == "int") {
        functionDefinition();
    }
    else {
        declaration();
    }
}

// 产生式规则：function_definition -> type_specifier declarator compound_statement
void Parser::functionDefinition() {
    ASTNode* typeSpecifierNode = typeSpecifier();
    ASTNode* declaratorNode = declarator();
    ASTNode* compoundStatementNode = compoundStatement();

    ast = createASTNode("FunctionDefinition", "");
    connectChildren(ast, { typeSpecifierNode, declaratorNode, compoundStatementNode });
}

// 产生式规则：declaration -> type_specifier init_declarator_list ';'
void Parser::declaration() {
    ASTNode* typeSpecifierNode = typeSpecifier();
    ASTNode* initDeclaratorListNode = initDeclaratorList();

    if (getCurrentToken().type == TokenType::SEMICOLON) {
        consumeToken();
    }
    else {
        // 错误处理
        std::cout << "Expected ';' at the end of declaration." << std::endl;
    }

    ast = createASTNode("Declaration", "");
    connectChildren(ast, { typeSpecifierNode, initDeclaratorListNode });
}

// 产生式规则：init_declarator_list -> init_declarator (',' init_declarator)*
ASTNode* Parser::initDeclaratorList() {
    ASTNode* initDeclaratorNode = initDeclarator();
    std::vector<ASTNode*> children = { initDeclaratorNode };

    while (getCurrentToken().type == TokenType::COMMA) {
        consumeToken();
        initDeclaratorNode = initDeclarator();
        children.push_back(initDeclaratorNode);
    }

    ASTNode* initDeclaratorListNode = createASTNode("InitDeclaratorList", "");
    connectChildren(initDeclaratorListNode, children);

    return initDeclaratorListNode;
}

// 产生式规则：init_declarator -> declarator ('=' initializer)?
ASTNode* Parser::initDeclarator() {
    ASTNode* declaratorNode = declarator();
    ASTNode* initializerNode = nullptr;

    if (getCurrentToken().type == TokenType::ASSIGN) {
        consumeToken();
        initializerNode = initializer();
    }

    ASTNode* initDeclaratorNode = createASTNode("InitDeclarator", "");
    connectChildren(initDeclaratorNode, { declaratorNode, initializerNode });

    return initDeclaratorNode;
}

// 产生式规则：declarator -> identifier
ASTNode* Parser::declarator() {
    if (getCurrentToken().type == TokenType::IDENTIFIER) {
        std::string identifierValue = getCurrentToken().lexeme;
        consumeToken();

        ASTNode* declaratorNode = createASTNode("Declarator", identifierValue);
        return declaratorNode;
    }
    else {
        // 错误处理
        std::cout << "Expected identifier in declarator." << std::endl;
        return nullptr;
    }
}

// 产生式规则：type_specifier -> 'int' | 'float' | 'char'
ASTNode* Parser::typeSpecifier() {
    if (getCurrentToken().type == TokenType::Keyword &&
        (getCurrentToken().lexeme == "int" || getCurrentToken().lexeme == "float" || getCurrentToken().lexeme == "char")) {
        std::string typeSpecifierValue = getCurrentToken().lexeme;
        consumeToken();

        ASTNode* typeSpecifierNode = createASTNode("TypeSpecifier", typeSpecifierValue);
        return typeSpecifierNode;
    }
    else {
        // 错误处理
        std::cout << "Expected type specifier." << std::endl;
        return nullptr;
    }
}

// 产生式规则：initializer -> assignment_expression
ASTNode* Parser::initializer() {
    return assignmentExpression();
}

// 产生式规则：assignment_expression -> additive_expression
ASTNode* Parser::assignmentExpression() {
    return additiveExpression();
}

// 产生式规则：additive_expression -> multiplicative_expression (('+' | '-') multiplicative_expression)*
ASTNode* Parser::additiveExpression() {
    ASTNode* multiplicativeExpressionNode = multiplicativeExpression();
    std::vector<ASTNode*> children = { multiplicativeExpressionNode };

    while (getCurrentToken().type == TokenType::PLUS || getCurrentToken().type == TokenType::MINUS) {
        std::string operatorValue = getCurrentToken().lexeme;
        consumeToken();

        ASTNode* multiplicativeExpressionNode = multiplicativeExpression();
        ASTNode* additiveExpressionNode = createASTNode("AdditiveExpression", operatorValue);
        connectChildren(additiveExpressionNode, { children.back(), multiplicativeExpressionNode });
        children.back() = additiveExpressionNode;
    }

    return children.back();
}

// 产生式规则：multiplicative_expression -> primary_expression (('*' | '/') primary_expression)*
ASTNode* Parser::multiplicativeExpression() {
    ASTNode* primaryExpressionNode = primaryExpression();
    std::vector<ASTNode*> children = { primaryExpressionNode };

    while (getCurrentToken().type == TokenType::MULTIPLY || getCurrentToken().type == TokenType::DIVIDE) {
        std::string operatorValue = getCurrentToken().lexeme;
        consumeToken();

        ASTNode* primaryExpressionNode = primaryExpression();
        ASTNode* multiplicativeExpressionNode = createASTNode("MultiplicativeExpression", operatorValue);
        connectChildren(multiplicativeExpressionNode, { children.back(), primaryExpressionNode });
        children.back() = multiplicativeExpressionNode;
    }

    return children.back();
}

// 产生式规则：primary_expression -> identifier | constant | '(' expression ')'
ASTNode* Parser::primaryExpression() {
    if (getCurrentToken().type == TokenType::IDENTIFIER || getCurrentToken().type == TokenType::Constant) {
        std::string value = getCurrentToken().lexeme;
        consumeToken();

        ASTNode* primaryExpressionNode = createASTNode("PrimaryExpression", value);
        return primaryExpressionNode;
    }
    else if (getCurrentToken().type == TokenType::LEFT_PAREN) {
        consumeToken();
        ASTNode* expressionNode = expression();

        if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
            consumeToken();
            return expressionNode;
        }
        else {
            // 错误处理
            std::cout << "Expected ')' in primary expression." << std::endl;
            return nullptr;
        }
    }
    else {
        // 错误处理
        std::cout << "Expected identifier, constant, or '(' in primary expression." << std::endl;
        return nullptr;
    }
}

// 产生式规则：compound_statement -> '{' (declaration | statement)* '}'
ASTNode* Parser::compoundStatement() {
    if (getCurrentToken().type == TokenType::LEFT_BRACE) {
        consumeToken();
        std::vector<ASTNode*> children;

        while (getCurrentToken().type != TokenType::RIGHT_BRACE) {
            if (getCurrentToken().type == TokenType::Keyword && getCurrentToken().lexeme == "int") {
                children.push_back(declaration);
            }
            else {
                children.push_back(statement());
            }
        }

        if (getCurrentToken().type == TokenType::RIGHT_BRACE) {
            consumeToken();
        }
        else {
            // 错误处理
            std::cout << "Expected '}' at the end of compound statement." << std::endl;
            return nullptr;
        }

        ASTNode* compoundStatementNode = createASTNode("CompoundStatement", "");
        connectChildren(compoundStatementNode, children);
        return compoundStatementNode;
    }
    else {
        // 错误处理
        std::cout << "Expected '{' at the beginning of compound statement." << std::endl;
        return nullptr;
    }
}

// 产生式规则：statement -> compound_statement | expression_statement
ASTNode* Parser::statement() {
    if (getCurrentToken().type == TokenType::LEFT_BRACE) {
        return compoundStatement();
    }
    else {
        return expressionStatement();
    }
}

// 产生式规则：expression_statement -> expression? ';'
ASTNode* Parser::expressionStatement() {
    ASTNode* expressionNode = nullptr;

    if (getCurrentToken().type != TokenType::SEMICOLON) {
        expressionNode = expression();
    }

    if (getCurrentToken().type == TokenType::SEMICOLON) {
        consumeToken();
    }
    else {
        // 错误处理
        std::cout << "Expected ';' at the end of expression statement." << std::endl;
    }

    return expressionNode;
}

// 产生式规则：expression -> assignment_expression
ASTNode* Parser::expression() {
    return assignmentExpression();
}
