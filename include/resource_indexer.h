#pragma once

#include "resource_registry.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <typeindex>
#include <memory>
#include <type_traits>
#include <sstream>
#include <algorithm>

namespace resource {

class ResourceIndexer {
public:
    explicit ResourceIndexer(ResourceRegistry& registry);
    ~ResourceIndexer();
    
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
    
    // === 属性索引功能，现在使用有序索引 ===
    
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
                    
                    // 使用模板特化将T类型转换为索引键类型
                    IndexKey indexValue = convertToIndexKey<T>(value);
                    attributeIndices_[indexKey][indexValue].push_back(node);
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
        
        // 转换值为索引键
        IndexKey indexValue = convertToIndexKey<T>(value);
        
        // 查找索引
        auto& indexMap = attributeIndices_[indexKey];
        auto it = indexMap.find(indexValue);
        if (it != indexMap.end()) {
            return it->second;
        }
        
        return std::vector<std::shared_ptr<ResourceNode>>();
    }
    
    // 大于查询
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, std::vector<std::shared_ptr<ResourceNode>>>::type
    findGreaterThan(const std::string& attrName, const T& value) {
        // 生成属性索引键
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        
        // 检查是否有此属性的索引
        if (indexedAttributes_.count(indexKey) == 0) {
            // 没有索引，创建索引
            createAttributeIndex<T>(attrName);
        }
        
        // 转换值为索引键
        IndexKey indexValue = convertToIndexKey<T>(value);
        
        std::vector<std::shared_ptr<ResourceNode>> results;
        auto& indexMap = attributeIndices_[indexKey];
        
        // 找到第一个大于value的位置
        auto it = indexMap.upper_bound(indexValue);
        
        // 收集所有大于value的节点
        while (it != indexMap.end()) {
            results.insert(results.end(), it->second.begin(), it->second.end());
            ++it;
        }
        
        return results;
    }
    
    // 小于查询
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, std::vector<std::shared_ptr<ResourceNode>>>::type
    findLessThan(const std::string& attrName, const T& value) {
        // 生成属性索引键
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        
        // 检查是否有此属性的索引
        if (indexedAttributes_.count(indexKey) == 0) {
            // 没有索引，创建索引
            createAttributeIndex<T>(attrName);
        }
        
        // 转换值为索引键
        IndexKey indexValue = convertToIndexKey<T>(value);
        
        std::vector<std::shared_ptr<ResourceNode>> results;
        auto& indexMap = attributeIndices_[indexKey];
        
        // 收集所有小于value的节点
        for (auto it = indexMap.begin(); it != indexMap.end() && it->first < indexValue; ++it) {
            results.insert(results.end(), it->second.begin(), it->second.end());
        }
        
        return results;
    }
    
    // 范围查询
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, std::vector<std::shared_ptr<ResourceNode>>>::type
    findInRange(const std::string& attrName, const T& minValue, const T& maxValue) {
        // 生成属性索引键
        std::string indexKey = getAttributeIndexKey(attrName, typeid(T));
        
        // 检查是否有此属性的索引
        if (indexedAttributes_.count(indexKey) == 0) {
            // 没有索引，创建索引
            createAttributeIndex<T>(attrName);
        }
        
        // 转换值为索引键
        IndexKey minIndexValue = convertToIndexKey<T>(minValue);
        IndexKey maxIndexValue = convertToIndexKey<T>(maxValue);
        
        std::vector<std::shared_ptr<ResourceNode>> results;
        auto& indexMap = attributeIndices_[indexKey];
        
        // 找到第一个大于等于minValue的位置
        auto it = indexMap.lower_bound(minIndexValue);
        
        // 收集所有在[minValue, maxValue]范围内的节点
        while (it != indexMap.end() && it->first <= maxIndexValue) {
            results.insert(results.end(), it->second.begin(), it->second.end());
            ++it;
        }
        
        return results;
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
    
    // 自定义索引键类型，支持排序
    class IndexKey {
    public:
        enum class Type { STRING, DOUBLE, BOOL };
        
        // 构造函数
        explicit IndexKey(const std::string& s) : type_(Type::STRING), stringValue_(s), doubleValue_(0.0), boolValue_(false) {}
        explicit IndexKey(double d) : type_(Type::DOUBLE), stringValue_(""), doubleValue_(d), boolValue_(false) {}
        explicit IndexKey(bool b) : type_(Type::BOOL), stringValue_(""), doubleValue_(0.0), boolValue_(b) {}
        
        // 比较运算符
        bool operator<(const IndexKey& other) const {
            if (type_ != other.type_) {
                return static_cast<int>(type_) < static_cast<int>(other.type_);
            }
            
            switch (type_) {
                case Type::STRING: return stringValue_ < other.stringValue_;
                case Type::DOUBLE: return doubleValue_ < other.doubleValue_;
                case Type::BOOL: return !boolValue_ && other.boolValue_; // false < true
                default: return false;
            }
        }
        
        bool operator==(const IndexKey& other) const {
            if (type_ != other.type_) return false;
            
            switch (type_) {
                case Type::STRING: return stringValue_ == other.stringValue_;
                case Type::DOUBLE: return doubleValue_ == other.doubleValue_;
                case Type::BOOL: return boolValue_ == other.boolValue_;
                default: return false;
            }
        }

        bool operator<=(const IndexKey& other) const {
            return *this < other || *this == other;
        }
        
    private:
        Type type_;
        std::string stringValue_;
        double doubleValue_;
        bool boolValue_;
    };
    
    // 属性索引: attribute_type:attribute_name -> value -> nodes
    // 使用有序映射来支持范围查询
    std::unordered_map<std::string, std::map<IndexKey, std::vector<std::shared_ptr<ResourceNode>>>> attributeIndices_;
    std::unordered_set<std::string> indexedAttributes_;
    
    void buildIndices();
    void rebuildAttributeIndex(const std::string& typeName, const std::string& attrName);
    
    // 获取属性索引键
    std::string getAttributeIndexKey(const std::string& attrName, const std::type_info& type) {
        return std::string(type.name()) + ":" + attrName;
    }
    
    // 将各种类型转换为IndexKey
    template<typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, IndexKey>::type
    convertToIndexKey(const T& value) {
        return IndexKey(value);
    }
    
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, IndexKey>::type
    convertToIndexKey(const T& value) {
        return IndexKey(static_cast<double>(value));
    }
    
    template<typename T>
    typename std::enable_if<std::is_same<T, bool>::value, IndexKey>::type
    convertToIndexKey(const T& value) {
        return IndexKey(value);
    }
};

} // namespace resource