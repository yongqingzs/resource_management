#include "../include/resource_node.h"
#include "../include/resource_registry.h"
#include "../include/resource_indexer.h"
#include <iostream>
#include <string>

using namespace resource_management;

int main() {
    // 创建资源注册表和索引器
    ResourceRegistry registry;
    ResourceIndexer indexer(registry);
    
    // 创建服务器资源树
    auto datacenter = std::make_shared<ResourceNode>("数据中心", "dc001");
    
    // 添加服务器集群
    auto cluster1 = std::make_shared<ResourceNode>("Web集群", "cluster001");
    auto cluster2 = std::make_shared<ResourceNode>("数据库集群", "cluster002");
    
    datacenter->addChild(cluster1);
    datacenter->addChild(cluster2);
    
    // 为集群1添加服务器
    auto webServer1 = std::make_shared<ResourceNode>("Web服务器1", "web001");
    auto webServer2 = std::make_shared<ResourceNode>("Web服务器2", "web002");
    
    // 为服务器添加属性
    webServer1->setAttribute("cpu_cores", 8);
    webServer1->setAttribute("memory", 16.0); // GB
    webServer1->setAttribute("ip_address", std::string("192.168.1.10"));
    webServer1->setAttribute("is_active", true);
    
    webServer2->setAttribute("cpu_cores", 16);
    webServer2->setAttribute("memory", 32.0); // GB
    webServer2->setAttribute("ip_address", std::string("192.168.1.11"));
    webServer2->setAttribute("is_active", true);
    
    cluster1->addChild(webServer1);
    cluster1->addChild(webServer2);
    
    // 为集群2添加数据库服务器
    auto dbServer1 = std::make_shared<ResourceNode>("DB服务器1", "db001");
    auto dbServer2 = std::make_shared<ResourceNode>("DB服务器2", "db002");
    
    dbServer1->setAttribute("cpu_cores", 32);
    dbServer1->setAttribute("memory", 128.0); // GB
    dbServer1->setAttribute("ip_address", std::string("192.168.2.10"));
    dbServer1->setAttribute("db_type", std::string("MySQL"));
    dbServer1->setAttribute("is_active", true);
    
    dbServer2->setAttribute("cpu_cores", 32);
    dbServer2->setAttribute("memory", 128.0); // GB
    dbServer2->setAttribute("ip_address", std::string("192.168.2.11"));
    dbServer2->setAttribute("db_type", std::string("PostgreSQL"));
    dbServer2->setAttribute("is_active", false);
    
    cluster2->addChild(dbServer1);
    cluster2->addChild(dbServer2);
    
    // 为数据库服务器添加子资源
    auto mysqlInstance = std::make_shared<ResourceNode>("MySQL实例", "mysql001");
    mysqlInstance->setAttribute("port", 3306);
    mysqlInstance->setAttribute("version", std::string("8.0"));
    mysqlInstance->setAttribute("max_connections", 1000);
    
    dbServer1->addChild(mysqlInstance);
    
    // 注册根节点
    registry.registerRootNode(datacenter);
    
    // 使用索引器查询资源
    std::cout << "=== 按属性查询资源示例 ===" << std::endl;
    
    // 查找所有活跃的服务器
    auto activeNodes = indexer.findByAttribute<bool>("is_active", true);
    std::cout << "活跃的服务器数: " << activeNodes.size() << std::endl;
    for (const auto& node : activeNodes) {
        std::cout << "- " << node->getName() << " (ID: " << node->getId() << ")" << std::endl;
    }
    
    std::cout << "\n=== 路径查询示例 ===" << std::endl;
    
    // 通过路径获取节点
    auto dbNode = registry.getNodeByPath("dc001/cluster002/db001");
    if (dbNode) {
        std::cout << "找到数据库服务器: " << dbNode->getName() << std::endl;
        std::cout << "IP地址: " << dbNode->getAttribute<std::string>("ip_address") << std::endl;
        std::cout << "数据库类型: " << dbNode->getAttribute<std::string>("db_type") << std::endl;
    }
    
    std::cout << "\n=== 条件查询示例 ===" << std::endl;
    
    // 查找所有内存大于64GB的服务器
    auto highMemoryNodes = indexer.findByPredicate([](const std::shared_ptr<ResourceNode>& node) {
        if (!node->hasAttribute("memory")) return false;
        try {
            double memory = node->getAttribute<double>("memory");
            return memory > 64.0;
        } catch (...) {
            return false;
        }
    });
    
    std::cout << "内存大于64GB的服务器: " << highMemoryNodes.size() << std::endl;
    for (const auto& node : highMemoryNodes) {
        std::cout << "- " << node->getName() 
                  << " 内存: " << node->getAttribute<double>("memory") << "GB" << std::endl;
    }
    
    // 遍历整个资源树
    std::cout << "\n=== 资源树结构 ===" << std::endl;
    datacenter->traverse([](const std::shared_ptr<ResourceNode>& node, int depth) {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "- " << node->getName() << " (ID: " << node->getId() << ")" << std::endl;
        
        // 打印节点属性
        auto keys = node->getAttributeKeys();
        if (!keys.empty()) {
            std::string attrIndent(depth * 2 + 2, ' ');
            std::cout << attrIndent << "属性:" << std::endl;
            for (const auto& key : keys) {
                std::cout << attrIndent << "  " << key << ": ";
                
                // 尝试输出常见类型的属性值
                try {
                    if (node->getAttributeType(key) == typeid(int)) {
                        std::cout << node->getAttribute<int>(key);
                    }
                    else if (node->getAttributeType(key) == typeid(double)) {
                        std::cout << node->getAttribute<double>(key);
                    }
                    else if (node->getAttributeType(key) == typeid(std::string)) {
                        std::cout << node->getAttribute<std::string>(key);
                    }
                    else if (node->getAttributeType(key) == typeid(bool)) {
                        std::cout << (node->getAttribute<bool>(key) ? "true" : "false");
                    }
                    else {
                        std::cout << "[复杂类型]";
                    }
                } catch (...) {
                    std::cout << "[错误:无法读取]";
                }
                std::cout << std::endl;
            }
        }
    });
    
    return 0;
}