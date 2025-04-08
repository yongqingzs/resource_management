#include "resource_api.h"
#include <iostream>
#include <string>
#include <chrono>
#include <unordered_set>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace resource;
using namespace std::chrono;

// 打印查询结果的辅助函数
void printResults(const std::string& queryTitle, const std::vector<std::shared_ptr<ResourceNode>>& results) {
    std::cout << "\n=== " << queryTitle << " ===" << std::endl;
    std::cout << "找到 " << results.size() << " 个结果:" << std::endl;
    
    // 只打印前5个结果，避免输出太多
    int count = 0;
    for (const auto& node : results) {
        if (count++ >= 5) {
            std::cout << "... 以及 " << (results.size() - 5) << " 个更多结果" << std::endl;
            break;
        }
        
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

// 计时函数模板，用于比较性能
template<typename Func>
long long measureTime(Func func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count();
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // 创建资源注册表和索引器
    ResourceRegistry registry;
    ResourceIndexer indexer(registry);
    
    std::cout << "=== 资源索引示例 ===" << std::endl;
    std::cout << "本示例演示索引如何加速属性查询，包括精确匹配和范围查询" << std::endl;
    
    // 创建弹群结构
    auto group = std::make_shared<ResourceNode>("导弹集群", "missile-group");
    
    // 创建多种不同的导弹类型
    const int MISSILE_COUNT = 5000;  // 创建大量导弹来测试性能
    
    std::cout << "\n正在创建 " << MISSILE_COUNT << " 个导弹节点..." << std::endl;
    
    // 导弹类型
    std::vector<std::string> missileTypes = {
        "空空导弹", "地空导弹", "空地导弹", "地地导弹", "反舰导弹"
    };
    
    // 导引头类型
    std::vector<std::string> seekerTypes = {
        "主动雷达", "半主动雷达", "红外", "激光", "光电", "复合型"
    };
    
    // 燃料类型
    std::vector<std::string> fuelTypes = {
        "固体燃料", "液体燃料", "混合燃料"
    };
    
    // 随机创建导弹
    for (int i = 0; i < MISSILE_COUNT; ++i) {
        std::string id = "missile-" + std::to_string(i + 1);
        std::string name = "导弹" + std::to_string(i + 1);
        
        auto missile = std::make_shared<ResourceNode>(name, id);
        
        // 设置随机属性
        std::string missileType = missileTypes[i % missileTypes.size()];
        std::string seekerType = seekerTypes[(i * 3) % seekerTypes.size()];
        std::string fuelType = fuelTypes[(i * 7) % fuelTypes.size()];
        
        missile->setAttribute("类型", missileType);
        missile->setAttribute("导引头", seekerType);
        missile->setAttribute("燃料", fuelType);
        missile->setAttribute("射程", 100.0 + (i % 10) * 50.0);  // 100-550公里
        missile->setAttribute("速度", 2.5 + (i % 6) * 0.5);      // 2.5-5.0马赫
        missile->setAttribute("重量", 500 + (i % 20) * 100);     // 500-2400公斤
        missile->setAttribute("已部署", (i % 3) == 0);           // 1/3的导弹已部署
        missile->setAttribute("库存数量", 10 + (i % 10));        // 10-19个
        
        group->addChild(missile);
    }
    
    // 注册根节点
    registry.registerRootNode(group);
    std::cout << "资源树创建完成，共有 " << MISSILE_COUNT << " 个导弹节点" << std::endl;
    
    // 刷新基本索引
    indexer.refreshIndex();
    
    std::cout << "\n=== 开始性能对比测试 ===" << std::endl;
    std::cout << "将分别使用遍历查询和哈希查询进行比较" << std::endl;
    
    // 测试1：按导弹类型查询
    std::cout << "\n测试1: 查询所有空空导弹" << std::endl;
    
    // 遍历查询
    long long normalTime1 = measureTime([&]() {
        auto result = indexer.findByAttribute<std::string>("类型", "空空导弹");
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime1 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建导弹类型属性索引..." << std::endl;
    indexer.createAttributeIndex<std::string>("类型");
    
    // 哈希查询
    long long indexedTime1 = measureTime([&]() {
        auto result = indexer.findByAttributeIndexed<std::string>("类型", "空空导弹");
        std::cout << "哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "哈希查询耗时: " << indexedTime1 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup1 = static_cast<double>(normalTime1) / indexedTime1;
    std::cout << "性能提升: " << speedup1 << " 倍" << std::endl;
    
    // 测试2：按导引头类型查询
    std::cout << "\n测试2: 查询所有使用红外导引头的导弹" << std::endl;
    
    // 遍历查询
    long long normalTime2 = measureTime([&]() {
        auto result = indexer.findByAttribute<std::string>("导引头", "红外");
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime2 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建导引头属性索引..." << std::endl;
    indexer.createAttributeIndex<std::string>("导引头");
    
    // 哈希查询
    long long indexedTime2 = measureTime([&]() {
        auto result = indexer.findByAttributeIndexed<std::string>("导引头", "红外");
        std::cout << "哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "哈希查询耗时: " << indexedTime2 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup2 = static_cast<double>(normalTime2) / indexedTime2;
    std::cout << "性能提升: " << speedup2 << " 倍" << std::endl;
    
    // 测试3：按部署状态查询
    std::cout << "\n测试3: 查询所有已部署的导弹" << std::endl;
    
    // 遍历查询
    long long normalTime3 = measureTime([&]() {
        auto result = indexer.findByAttribute<bool>("已部署", true);
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime3 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建部署状态属性索引..." << std::endl;
    indexer.createAttributeIndex<bool>("已部署");
    
    // 哈希查询
    long long indexedTime3 = measureTime([&]() {
        auto result = indexer.findByAttributeIndexed<bool>("已部署", true);
        std::cout << "哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "哈希查询耗时: " << indexedTime3 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup3 = static_cast<double>(normalTime3) / indexedTime3;
    std::cout << "性能提升: " << speedup3 << " 倍" << std::endl;
    
    // 测试4: 范围查询 - 射程大于400公里的导弹
    std::cout << "\n测试4: 范围查询 - 射程大于400公里的导弹" << std::endl;
    
    // 遍历查询
    long long normalTime4 = measureTime([&]() {
        auto result = indexer.findByPredicate([](const std::shared_ptr<ResourceNode>& node) {
            if (!node->hasAttribute("射程")) return false;
            try {
                double range = node->getAttribute<double>("射程");
                return range > 400.0;
            } catch (...) {
                return false;
            }
        });
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime4 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建射程属性索引..." << std::endl;
    indexer.createAttributeIndex<double>("射程");
    
    // 范围查询
    long long indexedTime4 = measureTime([&]() {
        auto result = indexer.findGreaterThan<double>("射程", 400.0);
        std::cout << "范围哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "范围哈希查询耗时: " << indexedTime4 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup4 = static_cast<double>(normalTime4) / indexedTime4;
    std::cout << "性能提升: " << speedup4 << " 倍" << std::endl;
    
    // 打印一些结果
    auto highRangeMissiles = indexer.findGreaterThan<double>("射程", 400.0);
    printResults("射程>400公里的导弹(前5个)", highRangeMissiles);
    
    // 测试5: 范围查询 - 速度小于3.5马赫的导弹
    std::cout << "\n测试5: 范围查询 - 速度小于3.5马赫的导弹" << std::endl;
    
    // 遍历查询
    long long normalTime5 = measureTime([&]() {
        auto result = indexer.findByPredicate([](const std::shared_ptr<ResourceNode>& node) {
            if (!node->hasAttribute("速度")) return false;
            try {
                double speed = node->getAttribute<double>("速度");
                return speed < 3.5;
            } catch (...) {
                return false;
            }
        });
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime5 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建速度属性索引..." << std::endl;
    indexer.createAttributeIndex<double>("速度");
    
    // 范围查询
    long long indexedTime5 = measureTime([&]() {
        auto result = indexer.findLessThan<double>("速度", 3.5);
        std::cout << "范围哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "范围哈希查询耗时: " << indexedTime5 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup5 = static_cast<double>(normalTime5) / indexedTime5;
    std::cout << "性能提升: " << speedup5 << " 倍" << std::endl;
    
    // 测试6: 范围查询 - 重量在1000-2000公斤之间的导弹
    std::cout << "\n测试6: 范围查询 - 重量在1000-2000公斤之间的导弹" << std::endl;
    
    // 遍历查询
    long long normalTime6 = measureTime([&]() {
        auto result = indexer.findByPredicate([](const std::shared_ptr<ResourceNode>& node) {
            if (!node->hasAttribute("重量")) return false;
            try {
                int weight = node->getAttribute<int>("重量");
                return weight >= 1000 && weight <= 2000;
            } catch (...) {
                return false;
            }
        });
        std::cout << "遍历查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "遍历查询耗时: " << normalTime6 << " 微秒" << std::endl;
    
    // 创建索引
    std::cout << "创建重量属性索引..." << std::endl;
    indexer.createAttributeIndex<int>("重量");
    
    // 范围查询
    long long indexedTime6 = measureTime([&]() {
        auto result = indexer.findInRange<int>("重量", 1000, 2000);
        std::cout << "范围哈希查询找到 " << result.size() << " 个结果" << std::endl;
    });
    std::cout << "范围哈希查询耗时: " << indexedTime6 << " 微秒" << std::endl;
    
    // 计算加速比
    double speedup6 = static_cast<double>(normalTime6) / indexedTime6;
    std::cout << "性能提升: " << speedup6 << " 倍" << std::endl;
    
    return 0;
}