#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeinfo>

namespace resource_management {

// 抽象的属性值基类，用于类型擦除
class AttributeValue {
public:
    virtual ~AttributeValue() {}
    virtual const std::type_info& getType() const = 0;
    virtual std::unique_ptr<AttributeValue> clone() const = 0;
};

// 具体的属性值类，可存储任意类型
template<typename T>
class TypedAttributeValue : public AttributeValue {
public:
    explicit TypedAttributeValue(const T& value) : value_(value) {}
    
    const std::type_info& getType() const override {
        return typeid(T);
    }
    
    const T& getValue() const {
        return value_;
    }
    
    std::unique_ptr<AttributeValue> clone() const override {
        return std::unique_ptr<AttributeValue>(new TypedAttributeValue<T>(value_));
    }
    
private:
    T value_;
};

// 通用资源节点类 - 继承自enable_shared_from_this以支持shared_from_this()
class ResourceNode : public std::enable_shared_from_this<ResourceNode> {
public:
    ResourceNode(const std::string& name, const std::string& id);
    virtual ~ResourceNode();

    // 节点基本属性
    const std::string& getName() const;
    const std::string& getId() const;
    void setName(const std::string& name);
    
    // 子节点管理
    void addChild(std::shared_ptr<ResourceNode> child);
    void removeChild(const std::string& id);
    std::shared_ptr<ResourceNode> getChild(const std::string& id) const;
    const std::vector<std::shared_ptr<ResourceNode>>& getChildren() const;
    
    // 属性管理 - 允许节点存储任意类型的属性
    template<typename T>
    void setAttribute(const std::string& key, const T& value) {
        attributes_[key] = std::unique_ptr<AttributeValue>(new TypedAttributeValue<T>(value));
    }
    
    template<typename T>
    T getAttribute(const std::string& key) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            auto* typedValue = dynamic_cast<TypedAttributeValue<T>*>(it->second.get());
            if (typedValue) {
                return typedValue->getValue();
            }
            throw std::bad_cast();
        }
        throw std::runtime_error("Attribute not found: " + key);
    }
    
    bool hasAttribute(const std::string& key) const {
        return attributes_.find(key) != attributes_.end();
    }
    
    void removeAttribute(const std::string& key) {
        attributes_.erase(key);
    }
    
    std::vector<std::string> getAttributeKeys() const {
        std::vector<std::string> keys;
        keys.reserve(attributes_.size());
        for (const auto& pair : attributes_) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    const std::type_info& getAttributeType(const std::string& key) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            return it->second->getType();
        }
        throw std::runtime_error("Attribute not found: " + key);
    }
    
    // 节点类型标识
    virtual std::string getType() const;
    
    // 复制节点（不复制子节点）
    std::shared_ptr<ResourceNode> clone() const;
    
    // 深度遍历节点
    void traverse(const std::function<void(const std::shared_ptr<ResourceNode>&, int depth)>& visitor, int depth = 0) const;

private:
    std::string name_;
    std::string id_;
    std::vector<std::shared_ptr<ResourceNode>> children_;
    std::unordered_map<std::string, std::shared_ptr<ResourceNode>> childMap_;
    
    // 通用属性存储 - 使用类型擦除代替std::any
    std::unordered_map<std::string, std::unique_ptr<AttributeValue>> attributes_;
};

} // namespace resource_management