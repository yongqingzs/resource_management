#pragma once

#include "resource_registry.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <typeindex>
#include <memory>
#include <type_traits>
#include <sstream>

namespace resource {

class ResourceIndexer {
public:
    explicit ResourceIndexer(ResourceRegistry& registry);
    
    // 原有的方法保持不变
    std::vector<std::shared_ptr<ResourceNode>> findByName(const std::string& name);
    std::vector<std::shared_ptr<ResourceNode>> findById(const std::string& id);
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
    
    // 原有的索引维护
    void refreshIndex();
    
    // === 新增属性索引功能 ===
    
    // 为特定属性创建索引
    template<typename T>
    void createAttributeIndex(const std::string& attrName) {
        // 生成属性索引键
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        
        // 创建或清空索引
        attributeIndices_[indexKey].clear();
        
        // 遍历所有节点，构建索引
        registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
            if (node->hasAttribute(attrName)) {
                try {
                    T value = node->getAttribute<T>(attrName);
                    std::string valueKey = serializeValue(value);
                    attributeIndices_[indexKey][valueKey].push_back(node);
                } catch (...) {
                    // 忽略类型不匹配的属性
                }
            }
        });
        
        // 记录已创建的索引
        indexedAttributes_.insert(indexKey);
    }
    
    // 使用索引查询属性
    template<typename T>
    std::vector<std::shared_ptr<ResourceNode>> findByAttributeIndexed(const std::string& attrName, const T& value) {
        // 生成属性索引键
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        
        // 检查是否有此属性的索引
        if (indexedAttributes_.count(indexKey) == 0) {
            // 没有索引，创建索引
            createAttributeIndex<T>(attrName);
        }
        
        // 序列化值用于查找
        std::string valueKey = serializeValue(value);
        
        // 查找索引
        auto& indexMap = attributeIndices_[indexKey];
        auto it = indexMap.find(valueKey);
        if (it != indexMap.end()) {
            return it->second;
        }
        
        return std::vector<std::shared_ptr<ResourceNode>>();
    }
    
    // 检查属性索引是否存在
    template<typename T>
    bool hasAttributeIndex(const std::string& attrName) {
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        return indexedAttributes_.count(indexKey) > 0;
    }
    
    // 删除属性索引
    template<typename T>
    void removeAttributeIndex(const std::string& attrName) {
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        attributeIndices_.erase(indexKey);
        indexedAttributes_.erase(indexKey);
    }
    
private:
    ResourceRegistry& registry_;
    
    // 基本索引
    std::unordered_map<std::string, std::vector<std::shared_ptr<ResourceNode>>> nameIndex_;
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> idIndex_;
    
    // 属性索引: attribute_type:attribute_name -> value -> nodes
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<ResourceNode>>>> attributeIndices_;
    std::unordered_set<std::string> indexedAttributes_;
    
    void buildIndices();
    
    // 获取属性索引键
    std::string getAttributeIndexKey(const std::string& attrName, const std::type_info& type) {
        return std::string(type.name()) + ":" + attrName;
    }
    
    // 序列化值为字符串（用于索引键）- C++11 版本
    // 标准字符串类型
    template<typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type
    serializeValue(const T& value) {
        return value;
    }
    
    // 算术类型 (int, double 等)
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, std::string>::type
    serializeValue(const T& value) {
        return std::to_string(value);
    }
    
    // 布尔类型
    template<typename T>
    typename std::enable_if<std::is_same<T, bool>::value, std::string>::type
    serializeValue(const T& value) {
        return value ? "true" : "false";
    }
    
    // 其他类型
    template<typename T>
    typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_same<T, std::string>::value, std::string>::type
    serializeValue(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
};

} // namespace resource