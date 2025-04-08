#include "resource_api.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace resource;

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    auto group1 = std::make_shared<ResourceNode>("弹群1", "group001");
    group1->setAttribute("类型", std::string("演示用混合弹群"));
    auto cluster1 = std::make_shared<ResourceNode>("弹簇1", "cluster001");
    auto cluster2 = std::make_shared<ResourceNode>("弹簇2", "cluster002");
    group1->addChild(cluster1);
    group1->addChild(cluster2);
    
    auto missile1_1 = std::make_shared<ResourceNode>("弹1", "m1-1");
    auto missile1_2 = std::make_shared<ResourceNode>("弹2", "m1-2");
    auto missile1_3 = std::make_shared<ResourceNode>("弹3", "m1-3");
    auto missile1_4 = std::make_shared<ResourceNode>("弹4", "m1-4");
    auto missile1_5 = std::make_shared<ResourceNode>("弹5", "m1-5");
    auto missile1_6 = std::make_shared<ResourceNode>("弹6", "m1-6");
    missile1_1->setAttribute("群首", true);
    missile1_1->setAttribute("簇首", true);
    cluster1->addChild(missile1_1);
    cluster1->addChild(missile1_2);
    cluster1->addChild(missile1_3);
    cluster1->addChild(missile1_4);
    cluster1->addChild(missile1_5);
    cluster1->addChild(missile1_6);
    
    auto missile2_1 = std::make_shared<ResourceNode>("弹7", "m2-1");
    auto missile2_2 = std::make_shared<ResourceNode>("弹8", "m2-2");
    auto missile2_3 = std::make_shared<ResourceNode>("弹9", "m2-3");
    auto missile2_4 = std::make_shared<ResourceNode>("弹10", "m2-4");
    auto missile2_5 = std::make_shared<ResourceNode>("弹11", "m2-5");
    auto missile2_6 = std::make_shared<ResourceNode>("弹12", "m2-6");
    missile2_1->setAttribute("簇首", true);
    cluster2->addChild(missile2_1);
    cluster2->addChild(missile2_2);
    cluster2->addChild(missile2_3);
    cluster2->addChild(missile2_4);
    cluster2->addChild(missile2_5);
    cluster2->addChild(missile2_6);

    std::cout << "\n=== 资源树结构 ===" << std::endl;
    group1->traverse(simple_visitor);

    std::cout << "\n=== 节点添加(添加弹13) ===" << std::endl;
    cluster2->addChild(std::make_shared<ResourceNode>("弹13", "m2-7"));
    group1->traverse(simple_visitor);

    std::cout << "\n=== 属性添加(添加弹8目标) ===" << std::endl;
    missile2_2->setAttribute("目标", std::string("Berkeley1"));
    group1->traverse(simple_visitor);

    std::cout << "\n=== 属性修改(修改弹8目标) ===" << std::endl;
    missile2_2->modifyAttribute("目标", std::string("YorkCity1"));
    group1->traverse(simple_visitor);

    std::cout << "\n=== 属性删除(删除弹8目标) ===" << std::endl;
    missile2_2->removeAttribute("目标");
    group1->traverse(simple_visitor);

    std::cout << "\n=== 节点删除（删除弹簇1） ===" << std::endl;  // 注意只针对child节点
    group1->removeChild("cluster001");
    group1->traverse(simple_visitor);
}