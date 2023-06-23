#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.hpp"
#include "newVector.hpp"
#include "newVector.cpp"
// Open this Project in UTF-8
// BNF 参考自 https://blog.csdn.net/Alexabc3000/article/details/126789474
void printASTNode(ASTNode* node, int indent = 0) {
    if (node == nullptr) {
        return;
    }

    // 打印缩进
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }

    // 打印节点类型和值
    std::cout << node->type << ": " << node->value << std::endl;

    // 打印子节点
    for (auto child : node->children) {
        printASTNode(child, indent + 1);
    }
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    std::filesystem::path file_path(argv[1]);
    if (!std::filesystem::exists(file_path)) {
        std::cerr << "File not found: " << file_path << "\n";
        return 1;
    }

    file_path = std::filesystem::absolute(file_path);

    std::cout << "Reading file: " << file_path << "\n";

    //Read file contents
    std::ifstream file_stream(file_path);
    if (!file_stream.is_open()) {
        std::cerr << "Failed to open file: " << file_path << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string file_contents = buffer.str();

    Lexer lexer(file_contents);
    newVector<Token> tokens = lexer.lex();

    for (const Token& token : tokens) {
        std::cout << "Token: " << static_cast<int>(token.type) << ", Lexeme: " << token.lexeme << ", Line: " << token.line << ", Column: " << token.column << "\n";
    }
    // 创建Parser对象并启动语法分析
    Parser parser(tokens);
    parser.parse();

    // 获取构建的AST
    ASTNode* ast = parser.getAST();
    if (ast != nullptr) {
        // 打印AST或执行其他操作
        std::cout << "AST constructed." << std::endl;
        printASTNode(ast);
    }

    // 释放AST内存
    delete ast;

    return 0;
}