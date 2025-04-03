#pragma once

#include "resource_registry.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <string>

namespace resource {

class ResourceIndexer {
public:
    explicit ResourceIndexer(ResourceRegistry& registry);
    
    // 基本查询功能
    std::vector<std::shared_ptr<ResourceNode>> findByName(const std::string& name);
    std::vector<std::shared_ptr<ResourceNode>> findById(const std::string& id);
    
    // 高级查询功能
    std::vector<std::shared_ptr<ResourceNode>> findByPredicate(
        const std::function<bool(const std::shared_ptr<ResourceNode>&)>& predicate);
    
    template<typename T>
    std::vector<std::shared_ptr<ResourceNode>> findByAttribute(const std::string& attrName, const T& value) {
        return findByPredicate([&](const std::shared_ptr<ResourceNode>& node) -> bool {
            if (!node->hasAttribute(attrName)) return false;
            
            try {
                return node->getAttribute<T>(attrName) == value;
            } catch (...) {
                return false;
            }
        });
    }
    
    std::vector<std::shared_ptr<ResourceNode>> findByMultiConditions(
        const std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>>& conditions,
        bool matchAll = true);
    
    // 索引维护
    void refreshIndex();
    
private:
    ResourceRegistry& registry_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<ResourceNode>>> nameIndex_;
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> idIndex_;
    
    void buildIndices();
};

} // namespace resource