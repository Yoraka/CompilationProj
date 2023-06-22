#pragma once
#include <iostream>
#include <vector>
// AST节点的类定义
class ASTNode {
public:
    std::string type;  // 节点类型
    std::string value; // 节点值
    std::vector<ASTNode*> children; // 子节点列表

    ASTNode(const std::string& type, const std::string& value)
        : type(type), value(value) {
    }

    ~ASTNode() {
        for (auto child : children) {
            delete child;
        }
    }
};
