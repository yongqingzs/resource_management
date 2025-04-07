#include "resource_indexer.h"
#include <algorithm>

namespace resource {

ResourceIndexer::ResourceIndexer(ResourceRegistry& registry)
    : registry_(registry) {
    refreshIndex();
}

ResourceIndexer::~ResourceIndexer() = default;

std::vector<std::shared_ptr<ResourceNode>> ResourceIndexer::findByName(const std::string& name) {
    auto it = nameIndex_.find(name);
    if (it != nameIndex_.end()) {
        return it->second;
    }
    return std::vector<std::shared_ptr<ResourceNode>>();
}

std::vector<std::shared_ptr<ResourceNode>> ResourceIndexer::findById(const std::string& id) {
    auto it = idIndex_.find(id);
    if (it != idIndex_.end()) {
        return {it->second};
    }
    return std::vector<std::shared_ptr<ResourceNode>>();
}

std::vector<std::shared_ptr<ResourceNode>> ResourceIndexer::findByPredicate(
    const std::function<bool(const std::shared_ptr<ResourceNode>&)>& predicate) {
    std::vector<std::shared_ptr<ResourceNode>> results;

    // 通过注册表的遍历函数收集符合条件的节点
    registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
        if (predicate(node)) {
            results.push_back(node);
        }
    });
    
    return results;
}

std::vector<std::shared_ptr<ResourceNode>> ResourceIndexer::findByMultiConditions(
    const std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>>& conditions,
    bool matchAll) {
    
    if (conditions.empty()) {
        return std::vector<std::shared_ptr<ResourceNode>>();
    }

    return findByPredicate([&](const std::shared_ptr<ResourceNode>& node) -> bool {
        if (matchAll) {
            // 所有条件都必须满足（AND）
            for (const auto& condition : conditions) {
                if (!condition(node)) {
                    return false;
                }
            }
            return true;
        } else {
            // 满足任一条件即可（OR）
            for (const auto& condition : conditions) {
                if (condition(node)) {
                    return true;
                }
            }
            return false;
        }
    });
}

void ResourceIndexer::refreshIndex() {
    // 构建基本索引
    buildIndices();
    
    // 保存要重建的索引键
    std::vector<std::string> indexKeys(indexedAttributes_.begin(), indexedAttributes_.end());
    
    // 清空属性索引
    attributeIndices_.clear();
    indexedAttributes_.clear();
    
    // 重新构建所有属性索引
    for (const auto& indexKey : indexKeys) {
        // 解析索引键以获取属性名和类型
        size_t separatorPos = indexKey.find(':');
        if (separatorPos != std::string::npos) {
            std::string typeName = indexKey.substr(0, separatorPos);
            std::string attrName = indexKey.substr(separatorPos + 1);
            
            // 根据类型重新构建索引
            rebuildAttributeIndex(typeName, attrName);
        }
    }
}

void ResourceIndexer::buildIndices() {
    // 清空现有索引
    nameIndex_.clear();
    idIndex_.clear();
    
    // 遍历所有节点构建索引
    registry_.traverseNodes([this](std::shared_ptr<ResourceNode> node) {
        // 按名称索引
        nameIndex_[node->getName()].push_back(node);
        
        // 按ID索引
        idIndex_[node->getId()] = node;
    });
}

void ResourceIndexer::rebuildAttributeIndex(const std::string& typeName, const std::string& attrName) {
    std::string indexKey = typeName + ":" + attrName;
    
    // 初始化索引
    attributeIndices_[indexKey].clear();
    indexedAttributes_.insert(indexKey);
    
    // 根据类型重建索引
    if (typeName == typeid(int).name()) {
        registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
            if (node->hasAttribute(attrName)) {
                try {
                    int value = node->getAttribute<int>(attrName);
                    attributeIndices_[indexKey][IndexKey(static_cast<double>(value))].push_back(node);
                } catch (...) {
                    // 忽略类型不匹配的属性
                }
            }
        });
    }
    else if (typeName == typeid(double).name()) {
        registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
            if (node->hasAttribute(attrName)) {
                try {
                    double value = node->getAttribute<double>(attrName);
                    attributeIndices_[indexKey][IndexKey(value)].push_back(node);
                } catch (...) {
                    // 忽略类型不匹配的属性
                }
            }
        });
    }
    else if (typeName == typeid(std::string).name()) {
        registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
            if (node->hasAttribute(attrName)) {
                try {
                    std::string value = node->getAttribute<std::string>(attrName);
                    attributeIndices_[indexKey][IndexKey(value)].push_back(node);
                } catch (...) {
                    // 忽略类型不匹配的属性
                }
            }
        });
    }
    else if (typeName == typeid(bool).name()) {
        registry_.traverseNodes([&](std::shared_ptr<ResourceNode> node) {
            if (node->hasAttribute(attrName)) {
                try {
                    bool value = node->getAttribute<bool>(attrName);
                    attributeIndices_[indexKey][IndexKey(value)].push_back(node);
                } catch (...) {
                    // 忽略类型不匹配的属性
                }
            }
        });
    }
    // 可以根据需要添加更多类型的处理
}

} // namespace resource