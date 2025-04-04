#include "resource_registry.h"
#include <sstream>
#include <stdexcept>

namespace resource {

ResourceRegistry::ResourceRegistry() {}

void ResourceRegistry::registerRootNode(std::shared_ptr<ResourceNode> root) {
    if (!root) {
        throw std::invalid_argument("Cannot register null root node");
    }
    
    if (rootNodes_.find(root->getId()) != rootNodes_.end()) {
        throw std::invalid_argument("Root node with ID " + root->getId() + " already registered");
    }
    
    rootNodes_[root->getId()] = root;
}

void ResourceRegistry::unregisterRootNode(const std::string& rootId) {
    rootNodes_.erase(rootId);
}

std::shared_ptr<ResourceNode> ResourceRegistry::getRootNode(const std::string& rootId) const {
    auto it = rootNodes_.find(rootId);
    if (it != rootNodes_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ResourceNode>> ResourceRegistry::getAllRootNodes() const {
    std::vector<std::shared_ptr<ResourceNode>> result;
    result.reserve(rootNodes_.size());
    
    for (const auto& pair : rootNodes_) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<std::string> ResourceRegistry::splitPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

std::shared_ptr<ResourceNode> ResourceRegistry::getNodeByPath(const std::string& path) const {
    auto parts = splitPath(path);
    if (parts.empty()) {
        return nullptr;
    }
    
    // 找到根节点
    auto currentNode = getRootNode(parts[0]);
    if (!currentNode) {
        // std::cout << "getNodeByPath: can't find path!" << std::endl;
        return nullptr;
    }
    
    // 遍历路径
    for (size_t i = 1; i < parts.size(); ++i) {
        currentNode = currentNode->getChild(parts[i]);
        if (!currentNode) {
            // std::cout << "getNodeByPath: can't find path!" << std::endl;
            return nullptr;
        }
    }
    
    return currentNode;
}

bool ResourceRegistry::registerNodeAtPath(const std::string& path, std::shared_ptr<ResourceNode> node) {
    if (!node) {
        return false;
    }
    
    auto parts = splitPath(path);
    if (parts.empty()) {
        // 尝试作为根节点注册
        try {
            registerRootNode(node);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    // 查找父节点
    std::string parentPath;
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (i > 0) parentPath += "/";
        parentPath += parts[i];
    }
    
    auto parentNode = getNodeByPath(parentPath);
    if (!parentNode) {
        return false;
    }
    
    // 添加节点
    try {
        parentNode->addChild(node);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ResourceRegistry::removeNodeByPath(const std::string& path) {
    auto parts = splitPath(path);
    if (parts.empty()) {
        return false;
    }
    
    if (parts.size() == 1) {
        // 移除根节点
        auto it = rootNodes_.find(parts[0]);
        if (it != rootNodes_.end()) {
            rootNodes_.erase(it);
            return true;
        }
        return false;
    }
    
    // 查找父节点
    std::string parentPath;
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (i > 0) parentPath += "/";
        parentPath += parts[i];
    }
    
    auto parentNode = getNodeByPath(parentPath);
    if (!parentNode) {
        return false;
    }
    
    // 移除子节点
    parentNode->removeChild(parts.back());
    return true;
}

std::shared_ptr<ResourceNode> ResourceRegistry::createPath(const std::string& path) {
    auto parts = splitPath(path);
    if (parts.empty()) {
        return nullptr;
    }
    
    // 检查根节点是否存在，不存在则创建
    auto currentNode = getRootNode(parts[0]);
    if (!currentNode) {
        currentNode = std::make_shared<ResourceNode>(parts[0], parts[0]);
        registerRootNode(currentNode);
    }
    
    // 创建路径上的所有节点
    for (size_t i = 1; i < parts.size(); ++i) {
        auto childNode = currentNode->getChild(parts[i]);
        if (!childNode) {
            childNode = std::make_shared<ResourceNode>(parts[i], parts[i]);
            currentNode->addChild(childNode);
        }
        currentNode = childNode;
    }
    
    return currentNode;
}

void ResourceRegistry::traverseRootNode(const std::function<void(const std::shared_ptr<ResourceNode>&, int depth)>& visitor) const {
    // 首先访问每个根节点
    for (const auto& pair : rootNodes_) {
        // 从深度0开始递归遍历每个根节点的子树
        pair.second->traverse(visitor);
    }
}

void ResourceRegistry::traverseNodes(const std::function<void(std::shared_ptr<ResourceNode>)>& callback) const {
    // 定义内部递归函数
    std::function<void(std::shared_ptr<ResourceNode>)> traverse = 
    [&callback, &traverse](std::shared_ptr<ResourceNode> node) {
        // 处理当前节点
        callback(node);
        
        // 递归处理所有子节点
        for (const auto& child : node->getChildren()) {
            traverse(child);
        }
    };
    
    // 从所有根节点开始遍历
    for (const auto& pair : rootNodes_) {
        traverse(pair.second);
    }
}

void ResourceRegistry::clear() {
    rootNodes_.clear();
}

} // namespace resource