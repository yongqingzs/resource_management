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
    
    // 高级查询功能
    std::vector<std::shared_ptr<ResourceNode>> findNodes(
        const std::function<bool(const std::shared_ptr<ResourceNode>&)>& predicate) const;
    
    // 通过属性查询节点 - 模板函数
    template<typename T>
    std::vector<std::shared_ptr<ResourceNode>> findNodesByAttribute(
        const std::string& attrName, const T& attrValue) const {
        
        return findNodes([&](const std::shared_ptr<ResourceNode>& node) -> bool {
            if (!node->hasAttribute(attrName)) return false;
            
            try {
                return node->getAttribute<T>(attrName) == attrValue;
            } catch (const std::bad_cast&) {
                return false;
            } catch (const std::runtime_error&) {
                return false;
            }
        });
    }
    
    void traverse(const std::function<void(const std::shared_ptr<ResourceNode>&, int depth)>& visitor) const;
    
    // 清空注册表
    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> rootNodes_;
    
    std::vector<std::string> splitPath(const std::string& path) const;
    
    void collectNodes(
        std::shared_ptr<ResourceNode> node,
        const std::function<bool(const std::shared_ptr<ResourceNode>&)>& predicate,
        std::vector<std::shared_ptr<ResourceNode>>& results) const;
};

} // namespace resource