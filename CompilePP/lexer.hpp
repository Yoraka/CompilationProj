#pragma once 
#include <iostream>
#include <vector>
#include <cctype>
#include <string>
#include "newVector.hpp"
#include "astParser.hpp"

enum class TokenType {
    // Keywords
    IF, ELSE, WHILE, FOR, RETURN, //BREAK, CONTINUE, SWITCH, CASE, DEFAULT, DO, GOTO, CONST, STATIC, EXTERN, SIZEOF, TYPEDEF, //STRUCT, UNION, ENUM, VOID, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, SIGNED, UNSIGNED, AUTO, REGISTER, VOLATILE, INLINE, RESTRICT, BOOL, COMPLEX, IMAGINARY, ATOMIC, THREAD_LOCAL,

    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, NOT, SIZEOF, ASSIGN, EQUAL, NOT_EQUAL, LESS_THAN, GREATER_THAN,
    GREATER_THAN_OR_EQUAL_TO, LESS_THAN_OR_EQUAL_TO,
    PLUS_ASSIGN, MINUS_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN, MODULO_ASSIGN,
    INCREMENT, DECREMENT, LOGICAL_AND, LOGICAL_OR, BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    BITWISE_NOT, SHIFT_LEFT, SHIFT_RIGHT, SHIFT_RIGHT_UNSIGNED, TERNARY, ARROW, 

    // Delimiters
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET, SEMICOLON, COLON, COMMA, DOT,

    // Identifier
    IDENTIFIER, INTEGER, FLOAT, DOUBLE, STRING, CHARACTER, BOOLEAN, NULLPTR, 

    //Constant
    CONSTANT,

    // Preprocessor
    HASH, HASH_INCLUDE, HASH_DEFINE, HASH_IFDEF, HASH_IFNDEF, HASH_ELSE, HASH_ENDIF,

    // End of file
    END_OF_FILE
};

struct Token {
    TokenType type;         // 词法单元类型
    std::string lexeme;     // 词法单元的字符串值
    size_t line;               // 词法单元所在行号
    size_t column;             // 词法单元所在列号
    ~Token() {
        if (!lexeme.empty()) {
            lexeme.clear();
        }
    }
};

class Lexer {
public:
    const std::string& source_;
    newVector<Token> tokens_;
    size_t current_;
    size_t start_;
    size_t line_;
    size_t column_;

    Lexer(const std::string& source);

    newVector<Token> lex();

private:
    bool is_at_end() const;
    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    void number_literal();
    void string_literal();
    void identifier();
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);
    void skip_whitespace();
    void skip_comment();
    void add_token(std::tuple<TokenType,std::string>);
    void add_token(TokenType type, const std::string& lexeme);
    // void add_token(TokenType type, double value);
    // void add_token(TokenType type, int value);
    // void add_token(TokenType type, const std::string& lexeme, double value);
    // void add_token(TokenType type, const std::string& lexeme, int value);
    // void add_token(TokenType type, const std::string& lexeme, char value);
    // void add_token(TokenType type, const std::string& lexeme, const std::string& value);
    void error(const std::string& message);


};