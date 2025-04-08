#include "resource_api.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace resource;

// 自定义打印函数，用于展示查询结果
void printSearchResults(const std::string& queryTitle, const std::vector<std::shared_ptr<ResourceNode>>& results) {
    std::cout << "\n=== " << queryTitle << " ===" << std::endl;
    std::cout << "找到 " << results.size() << " 个结果:" << std::endl;
    
    for (const auto& node : results) {
        std::cout << "- " << node->getName() << " (ID: " << node->getId() << ")" << std::endl;
        
        // 打印节点的属性
        auto keys = node->getAttributeKeys();
        if (!keys.empty()) {
            std::cout << "  属性:" << std::endl;
            for (const auto& key : keys) {
                std::cout << "    " << key << ": ";
                
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
                } catch (...) {
                    std::cout << "[无法读取]";
                }
                std::cout << std::endl;
            }
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // 创建资源注册表和索引器
    ResourceRegistry registry;
    ResourceIndexer indexer(registry);
    
    // 创建弹群资源树
    auto group1 = std::make_shared<ResourceNode>("弹群1", "group001");
    group1->setAttribute("类型", std::string("演示用混合弹群"));
    auto cluster1 = std::make_shared<ResourceNode>("弹簇1", "cluster001");
    auto cluster2 = std::make_shared<ResourceNode>("弹簇2", "cluster002");
    group1->addChild(cluster1);
    group1->addChild(cluster2);
    
    // 创建第一个弹簇的弹体
    auto missile1_1 = std::make_shared<ResourceNode>("弹1", "m1-1");
    auto missile1_2 = std::make_shared<ResourceNode>("弹2", "m1-2");
    auto missile1_3 = std::make_shared<ResourceNode>("弹3", "m1-3");
    auto missile1_4 = std::make_shared<ResourceNode>("弹4", "m1-4");
    auto missile1_5 = std::make_shared<ResourceNode>("弹5", "m1-5");
    auto missile1_6 = std::make_shared<ResourceNode>("弹6", "m1-6");
    
    // 设置属性
    missile1_1->setAttribute("群首", true);
    missile1_1->setAttribute("簇首", true);
    missile1_1->setAttribute("导引头类型", std::string("主动雷达"));
    missile1_1->setAttribute("最大速度", 3.5);  // 马赫数
    
    missile1_2->setAttribute("导引头类型", std::string("红外"));
    missile1_2->setAttribute("最大速度", 3.0);
    
    missile1_3->setAttribute("导引头类型", std::string("被动雷达"));
    missile1_3->setAttribute("最大速度", 3.2);
    
    missile1_4->setAttribute("导引头类型", std::string("主动雷达"));
    missile1_4->setAttribute("最大速度", 3.5);
    
    missile1_5->setAttribute("导引头类型", std::string("红外"));
    missile1_5->setAttribute("最大速度", 3.0);
    
    missile1_6->setAttribute("导引头类型", std::string("被动雷达"));
    missile1_6->setAttribute("最大速度", 3.2);
    
    // 添加到弹簇
    cluster1->addChild(missile1_1);
    cluster1->addChild(missile1_2);
    cluster1->addChild(missile1_3);
    cluster1->addChild(missile1_4);
    cluster1->addChild(missile1_5);
    cluster1->addChild(missile1_6);
    
    // 创建第二个弹簇的弹体
    auto missile2_1 = std::make_shared<ResourceNode>("弹7", "m2-1");
    auto missile2_2 = std::make_shared<ResourceNode>("弹8", "m2-2");
    auto missile2_3 = std::make_shared<ResourceNode>("弹9", "m2-3");
    auto missile2_4 = std::make_shared<ResourceNode>("弹10", "m2-4");
    auto missile2_5 = std::make_shared<ResourceNode>("弹11", "m2-5");
    auto missile2_6 = std::make_shared<ResourceNode>("弹12", "m2-6");
    
    // 设置属性
    missile2_1->setAttribute("簇首", true);
    missile2_1->setAttribute("导引头类型", std::string("主动雷达"));
    missile2_1->setAttribute("最大速度", 4.0);
    missile2_1->setAttribute("搜索模式", std::string("主动扫描"));
    
    missile2_2->setAttribute("导引头类型", std::string("红外"));
    missile2_2->setAttribute("最大速度", 3.8);
    
    missile2_3->setAttribute("导引头类型", std::string("复合型"));
    missile2_3->setAttribute("最大速度", 3.9);
    missile2_3->setAttribute("搜索模式", std::string("被动接收"));
    
    missile2_4->setAttribute("导引头类型", std::string("主动雷达"));
    missile2_4->setAttribute("最大速度", 4.0);
    
    missile2_5->setAttribute("导引头类型", std::string("红外"));
    missile2_5->setAttribute("最大速度", 3.8);
    
    missile2_6->setAttribute("导引头类型", std::string("复合型"));
    missile2_6->setAttribute("最大速度", 3.9);
    missile2_6->setAttribute("搜索模式", std::string("被动接收"));
    missile2_6->setAttribute("seeker", 3);
    
    // 添加到弹簇
    cluster2->addChild(missile2_1);
    cluster2->addChild(missile2_2);
    cluster2->addChild(missile2_3);
    cluster2->addChild(missile2_4);
    cluster2->addChild(missile2_5);
    cluster2->addChild(missile2_6);
    
    // 注册根节点
    registry.registerRootNode(group1);
    
    // 显示整个资源树
    std::cout << "=== 完整资源树 ===" << std::endl;
    group1->traverse(simple_visitor);
    
    // 刷新索引
    indexer.refreshIndex();
    
    // 1. 通过名称查询节点
    auto nameResults = indexer.findByName("弹簇1");
    printSearchResults("按名称查询: '弹簇1'", nameResults);
    
    // 2. 通过ID查询节点
    auto idResults = indexer.findById("m1-1");
    printSearchResults("按ID查询: 'm1-1'", idResults);
    
    // 3. 通过属性查询 - 查找所有簇首
    auto clusterLeaders = indexer.findByAttribute<bool>("簇首", true);
    printSearchResults("按属性查询: 簇首=true", clusterLeaders);
    
    // 4. 通过属性查询 - 查找特定导引头类型
    auto radarMissiles = indexer.findByAttribute<std::string>("导引头类型", "主动雷达");
    printSearchResults("按属性查询: 导引头类型='主动雷达'", radarMissiles);
    
    // 5. 通过谓词查询 - 查找所有速度大于3.5马赫的弹体
    auto fastMissiles = indexer.findByPredicate([](const std::shared_ptr<ResourceNode>& node) {
        if (!node->hasAttribute("最大速度")) return false;
        
        try {
            double speed = node->getAttribute<double>("最大速度");
            return speed > 3.5;
        } catch (...) {
            return false;
        }
    });
    printSearchResults("按谓词查询: 最大速度>3.5", fastMissiles);
    
    // 6. 复合条件查询 - 同时满足多个条件(AND)
    std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>> andConditions;
    
    // 条件1: 必须是主动雷达导引头
    andConditions.push_back([](const std::shared_ptr<ResourceNode>& node) {
        if (!node->hasAttribute("导引头类型")) return false;
        try {
            return node->getAttribute<std::string>("导引头类型") == "主动雷达";
        } catch (...) {
            return false;
        }
    });
    
    // 条件2: 必须速度大于等于4.0
    andConditions.push_back([](const std::shared_ptr<ResourceNode>& node) {
        if (!node->hasAttribute("最大速度")) return false;
        try {
            return node->getAttribute<double>("最大速度") >= 4.0;
        } catch (...) {
            return false;
        }
    });
    
    auto andResults = indexer.findByMultiConditions(andConditions, true);
    printSearchResults("复合条件查询(AND): 主动雷达导引头 AND 速度>=4.0", andResults);
    
    // 7. 复合条件查询 - 满足任一条件(OR)
    std::vector<std::function<bool(const std::shared_ptr<ResourceNode>&)>> orConditions;
    
    // 条件1: 群首
    orConditions.push_back([](const std::shared_ptr<ResourceNode>& node) {
        if (!node->hasAttribute("群首")) return false;
        try {
            return node->getAttribute<bool>("群首") == true;
        } catch (...) {
            return false;
        }
    });
    
    // 条件2: 有seeker属性
    orConditions.push_back([](const std::shared_ptr<ResourceNode>& node) {
        return node->hasAttribute("seeker");
    });
    
    auto orResults = indexer.findByMultiConditions(orConditions, false);
    printSearchResults("复合条件查询(OR): 群首 OR 有seeker属性", orResults);
    
    return 0;
}