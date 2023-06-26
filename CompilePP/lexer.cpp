#include "Lexer.hpp"
#include "newVector.cpp"

Lexer::Lexer(const std::string& source) : source_(source), current_(0), line_(1), column_(0) {}

newVector<Token> Lexer::lex() {
    while (!is_at_end()) {
        char c = advance();

        switch (c) {
            // Preprocessor
            case '#': add_token(TokenType::HASH,"#"); break;

            // Single-character tokens
            case '(': add_token(TokenType::LEFT_PAREN,"("); break;
            case ')': add_token(TokenType::RIGHT_PAREN,")"); break;
            case '[': add_token(TokenType::LEFT_BRACKET,"["); break;
            case ']': add_token(TokenType::RIGHT_BRACKET,"]"); break;
            case '{': add_token(TokenType::LEFT_BRACE,"{"); break;
            case '}': add_token(TokenType::RIGHT_BRACE,"}"); break;
            case ',': add_token(TokenType::COMMA,","); break;
            case '.': add_token(TokenType::DOT,"."); break;
            case ';': add_token(TokenType::SEMICOLON,";"); break;

            // Operators
            case '+': if (match('=')) add_token(TokenType::PLUS_ASSIGN,"+=");
                    else if (match('+')) add_token(TokenType::INCREMENT,"++");
                    else add_token(TokenType::PLUS,"+");
                    break;
            case '-': if(match('=')) add_token(TokenType::MINUS_ASSIGN,"-=");
                	else if (match('-')) add_token(TokenType::DECREMENT,"--");
					else add_token(TokenType::MINUS,"-");
                    break;
            case '*': if(match('=')) add_token(TokenType::MULTIPLY_ASSIGN,"*=");
                    // TODO: Pointer
                	else add_token(TokenType::MULTIPLY,"*");
                break;
            case '/': if (match('=')) add_token(TokenType::DIVIDE_ASSIGN, " /= ");
                    else if (match('/')) skip_comment();
					else add_token(TokenType::DIVIDE, "/");
                break;
            case '%': add_token(match('=') ? std::tuple(TokenType::MODULO_ASSIGN,"%=") : std::tuple(TokenType::MODULO, "%")); break;
            case '&': add_token(match('&') ? std::tuple(TokenType::LOGICAL_AND,"&&") : std::tuple(TokenType::BITWISE_AND,"&")); break;
            case '|': add_token(match('|') ? std::tuple(TokenType::LOGICAL_OR,"||") : std::tuple(TokenType::BITWISE_OR,"|")); break;
            case '^': add_token(TokenType::BITWISE_XOR,"^"); break;
            case '~': add_token(TokenType::BITWISE_NOT,"~"); break;
            case '?': add_token(TokenType::TERNARY,"?"); break;
            case ':': add_token(TokenType::COLON,":"); break;
            case '=': add_token(match('=') ? std::tuple(TokenType::EQUAL,"==") : std::tuple(TokenType::ASSIGN,"=")); break;
            case '!': add_token(match('=') ? std::tuple(TokenType::NOT_EQUAL,"!=") : std::tuple(TokenType::NOT,"!")); break;
            case '<': add_token(match('=') ? std::tuple(TokenType::LESS_THAN_OR_EQUAL_TO,"<=") : std::tuple(TokenType::LESS_THAN,"<")); break;
            case '>': add_token(match('=') ? std::tuple(TokenType::GREATER_THAN_OR_EQUAL_TO,">=") : std::tuple(TokenType::GREATER_THAN,">")); break;

            // Whitespace
            case ' ':
            case '\t':
            case '\r':
                break;

            // Newline
            case '\n':
                line_++;
                column_ = 1;
                break;

            // String literals
            case '"': string_literal(); break;

            // Default case
            default:
                if (is_digit(c)) {
                    number_literal();
                } else if (is_alpha(c)) {
                    identifier();
                } else {
                    std::cerr << "Unexpected character: " << c << " at line " << line_ << ", column " << column_ << "\n";
                }
                break;
        }
    }

    tokens_.push_back({ TokenType::END_OF_FILE, "EOF", line_, column_ });
    return tokens_;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
}

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end() || source_[current_] != expected) {
        return false;
    }

    current_++;
    column_++;
    return true;
}

void Lexer::add_token(std::tuple<TokenType,std::string> TokenTuple) {
    add_token(std::get<0>(TokenTuple),std::get<1>(TokenTuple));
}

void Lexer::add_token(TokenType type, const std::string& lexeme) {
    // TODO: 太长的StringLiteral导致line_错误, column_为负数
    tokens_.push_back({ type, lexeme, line_, column_ - lexeme.size() + 1 });
}
// void Lexer::add_token(TokenType type, double value){
//     tokens_.push_back({ type, value, line_, column_ - lexeme.size() });
// }
// void Lexer::add_token(TokenType type, int value){
//     tokens_.push_back({ type,  value, line_, column_ - lexeme.size() });
// }
// void Lexer::add_token(TokenType type, const std::string& lexeme, double value){
//     tokens_.push_back({ type, lexeme, value, line_, column_ - lexeme.size() });
// }
// void Lexer::add_token(TokenType type, const std::string& lexeme, int value){
//     tokens_.push_back({ type, lexeme, value, line_, column_ - lexeme.size() });
// }
// void Lexer::add_token(TokenType type, const std::string& lexeme, char value){
//     tokens_.push_back({ type, lexeme, value, line_, column_ - lexeme.size() });
// }
// void Lexer::add_token(TokenType type, const std::string& lexeme, const std::string& value){
//     tokens_.push_back({ type, lexeme, value, line_, column_ - lexeme.size() });
// }


bool Lexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

void Lexer::number_literal() {
    int start = current_ - 1;
    TokenType type = TokenType::CONSTANT;
    while (is_digit(peek())) {
        advance();
    }

    if (peek() == '.' && is_digit(peek_next())) {
        type = TokenType::CONSTANT;
        advance();

        while (is_digit(peek())) {
            advance();
        }
    }

    add_token(type, source_.substr(start, current_ - start));
}

void Lexer::identifier() {
    int start = current_ - 1;
    while (is_alpha(peek()) || is_digit(peek())) {
        advance();
    }
    std::string lexeme = source_.substr(start, current_ - start);
    TokenType type = TokenType::IDENTIFIER;
    if (lexeme == "sizeof") {
        type = TokenType::SIZEOF;
    }
    else if (lexeme == "if") {
        type = TokenType::IF;
    } 
    else if (lexeme == "else") {
        type = TokenType::ELSE;
    } 
    else if (lexeme == "while") {
        type = TokenType::WHILE;
    } 
    else if (lexeme == "for") {
        type = TokenType::FOR;
    } 
    else if (lexeme == "return") {
        type = TokenType::RETURN;
    } 
    else if (lexeme == "int") {
        type = TokenType::INTEGER;
    }
    else if (lexeme == "float") {
        type = TokenType::FLOAT;
    }
    else if (lexeme == "double") {
        type = TokenType::DOUBLE;
	}
	else if (lexeme == "char") {
		type = TokenType::CHARACTER;
	}
	else if (lexeme == "string") {
		type = TokenType::STRING;
	}
	else if (lexeme == "bool") {
		type = TokenType::BOOLEAN;
	}
    add_token(type, lexeme);
}

void Lexer::string_literal() {
    int start = current_ - 1;
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }

        advance();
    }

    if (is_at_end()) {
        std::cerr << "Unterminated string literal at line " << line_ << ", column " << column_ << "\n";
        return;
    }

    advance();
    add_token(TokenType::STRING, source_.substr(start + 1, current_ - 1 - start));
}

char Lexer::peek() const {
    return is_at_end() ? '\0' : source_[current_];
}

char Lexer::peek_next() const {
    return current_ + 1 >= source_.size() ? '\0' : source_[current_ + 1];
}

void Lexer::skip_whitespace(){
    while(peek() == ' ' || peek() == '\t' || peek() == '\r' || peek() == '\n'){
        advance();
    }
}
void Lexer::skip_comment(){
    while(peek() != '\n' && !is_at_end()){
        advance();
    }
}

void Lexer::error(const std::string& message) {
    std::cerr << message << "\n";
    exit(1);
}