
#include "astParser.hpp"  // 语法分析器产生的头文件
#include "newVector.cpp"

// 公共接口，启动语法分析
void Parser::parse() {
    // 开始符号的递归调用
    translationUnit();

    // 判断是否成功解析了整个输入
    if (index == tokens.size() - 1) {
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

// 辅助函数，移动到上一个标记
void Parser::putBackToken() {
    index--;
}

DeclarationType Parser::isDeclarationOrFunctionDefinition() {
    // 备份当前的标记位置
    size_t currentPosition = index;

    if (getCurrentToken().type <= TokenType::NULLPTR && getCurrentToken().type >= TokenType::INTEGER) {
        consumeToken();
    }
    else {
		//error
		return DeclarationType::ELSE;
	}
    if (getCurrentToken().type == TokenType::IDENTIFIER) {
        consumeToken();
    }
    else {
        //error
        return DeclarationType::ELSE;
    }

    // 解析函数定义的声明符
    if (getCurrentToken().type == TokenType::LEFT_PAREN) {
        index = currentPosition;
        return DeclarationType::FunctionDefinition; // 是函数定义
    }
    //回退到之前的位置
    index = currentPosition;
    return DeclarationType::Declaration; // 是声明
}

// 创建AST节点
ASTNode* Parser::createASTNode(const std::string& type, const std::string& value = "") {
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
    ast = createASTNode("ExternalDeclaration", "");
    while (index < tokens.size() - 1) {
        externalDeclaration();
    }
}

// 产生式规则：external_declaration -> function_definition | declaration
void Parser::externalDeclaration() {
    ASTNode* functionDefinitionNode = createASTNode("PFunctionDefinitionNode","");
    ASTNode* declarationNode = createASTNode("PDeclarationNode","");
    // 判断式，不消耗Token
    if (isDeclarationOrFunctionDefinition() == DeclarationType::FunctionDefinition) {
		functionDefinitionNode = functionDefinition();
	}
    else if (isDeclarationOrFunctionDefinition() == DeclarationType::Declaration) {
		declarationNode = declaration();
	}
    else {
		// 错误处理
		std::cout << "Expected declaration or function definition." << std::endl;
		return;
	}
    if (functionDefinitionNode->children.size() != 0) {
        ast->addChild(functionDefinitionNode);
    }
    if (declarationNode->children.size() != 0) {
        ast->addChild(declarationNode);
    }

}

// 产生式规则：function_definition -> type_specifier declarator compound_statement
ASTNode* Parser::functionDefinition() {
    ASTNode* functionDefinitionNode = createASTNode("FunctionDefinitionNode","");

    ASTNode* typeSpecifierNode = typeSpecifier();
    ASTNode* declaratorNode = declarator();
    ASTNode* compoundStatementNode = compoundStatement();

    //ast = createASTNode("FunctionDefinition", "");
    functionDefinitionNode->addChild(typeSpecifierNode);
    functionDefinitionNode->addChild(declaratorNode);
    functionDefinitionNode->addChild(compoundStatementNode);
    return functionDefinitionNode;
}

// 产生式规则：declaration -> type_specifier init_declarator_list ';'
ASTNode* Parser::declaration() {
    ASTNode* declarationNode = createASTNode("DeclarationNode","");
    ASTNode* typeSpecifierNode = typeSpecifier();
    ASTNode* initDeclaratorListNode = initDeclaratorList();

    if (getCurrentToken().type == TokenType::SEMICOLON) {
        consumeToken();
    }
    else {
        // 错误处理
        std::cout << "Expected ';' at the end of declaration." << std::endl;
        return nullptr;
    }

    //ast = createASTNode("Declaration", "");

    connectChildren(declarationNode, { typeSpecifierNode, initDeclaratorListNode });
    return declarationNode;
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

// 产生式规则：declarator -> identifier parameter_list
ASTNode* Parser::declarator() {
    if (getCurrentToken().type == TokenType::IDENTIFIER) {
        std::string identifierValue = getCurrentToken().lexeme;
        consumeToken(); // 消耗掉当前标识符标记

        // 检查是否有参数列表
        if (getCurrentToken().type == TokenType::LEFT_PAREN) {
            consumeToken(); // 消耗掉左括号

            // 解析参数列表
            ASTNode* parameters = nullptr;
            if (getCurrentToken().type != TokenType::RIGHT_PAREN) {
                parameters = parameterList();
            }

            // 检查右括号
            if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
                consumeToken(); // 消耗掉右括号

                // 创建声明符节点，并将标识符和参数列表作为属性
                ASTNode* declaratorNode = createASTNode("Declarator", identifierValue);
                connectChildren(declaratorNode, { parameters });
                return declaratorNode;
            }
            else {
                // 错误处理：缺少右括号
                std::cout << "Expected ')' after parameter list in declarator." << std::endl;
                // 其他错误处理逻辑，例如抛出异常或采取适当的措施
                return nullptr;
            }
        }

        // 创建只包含标识符的声明符节点
        ASTNode* declaratorNode = createASTNode("Declarator", identifierValue);
        return declaratorNode;
    }
    else {
        // 错误处理
        std::cout << "Expected identifier in declarator." << std::endl;
        // 其他错误处理逻辑，例如抛出异常或采取适当的措施
        return nullptr;
    }
}

// 产生式规则：parameter_list -> '(' parameter_declaration ')'
ASTNode* Parser::parameterList() {
    ASTNode* parameterListNode = createASTNode("ParameterList", "");

    connectChildren(parameterListNode, { parameterDeclaration() });

    while (getCurrentToken().type == TokenType::COMMA) {
        consumeToken(); // 消耗逗号
        connectChildren(parameterListNode, { parameterDeclaration() });
    }

    return parameterListNode;
}

// 产生式规则：parameter_declaration -> declaration_specifiers identifier
ASTNode* Parser::parameterDeclaration() {
    // TODO:typeSpecifier() change to declarationSpecifiers()
    ASTNode* declarationSpecifiersNode = typeSpecifier();

    if (getCurrentToken().type == TokenType::IDENTIFIER) {
        std::string identifierValue = getCurrentToken().lexeme;
        consumeToken(); // 消耗标识符

        ASTNode* parameterDeclarationNode = createASTNode("ParameterDeclaration","");
        connectChildren(parameterDeclarationNode, { declarationSpecifiersNode });
        connectChildren(parameterDeclarationNode, { createASTNode("Identifier", identifierValue) });

        return parameterDeclarationNode;
    }
    else {
        // 错误处理
        std::cout << "Expected identifier in parameter declaration." << std::endl;
        // 其他错误处理逻辑，例如抛出异常或采取适当的措施
        return nullptr;
    }
}

// TODO:产生式规则：declaration_specifiers ::= storage_class_specifier
//                                          | type_specifier
//                                          | type_qualifier
//                                          | function_specifier
//                                          | declaration_specifiers storage_class_specifier
//                                          | declaration_specifiers type_specifier
//                                          | declaration_specifiers type_qualifier
//                                          | declaration_specifiers function_specifier


// 产生式规则：type_specifier -> 'int' | 'float' | 'char'
ASTNode* Parser::typeSpecifier() {
    // TODO:Stupid Design, need to be improved
    if (getCurrentToken().type <= TokenType::NULLPTR && getCurrentToken().type >= TokenType::INTEGER
        ) {
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

// 产生式规则：assignment_expression ::= conditional_expression
//| unary_expression assignment_operator assignment_expression
ASTNode* Parser::assignmentExpression() {
    ASTNode* exprNode = conditionalExpression();
    auto isAssignmentOperator = [](TokenType type) {
        // 返回true或false，表示是否是赋值操作符
        return type == TokenType::ASSIGN || type == TokenType::PLUS_ASSIGN || type == TokenType::MINUS_ASSIGN
            || type == TokenType::MULTIPLY_ASSIGN || type == TokenType::DIVIDE_ASSIGN || type == TokenType::MODULO_ASSIGN
            ;
    };
    if (isAssignmentOperator(getCurrentToken().type)) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗赋值操作符

        ASTNode* assignmentExprNode = assignmentExpression();

        ASTNode* binaryExprNode = createASTNode("AssignmentExpression","");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(assignmentExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}

ASTNode* Parser::conditionalExpression() {
    ASTNode* exprNode = logicalOrExpression();

    if (getCurrentToken().type == TokenType::TERNARY) {
        consumeToken(); // 消耗问号

        ASTNode* trueExprNode = expression();

        if (getCurrentToken().type != TokenType::COLON) {
            // 错误处理：缺少冒号
            std::cout << "Expected ':' in conditional expression." << std::endl;
            return nullptr;
        }
        consumeToken(); // 消耗冒号

        ASTNode* falseExprNode = conditionalExpression();

        ASTNode* ternaryExprNode = createASTNode("ConditionalExpression");
        ternaryExprNode->addChild(exprNode);
        ternaryExprNode->addChild(trueExprNode);
        ternaryExprNode->addChild(falseExprNode);

        exprNode = ternaryExprNode;
    }

    return exprNode;
}


ASTNode* Parser::logicalOrExpression() {
    ASTNode* exprNode = logicalAndExpression();

    while (getCurrentToken().type == TokenType::LOGICAL_OR) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗逻辑或操作符

        ASTNode* nextExprNode = logicalAndExpression();

        ASTNode* binaryExprNode = createASTNode("LogicalOrExpression","");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}

ASTNode* Parser::logicalAndExpression() {
    ASTNode* exprNode = inclusiveOrExpression();

    while (getCurrentToken().type == TokenType::LOGICAL_AND) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗逻辑与操作符

        ASTNode* nextExprNode = inclusiveOrExpression();

        ASTNode* binaryExprNode = createASTNode("LogicalAndExpression");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}

ASTNode* Parser::inclusiveOrExpression() {
    ASTNode* exprNode = exclusiveOrExpression();

    while (getCurrentToken().type == TokenType::BITWISE_OR) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗按位或操作符

        ASTNode* nextExprNode = exclusiveOrExpression();

        ASTNode* binaryExprNode = createASTNode("InclusiveOrExpression");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}



ASTNode* Parser::exclusiveOrExpression() {
    ASTNode* exprNode = andExpression();

    while (getCurrentToken().type == TokenType::BITWISE_XOR) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗按位异或操作符

        ASTNode* nextExprNode = andExpression();

        ASTNode* binaryExprNode = createASTNode("ExclusiveOrExpression");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}

ASTNode* Parser::andExpression() {
    ASTNode* exprNode = equalityExpression();

    while (getCurrentToken().type == TokenType::BITWISE_AND) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗按位与操作符

        ASTNode* nextExprNode = equalityExpression();

        ASTNode* binaryExprNode = createASTNode("AndExpression");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}

ASTNode* Parser::equalityExpression() {
    ASTNode* exprNode = relationalExpression();
    auto isEqualityOperator = [](TokenType type) {
        // 返回true或false，表示是否是相等性操作符
        return type == TokenType::EQUAL || type == TokenType::NOT_EQUAL;
    };
    while (isEqualityOperator(getCurrentToken().type)) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗相等性操作符

        ASTNode* nextExprNode = relationalExpression();

        ASTNode* binaryExprNode = createASTNode("EqualityExpression");
        binaryExprNode->addChild(exprNode);
        binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        binaryExprNode->addChild(nextExprNode);

        exprNode = binaryExprNode;
    }

    return exprNode;
}


ASTNode* Parser::relationalExpression() {
	ASTNode* exprNode = shiftExpression();
    auto isRelationalOperator = [](TokenType type) {
        // 返回true或false，表示是否是关系操作符
        return type == TokenType::LESS_THAN || type == TokenType::GREATER_THAN ||
            type == TokenType::LESS_THAN_OR_EQUAL_TO || type == TokenType::GREATER_THAN_OR_EQUAL_TO;
    };
    while (isRelationalOperator(getCurrentToken().type)) {
		Token operatorToken = getCurrentToken();
		consumeToken(); // 消耗关系操作符

		ASTNode* nextExprNode = shiftExpression();

		ASTNode* binaryExprNode = createASTNode("RelationalExpression");
		binaryExprNode->addChild(exprNode);
		binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
		binaryExprNode->addChild(nextExprNode);

		exprNode = binaryExprNode;
	}

	return exprNode;
}

ASTNode* Parser::shiftExpression() {
	ASTNode* exprNode = additiveExpression();
    auto isShiftOperator = [](TokenType type) {
        // 返回true或false，表示是否是移位操作符
        return type == TokenType::SHIFT_LEFT || type == TokenType::SHIFT_RIGHT || type == TokenType::SHIFT_RIGHT_UNSIGNED;
    };
    while (isShiftOperator(getCurrentToken().type)) {
		Token operatorToken = getCurrentToken();
		consumeToken(); // 消耗移位操作符

		ASTNode* nextExprNode = additiveExpression();

		ASTNode* binaryExprNode = createASTNode("ShiftExpression");
		binaryExprNode->addChild(exprNode);
		binaryExprNode->addChild(createASTNode(operatorToken.lexeme));
		binaryExprNode->addChild(nextExprNode);

		exprNode = binaryExprNode;
	}

	return exprNode;
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

// TODO: castExpression
ASTNode* Parser::castExpression() {
    if (getCurrentToken().type == TokenType::LEFT_PAREN) {
        consumeToken(); // 消耗左括号

        ASTNode* typeNameNode = createASTNode("typenameTemp");//typeName();

        if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
            consumeToken(); // 消耗右括号

            ASTNode* castExprNode = castExpression();

            ASTNode* castExpressionNode = createASTNode("CastExpression");
            castExpressionNode->addChild(createASTNode("("));
            castExpressionNode->addChild(typeNameNode);
            castExpressionNode->addChild(createASTNode(")"));
            castExpressionNode->addChild(castExprNode);

            return castExpressionNode;
        }
        else {
            // 错误处理：缺少右括号
            std::cout << "Expected ')' after type name in cast expression." << std::endl;
            return nullptr;
        }
    }
    else {
        return unaryExpression();
    }
}


ASTNode* Parser::unaryExpression() {
    auto isUnaryOperator = [](TokenType type) {
        // 返回true或false，表示是否是一元操作符
        return type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::NOT;
    };
    if (isUnaryOperator(getCurrentToken().type)) {
        Token operatorToken = getCurrentToken();
        consumeToken(); // 消耗一元操作符

        ASTNode* castExprNode = castExpression();

        ASTNode* unaryExprNode = createASTNode("UnaryExpression");
        unaryExprNode->addChild(createASTNode(operatorToken.lexeme));
        unaryExprNode->addChild(castExprNode);

        return unaryExprNode;
    }
    // TODO:SIZEOF
    else if (getCurrentToken().type == TokenType::SIZEOF) {
        Token sizeofToken = getCurrentToken();
        consumeToken(); // 消耗 sizeof 关键字

        if (getCurrentToken().type == TokenType::LEFT_PAREN) {
            consumeToken(); // 消耗左括号

            ASTNode* typeNameNode = createASTNode("typenameTemp");//typeName();

            if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
                consumeToken(); // 消耗右括号

                ASTNode* sizeofExprNode = createASTNode("SizeofExpression");
                sizeofExprNode->addChild(createASTNode(sizeofToken.lexeme));
                sizeofExprNode->addChild(createASTNode("("));
                sizeofExprNode->addChild(typeNameNode);
                sizeofExprNode->addChild(createASTNode(")"));

                return sizeofExprNode;
            } else {
                // 错误处理：缺少右括号
                std::cout << "Expected ')' after type name in sizeof expression." << std::endl;
                return nullptr;
            }
        }
        else {
            ASTNode* unaryExprNode = createASTNode("SizeofExpression");
            unaryExprNode->addChild(createASTNode(sizeofToken.lexeme));
            unaryExprNode->addChild(castExpression());

            return unaryExprNode;
        }
    }
    else {
        return postfixExpression();
    }
}


// 产生式规则：postfix_expression -> primary_expression (('[' expression ']') | ('(' ')') | ('(' argument_expression_list ')') | ('.' identifier) | ('->' identifier) | ('++') | ('--'))*
ASTNode* Parser::postfixExpression() {
    ASTNode* exprNode = primaryExpression();

    while (true) {
        if (getCurrentToken().type == TokenType::LEFT_BRACKET) {
            consumeToken(); // 消耗左方括号

            ASTNode* indexExprNode = expression();

            if (getCurrentToken().type == TokenType::RIGHT_BRACKET) {
                consumeToken(); // 消耗右方括号

                ASTNode* arrayAccessNode = createASTNode("ArrayAccess");
                arrayAccessNode->addChild(exprNode);
                arrayAccessNode->addChild(createASTNode("["));
                arrayAccessNode->addChild(indexExprNode);
                arrayAccessNode->addChild(createASTNode("]"));

                exprNode = arrayAccessNode;
            }
            else {
                // 错误处理：缺少右方括号
                std::cout << "Expected ']' after array index." << std::endl;
                return nullptr;
            }
        }
        else if (getCurrentToken().type == TokenType::LEFT_PAREN) {
            consumeToken(); // 消耗左括号

            if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
                consumeToken(); // 消耗右括号

                ASTNode* functionCallNode = createASTNode("FunctionCall");
                functionCallNode->addChild(exprNode);
                functionCallNode->addChild(createASTNode("("));
                functionCallNode->addChild(createASTNode(")"));

                exprNode = functionCallNode;
            }
            else {
                ASTNode* argExprListNode = argumentExpressionList();

                if (getCurrentToken().type == TokenType::LEFT_PAREN) {
                    consumeToken(); // 消耗右括号

                    ASTNode* functionCallNode = createASTNode("FunctionCall");
                    functionCallNode->addChild(exprNode);
                    functionCallNode->addChild(createASTNode("("));
                    functionCallNode->addChild(argExprListNode);
                    functionCallNode->addChild(createASTNode(")"));

                    exprNode = functionCallNode;
                }
                else {
                    // 错误处理：缺少右括号
                    std::cout << "Expected ')' after function arguments." << std::endl;
                    return nullptr;
                }
            }
        }
        else if (getCurrentToken().type == TokenType::DOT || getCurrentToken().type == TokenType::ARROW) {
            Token operatorToken = getCurrentToken();
            consumeToken(); // 消耗点号或箭头

            if (getCurrentToken().type == TokenType::IDENTIFIER) {
                std::string identifierValue = getCurrentToken().lexeme;
                consumeToken(); // 消耗标识符

                ASTNode* memberAccessNode = createASTNode("MemberAccess");
                memberAccessNode->addChild(exprNode);
                memberAccessNode->addChild(createASTNode(operatorToken.lexeme));
                memberAccessNode->addChild(createASTNode(identifierValue));

                exprNode = memberAccessNode;
            }
            else {
                // 错误处理：点号或箭头后缺少标识符
                std::cout << "Expected identifier after '.' or '->'." << std::endl;
                return nullptr;
            }
        }
        else if (getCurrentToken().type == TokenType::INCREMENT || getCurrentToken().type == TokenType::DECREMENT) {
            Token operatorToken = getCurrentToken();
            consumeToken(); // 消耗自增或自减操作符

            ASTNode* postfixExprNode = createASTNode("PostfixExpression");
            postfixExprNode->addChild(exprNode);
            postfixExprNode->addChild(createASTNode(operatorToken.lexeme));

            exprNode = postfixExprNode;
        }
        else {
            break;
        }
    }

    return exprNode;
}

ASTNode* Parser::argumentExpressionList() {
    ASTNode* argExprListNode = createASTNode("ArgumentExpressionList");

    ASTNode* exprNode = assignmentExpression();
    argExprListNode->addChild(exprNode);

    while (getCurrentToken().type == TokenType::COMMA) {
        consumeToken(); // 消耗逗号

        exprNode = assignmentExpression();
        argExprListNode->addChild(exprNode);
    }

    return argExprListNode;
}

// 产生式规则：primary_expression -> identifier | constant | string | '(' expression ')'
ASTNode* Parser::primaryExpression() {
    if (getCurrentToken().type == TokenType::IDENTIFIER || 
        getCurrentToken().type == TokenType::CONSTANT
        ) {
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

        while (getCurrentToken().type != TokenType::RIGHT_BRACE && index < tokens.size()) {
            if (isDeclarationOrFunctionDefinition() == DeclarationType::Declaration) {
                children.push_back(declaration());
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
    else if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF) {
        if (getCurrentToken().lexeme == "if") {
            return selectionStatement();
        }
        else if (getCurrentToken().lexeme == "while") {
            return iterationStatement();
        }
        else if (getCurrentToken().lexeme == "return") {
			return jumpStatement();
		}
        else if (getCurrentToken().lexeme == "break") {
            return jumpStatement();
        }
        else if (getCurrentToken().lexeme == "continue") {
			return jumpStatement();
		}
        else {
			// 错误处理：不支持的语句类型
			std::cout << "Unsupported statement type." << std::endl;
			consumeToken();
			return nullptr;
		}
    }
    else if (getCurrentToken().type == TokenType::IDENTIFIER || getCurrentToken().type == TokenType::CONSTANT) {
        return expressionStatement();
    }
    else {
        // 错误处理：不支持的语句类型
        std::cout << "Unsupported statement type." << std::endl;
        consumeToken();
        return nullptr;
    }
}

ASTNode* Parser::selectionStatement() {
    consumeToken(); // 消耗关键字 if

    if (getCurrentToken().type != TokenType::LEFT_PAREN) {
        // 错误处理：期望左括号
        std::cout << "Expected '(' after 'if' in selection statement." << std::endl;
        return nullptr;
    }

    consumeToken(); // 消耗左括号

    ASTNode* selectionStmtNode = createASTNode("SelectionStatement","");
    connectChildren(selectionStmtNode, { expression() });

    if (getCurrentToken().type != TokenType::RIGHT_PAREN) {
        // 错误处理：期望右括号
        std::cout << "Expected ')' after expression in selection statement." << std::endl;
        return nullptr;
    }

    consumeToken(); // 消耗右括号

    connectChildren(selectionStmtNode,{statement()});

    if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "else") {
        consumeToken(); // 消耗关键字 else
        connectChildren(selectionStmtNode,{statement()});
    }

    return selectionStmtNode;
}

ASTNode* Parser::iterationStatement() {
    if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "while") {
        consumeToken(); // 消耗关键字 while

        if (getCurrentToken().type != TokenType::LEFT_PAREN) {
            // 错误处理：期望左括号
            std::cout << "Expected '(' after 'while' in iteration statement." << std::endl;
            return nullptr;
        }

        consumeToken(); // 消耗左括号

        ASTNode* iterationStmtNode = createASTNode("IterationStatement","");
        connectChildren(iterationStmtNode, { expression() });

        if (getCurrentToken().type != TokenType::RIGHT_PAREN) {
            // 错误处理：期望右括号
            std::cout << "Expected ')' after expression in iteration statement." << std::endl;
            return nullptr;
        }

        consumeToken(); // 消耗右括号
        connectChildren(iterationStmtNode, { statement() });
        return iterationStmtNode;
    }
    else if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "for") {
        consumeToken(); // 消耗关键字 for

        if (getCurrentToken().type != TokenType::LEFT_PAREN) {
            // 错误处理：期望左括号
            std::cout << "Expected '(' after 'for' in iteration statement." << std::endl;
            return nullptr;
        }

        consumeToken(); // 消耗左括号

        ASTNode* iterationStmtNode = createASTNode("IterationStatement","");
        ASTNode* expressionStmt1 = expressionStatement();
        ASTNode* expressionStmt2 = expressionStatement();

        if (getCurrentToken().type == TokenType::RIGHT_PAREN) {
            // for循环没有第三个表达式
            consumeToken(); // 消耗右括号
        }
        else {
            connectChildren(iterationStmtNode, { expression() });

            if (getCurrentToken().type != TokenType::RIGHT_PAREN) {
                // 错误处理：期望右括号
                std::cout << "Expected ')' after expression in iteration statement." << std::endl;
                return nullptr;
            }

            consumeToken(); // 消耗右括号
        }

        connectChildren(iterationStmtNode, { statement() });
        return iterationStmtNode;
    }
    else {
        // 错误处理：不支持的迭代语句类型
        std::cout << "Unsupported iteration statement type." << std::endl;
        consumeToken();
        return nullptr;
    }
}

ASTNode* Parser::jumpStatement() {
    if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "continue") {
        consumeToken(); // 消耗关键字 continue

        if (getCurrentToken().type == TokenType::SEMICOLON) {
            consumeToken(); // 消耗分号
            return createASTNode("JumpStatement", "continue");
        }
        else {
            // 错误处理：缺少分号
            std::cout << "Expected ';' after 'continue' in jump statement." << std::endl;
            return nullptr;
        }
    }
    else if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "break") {
        consumeToken(); // 消耗关键字 break

        if (getCurrentToken().type == TokenType::SEMICOLON) {
            consumeToken(); // 消耗分号
            return createASTNode("JumpStatement", "break");
        }
        else {
            // 错误处理：缺少分号
            std::cout << "Expected ';' after 'break' in jump statement." << std::endl;
            return nullptr;
        }
    }
    else if (getCurrentToken().type <= TokenType::RETURN && getCurrentToken().type >= TokenType::IF && getCurrentToken().lexeme == "return") {
        consumeToken(); // 消耗关键字 return

        if (getCurrentToken().type == TokenType::SEMICOLON) {
            consumeToken(); // 消耗分号
            return createASTNode("JumpStatement", "return");
        }
        else {
            ASTNode* jumpStmtNode = createASTNode("JumpStatement", "return");
            connectChildren(jumpStmtNode, { expression() });

            if (getCurrentToken().type == TokenType::SEMICOLON) {
                consumeToken(); // 消耗分号
            }
            else {
                // 错误处理：缺少分号
                std::cout << "Expected ';' after 'return' in jump statement." << std::endl;
            }

            return jumpStmtNode;
        }
    }
    else {
        // 错误处理：不支持的跳转语句类型
        std::cout << "Unsupported jump statement type." << std::endl;
        consumeToken();
        return nullptr;
    }
}

// 产生式规则：expression_statement -> expression? ';'
ASTNode* Parser::expressionStatement() {
    ASTNode* expressionNode = createASTNode("ExpressionNode","");

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

// 产生式规则：expression ::= assignment_expression | expression ',' assignment_expression
ASTNode* Parser::expression() {
    ASTNode* exprNode = assignmentExpression();

    while (getCurrentToken().type == TokenType::COMMA) {
        consumeToken(); // 消耗逗号

        ASTNode* nextExprNode = assignmentExpression();

        ASTNode* commaExprNode = createASTNode("CommaExpression");
        commaExprNode->addChild(exprNode);
        commaExprNode->addChild(nextExprNode);

        exprNode = commaExprNode;
    }

    return exprNode;
}
