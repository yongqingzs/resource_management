#include "../include/resource_node.h"
#include <algorithm>
#include <stdexcept>

namespace resource_management {

ResourceNode::ResourceNode(const std::string& name, const std::string& id)
    : name_(name), id_(id) {}

ResourceNode::~ResourceNode() = default;

const std::string& ResourceNode::getName() const {
    return name_;
}

const std::string& ResourceNode::getId() const {
    return id_;
}

void ResourceNode::setName(const std::string& name) {
    name_ = name;
}

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

std::shared_ptr<ResourceNode> ResourceNode::getChild(const std::string& id) const {
    auto it = childMap_.find(id);
    if (it != childMap_.end()) {
        return it->second;
    }
    return nullptr;
}

const std::vector<std::shared_ptr<ResourceNode>>& ResourceNode::getChildren() const {
    return children_;
}

std::string ResourceNode::getType() const {
    return "ResourceNode";
}

std::shared_ptr<ResourceNode> ResourceNode::clone() const {
    auto node = std::make_shared<ResourceNode>(name_, id_);
    
    // 复制属性
    for (const auto& pair : attributes_) {
        node->attributes_[pair.first] = pair.second->clone();
    }
    
    return node;
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

} // namespace resource_management