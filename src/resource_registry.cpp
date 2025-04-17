#include "resource_registry.h"
#include <sstream>
#include <stdexcept>

namespace resource {

ResourceRegistry::ResourceRegistry() {}

bool ResourceRegistry::registerRootNode(std::shared_ptr<ResourceNode> root) {
    if (!root) {
        throw std::invalid_argument("Cannot register null root node");
        return false;
    }
    
    if (rootNodes_.find(root->getId()) != rootNodes_.end()) {
        throw std::invalid_argument("Root node with ID " + root->getId() + " already registered");
        return false;
    }
    
    rootNodes_[root->getId()] = root;
    return true;
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

bool ResourceRegistry::updateNode(std::shared_ptr<ResourceNode> node, 
    const void* objPtr,
    std::shared_ptr<const StructConverter> converter) {
    if (!node || !objPtr || !converter) return false;

    // 创建临时节点以获取最新属性
    auto tempNode = converter->convert(objPtr, node->getName());
    if (!tempNode) return false;

    // 更新当前节点的属性
    updateNodeAttributes(node, tempNode);

    return true;
}

bool ResourceRegistry::removeDynamicObject(std::shared_ptr<ResourceNode> node) {
    auto it = std::find_if(dynamicObjects_.begin(), dynamicObjects_.end(),
        [&node](const auto& item) {
            return std::get<3>(item) == node;
        });
    
    if (it != dynamicObjects_.end()) {
        dynamicObjects_.erase(it);
        return true;
    }
    return false;
}

void ResourceRegistry::updateNodeAttributes(std::shared_ptr<ResourceNode> target, 
                            std::shared_ptr<ResourceNode> source) {
    // 1. 更新所有属性
    auto& sourceAttrs = source->getAttributes();
    for (const auto& attr : sourceAttrs) {
        target->updateAttributeRaw(std::get<0>(attr), std::get<1>(attr)->clone());  // key, value
    }
    
    // 2. 处理子节点
    const auto& sourceChildren = source->getChildren();
    const auto& targetChildren = target->getChildren();
    
    // 查找匹配的子节点并更新
    for (const auto& sourceChild : sourceChildren) {
        bool found = false;
        for (const auto& targetChild : targetChildren) {
            if (targetChild->getId() == sourceChild->getId()) {
                // 递归更新子节点
                updateNodeAttributes(targetChild, sourceChild);
                found = true;
                break;
            }
        }
        
        // 如果没找到匹配的子节点，添加新节点
        if (!found) {
            target->addChild(sourceChild->clone());
        }
    }
    
    // 3. 删除已不存在的子节点
    std::vector<std::string> childrenToRemove;
    for (const auto& targetChild : targetChildren) {
        bool found = false;
        for (const auto& sourceChild : sourceChildren) {
            if (targetChild->getId() == sourceChild->getId()) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            childrenToRemove.push_back(targetChild->getId());
        }
    }
    
    for (const auto& childId : childrenToRemove) {
        target->removeChild(childId);
    }
}

void ResourceRegistry::clear() {
    rootNodes_.clear();
}

} // namespace resource