#pragma once

#include "resource_node.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <tuple>

namespace resource {

// 通用的结构体转换器接口
class StructConverter {
public:
    virtual ~StructConverter() = default;
    virtual std::shared_ptr<ResourceNode> convert(const void* structPtr, const std::string& nodeName) const = 0;
    virtual StructConverter* clone() const = 0;  // 添加克隆方法
};

class ResourceRegistry {
public:
    ResourceRegistry();
    
    // 根节点管理
    bool registerRootNode(std::shared_ptr<ResourceNode> root);

    void unregisterRootNode(const std::string& rootId) { rootNodes_.erase(rootId);}

    std::shared_ptr<ResourceNode> getRootNode(const std::string& rootId) const {
        auto it = rootNodes_.find(rootId);
        if (it != rootNodes_.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<ResourceNode>> getAllRootNodes() const {
        std::vector<std::shared_ptr<ResourceNode>> result;
        result.reserve(rootNodes_.size());
        for (const auto& pair : rootNodes_) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
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

    // 通用结构体注册方法
    template<typename T>
    bool registerStruct(const T& obj, const std::string& path, const StructConverter& converter, const std::string& nodeName = "") {
        auto node = converter.convert(&obj, nodeName.empty() ? typeid(T).name() : nodeName);
        if (!node) return false;
        
        if (path.empty()) {
            return registerRootNode(node);
        } else {
            return registerNodeAtPath(path, node);
        }
    }

    // 批量注册
    template<typename T>
    bool registerStructs(const std::vector<T>& objects, const std::string& basePath, 
                         const StructConverter& converter, 
                         const std::function<std::string(const T&)>& pathGenerator) {
        bool success = true;
        for (const auto& obj : objects) {
            std::string path = basePath;
            if (pathGenerator) {
                std::string subPath = pathGenerator(obj);
                if (!subPath.empty()) {
                    path = path.empty() ? subPath : path + "/" + subPath;
                }
            }
            success &= registerStruct(obj, path, converter);
        }
        return success;
    }

    template<typename T>
    std::shared_ptr<ResourceNode> registerDynamicStruct(
        T& obj, 
        const std::string& path, 
        const StructConverter& converter,
        const std::string& nodeName = "") 
    {
        auto node = converter.convert(&obj, nodeName.empty() ? typeid(T).name() : nodeName);
        if (!node) return nullptr;
        
        // 存储对象引用和转换器以便后续更新
        dynamicObjects_.push_back(std::make_tuple(
            static_cast<const void*>(&obj), 
            std::type_index(typeid(T)), 
            std::shared_ptr<const StructConverter>(converter.clone()),
            node));
        
        if (path.empty()) {
            registerRootNode(node);
        } else {
            registerNodeAtPath(path, node);
        }
        
        return node;
    }
    
    // 更新所有动态对象
    void updateAllDynamicObjects() {
        for (const auto& obj : dynamicObjects_) {
            // obj: [objPtr, typeIdx, converter, node]
            // updateNode(node, objPtr, converter);
            updateNode(std::get<3>(obj), std::get<0>(obj), std::get<2>(obj));
        }
    }
    
    // 更新特定节点
    bool updateNode(std::shared_ptr<ResourceNode> node, 
                    const void* objPtr,
                    std::shared_ptr<const StructConverter> converter);

    // 清除所有动态对象跟踪
    void clearDynamicObjects() { dynamicObjects_.clear(); }

    // 移除特定节点的动态跟踪
    bool removeDynamicObject(std::shared_ptr<ResourceNode> node);

    // 清空注册表
    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> rootNodes_;
    
    std::vector<std::string> splitPath(const std::string& path) const;

    // 存储动态对象引用、类型信息、转换器和对应节点
    std::vector<std::tuple<const void*, std::type_index, 
                            std::shared_ptr<const StructConverter>, 
                            std::shared_ptr<ResourceNode>>> dynamicObjects_;
                            
    // 递归更新节点属性
    void updateNodeAttributes(std::shared_ptr<ResourceNode> target, 
                             std::shared_ptr<ResourceNode> source);
};

} // namespace resource