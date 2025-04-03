#pragma once

#include "resource_node.h"
#include <memory>
#include <unordered_map>
#include <functional>

namespace resource {

class ResourceRegistry {
public:
    ResourceRegistry();
    
    // 根节点管理
    void registerRootNode(std::shared_ptr<ResourceNode> root);
    void unregisterRootNode(const std::string& rootId);
    std::shared_ptr<ResourceNode> getRootNode(const std::string& rootId) const;
    std::vector<std::shared_ptr<ResourceNode>> getAllRootNodes() const;
    
    // 节点路径操作
    bool registerNodeAtPath(const std::string& path, std::shared_ptr<ResourceNode> node);
    std::shared_ptr<ResourceNode> getNodeByPath(const std::string& path) const;
    bool removeNodeByPath(const std::string& path);
    
    // 支持创建整个路径
    std::shared_ptr<ResourceNode> createPath(const std::string& path);
    
    // 遍历根节点
    void traverseRootNode(const std::function<void(const std::shared_ptr<ResourceNode>&, int depth)>& visitor) const;
    
    // 基础遍历功能 - 仅供内部和ResourceIndexer使用
    void traverseNodes(const std::function<void(std::shared_ptr<ResourceNode>)>& callback) const;

    // 清空注册表
    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> rootNodes_;
    
    std::vector<std::string> splitPath(const std::string& path) const;

};

} // namespace resource