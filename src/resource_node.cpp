#include "resource_node.h"
#include <algorithm>
#include <stdexcept>

namespace resource {

void ResourceNode::addChild(std::shared_ptr<ResourceNode> child) {
    if (!child) {
        throw std::invalid_argument("Cannot add null child");
    }
    
    // 检查是否存在相同ID的子节点
    if (childMap_.find(child->getId()) != childMap_.end()) {
        throw std::invalid_argument("Child with ID " + child->getId() + " already exists");
    }
    
    children_.push_back(child);
    childMap_[child->getId()] = child;
}

void ResourceNode::removeChild(const std::string& id) {
    auto it = childMap_.find(id);
    if (it == childMap_.end()) {
        return; // 节点不存在，直接返回
    }
    
    // 从vector中移除
    auto vecIt = std::find_if(children_.begin(), children_.end(),
        [&id](const std::shared_ptr<ResourceNode>& node) {
            return node->getId() == id;
        });
        
    if (vecIt != children_.end()) {
        children_.erase(vecIt);
    }
    
    // 从map中移除
    childMap_.erase(it);
}

// 添加克隆方法
std::shared_ptr<ResourceNode> ResourceNode::clone() const {
    auto copy = std::make_shared<ResourceNode>(name_, id_);
    
    // 复制属性
    for (const auto& attr : attributes_) {
        copy->attributes_[attr.first] = attr.second->clone();
    }
    
    // 递归复制子节点
    for (const auto& child : children_) {
        copy->addChild(child->clone());
    }
    
    return copy;
}

void ResourceNode::traverse(const std::function<void(const std::shared_ptr<ResourceNode>&, int depth)>& visitor, int depth) const {
    // 使用const_pointer_cast，因为shared_from_this()会返回非const指针
    // 但我们需要保持const正确性
    std::shared_ptr<ResourceNode> thisPtr = 
        std::const_pointer_cast<ResourceNode>(shared_from_this());
    
    // 访问当前节点
    visitor(thisPtr, depth);
    
    // 递归访问所有子节点
    for (const auto& child : children_) {
        child->traverse(visitor, depth + 1);
    }
}

void simple_visitor(const std::shared_ptr<ResourceNode>& node, int depth) {
    std::string indent(depth * 2, ' ');
    std::cout << indent << "- " << node->getName() << " (ID: " << node->getId() << ")" << std::endl;
    
    // 打印节点属性
    auto keys = node->getAttributeKeys();
    if (!keys.empty()) {
        std::string attrIndent(depth * 2 + 2, ' ');
        std::cout << attrIndent << "attr:" << std::endl;
        for (const auto& key : keys) {
            std::cout << attrIndent << "  " << key << ": ";
            
            // 尝试输出常见类型的属性值
            try {
                if (node->getAttributeType(key) == typeid(int)) {
                    std::cout << node->getAttribute<int>(key);
                }
                else if (node->getAttributeType(key) == typeid(double)) {
                    std::cout << node->getAttribute<double>(key);
                }
                else if (node->getAttributeType(key) == typeid(std::string)) {
                    std::cout << node->getAttribute<std::string>(key);
                }
                else if (node->getAttributeType(key) == typeid(bool)) {
                    std::cout << (node->getAttribute<bool>(key) ? "true" : "false");
                }
                else {
                    std::cout << "[complex type]";
                }
            } catch (...) {
                std::cout << "[error:can't read attribute]";
            }
            std::cout << std::endl;
        }
    }
}

} // namespace resource