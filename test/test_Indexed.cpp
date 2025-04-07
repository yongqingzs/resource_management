#include "resource_api.h"
#include <iostream>
#include <string>
#include <chrono>

using namespace resource;
using namespace std::chrono;

// 打印查询结果的辅助函数
void printResults(const std::string& queryTitle, const std::vector<std::shared_ptr<ResourceNode>>& results) {
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

// 计时函数模板，用于比较性能
template<typename Func>
long long measureTime(Func func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count();
}

int main() {
    // 创建资源注册表和索引器
    ResourceRegistry registry;
    ResourceIndexer indexer(registry);
    
    std::cout << "=== 资源索引哈希表示例 ===" << std::endl;
    std::cout << "本示例演示哈希表索引如何加速属性查询" << std::endl;
    
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
    
    // 测试4: 检查不存在的属性
    std::cout << "\n测试4: 查询具有'特殊标记'属性的导弹(该属性不存在)" << std::endl;
    
    // 哈希查询
    auto specialMarkedMissiles = indexer.findByAttributeIndexed<bool>("特殊标记", true);
    std::cout << "哈希查询找到 " << specialMarkedMissiles.size() << " 个结果" << std::endl;
    
    // 测试5: 动态添加新节点后的索引更新
    std::cout << "\n测试5: 添加新节点并测试索引更新机制" << std::endl;
    
    // 添加一个新的导弹节点
    auto newMissile = std::make_shared<ResourceNode>("新型高超音速导弹", "hypersonic-1");
    newMissile->setAttribute("类型", "高超音速导弹");
    newMissile->setAttribute("导引头", "复合型");
    newMissile->setAttribute("燃料", "特种燃料");
    newMissile->setAttribute("速度", 10.0);
    newMissile->setAttribute("已部署", true);
    
    group->addChild(newMissile);
    
    std::cout << "添加了一个新型高超音速导弹，但尚未刷新索引" << std::endl;
    
    // 在刷新索引前查询
    auto hypersonicBefore = indexer.findByAttributeIndexed<std::string>("类型", "高超音速导弹");
    std::cout << "刷新索引前查询高超音速导弹: 找到 " << hypersonicBefore.size() << " 个结果" << std::endl;
    
    // 刷新索引
    std::cout << "刷新索引..." << std::endl;
    indexer.refreshIndex();
    
    // 必须重新创建索引，因为刷新会清空所有索引
    indexer.createAttributeIndex<std::string>("类型");
    
    // 在刷新索引后查询
    auto hypersonicAfter = indexer.findByAttributeIndexed<std::string>("类型", "高超音速导弹");
    std::cout << "刷新索引后查询高超音速导弹: 找到 " << hypersonicAfter.size() << " 个结果" << std::endl;
    
    if (!hypersonicAfter.empty()) {
        printResults("新添加的高超音速导弹信息", hypersonicAfter);
    }
    
    // // 测试6: 实际使用场景 - 查找所有高射程、高速度的导弹
    // std::cout << "\n测试6: 实际使用场景 - 查找所有射程>=400公里且速度>=4.0马赫的导弹" << std::endl;
    
    // // 创建射程和速度索引
    // indexer.createAttributeIndex<double>("射程");
    // indexer.createAttributeIndex<double>("速度");
    
    // // 组装查询结果 - 演示如何组合多个哈希查询结果
    // auto highRangeMissiles = indexer.findByAttributeIndexed<double>("射程", 400.0);
    // auto highRangeMissiles2 = indexer.findByAttributeIndexed<double>("射程", 450.0);
    // auto highRangeMissiles3 = indexer.findByAttributeIndexed<double>("射程", 500.0);
    // auto highRangeMissiles4 = indexer.findByAttributeIndexed<double>("射程", 550.0);
    
    // auto highSpeedMissiles = indexer.findByAttributeIndexed<double>("速度", 4.0);
    // auto highSpeedMissiles2 = indexer.findByAttributeIndexed<double>("速度", 4.5);
    // auto highSpeedMissiles3 = indexer.findByAttributeIndexed<double>("速度", 5.0);
    
    // // 合并所有高射程导弹
    // std::unordered_set<std::shared_ptr<ResourceNode>> highRangeSet;
    // for (const auto& missile : highRangeMissiles) highRangeSet.insert(missile);
    // for (const auto& missile : highRangeMissiles2) highRangeSet.insert(missile);
    // for (const auto& missile : highRangeMissiles3) highRangeSet.insert(missile);
    // for (const auto& missile : highRangeMissiles4) highRangeSet.insert(missile);
    
    // // 合并所有高速度导弹
    // std::unordered_set<std::shared_ptr<ResourceNode>> highSpeedSet;
    // for (const auto& missile : highSpeedMissiles) highSpeedSet.insert(missile);
    // for (const auto& missile : highSpeedMissiles2) highSpeedSet.insert(missile);
    // for (const auto& missile : highSpeedMissiles3) highSpeedSet.insert(missile);
    
    // // 找出两个集合的交集
    // std::vector<std::shared_ptr<ResourceNode>> highPerformanceMissiles;
    // for (const auto& missile : highRangeSet) {
    //     if (highSpeedSet.count(missile) > 0) {
    //         highPerformanceMissiles.push_back(missile);
    //     }
    // }
    
    // std::cout << "找到 " << highPerformanceMissiles.size() << " 个高性能导弹" << std::endl;
    
    // // 显示前5个结果
    // int count = 0;
    // std::vector<std::shared_ptr<ResourceNode>> topResults;
    // for (const auto& missile : highPerformanceMissiles) {
    //     if (count++ < 5) {
    //         topResults.push_back(missile);
    //     }
    // }
    // printResults("高性能导弹(前5个)", topResults);
    
    // std::cout << "\n=== 索引哈希表测试总结 ===" << std::endl;
    // std::cout << "1. 哈希表索引在查询大量数据时能显著提升性能" << std::endl;
    // std::cout << "2. 索引需要在数据变更后刷新才能保持最新状态" << std::endl;
    // std::cout << "3. 可以组合多个哈希查询结果实现复杂查询" << std::endl;
    
    return 0;
}