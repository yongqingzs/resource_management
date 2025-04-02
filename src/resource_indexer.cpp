#include "../include/resource_indexer.h"

namespace resource_management {

ResourceIndexer::ResourceIndexer(ResourceRegistry& registry)
    : registry_(registry) {
    refreshIndex();
}

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
    return registry_.findNodes(predicate);
}

std::vector<std::shared_ptr<ResourceNode>> ResourceIndexer::findByMultiConditions(
    const std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>>& conditions,
    bool matchAll) {
    
    if (conditions.empty()) {
        return std::vector<std::shared_ptr<ResourceNode>>();
    }
    
    return registry_.findNodes([&](const std::shared_ptr<ResourceNode>& node) -> bool {
        if (matchAll) {
            // 所有条件必须满足
            for (const auto& condition : conditions) {
                if (!condition(node)) {
                    return false;
                }
            }
            return true;
        } else {
            // 至少一个条件满足
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
    buildIndices();
}

void ResourceIndexer::buildIndices() {
    // 清空现有索引
    nameIndex_.clear();
    idIndex_.clear();
    
    // 获取所有根节点
    auto rootNodes = registry_.getAllRootNodes();
    
    // 遍历整个树构建索引
    for (const auto& rootNode : rootNodes) {
        rootNode->traverse([this](const std::shared_ptr<ResourceNode>& node, int) {
            nameIndex_[node->getName()].push_back(node);
            idIndex_[node->getId()] = node;
        });
    }
}

} // namespace resource_management