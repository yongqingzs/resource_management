#pragma once

#include "resource_registry.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <string>

namespace resource_management {

class ResourceIndexer {
public:
    ResourceIndexer(ResourceRegistry& registry);
    
    // 基本查找功能
    std::vector<std::shared_ptr<ResourceNode>> findByName(const std::string& name);
    std::vector<std::shared_ptr<ResourceNode>> findById(const std::string& id);
    
    // 高级查询
    std::vector<std::shared_ptr<ResourceNode>> findByPredicate(
        const std::function<bool(const std::shared_ptr<ResourceNode>&)>& predicate);
    
    // 属性查询
    template<typename T>
    std::vector<std::shared_ptr<ResourceNode>> findByAttribute(const std::string& attrName, const T& value) {
        return registry_.findNodesByAttribute<T>(attrName, value);
    }
    
    // 多条件查询
    std::vector<std::shared_ptr<ResourceNode>> findByMultiConditions(
        const std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>>& conditions,
        bool matchAll = true);
    
    // 刷新索引缓存
    void refreshIndex();
    
private:
    ResourceRegistry& registry_;
    
    // 索引缓存
    std::unordered_map<std::string, std::vector<std::shared_ptr<ResourceNode>>> nameIndex_;
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> idIndex_;
    
    // 构建索引
    void buildIndices();
};

} // namespace resource_management