#include "resource_api.h"

using namespace resource;

int main()
{
    ResourceRegistry registry;
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
    missile2_6->setAttribute("seeker", int(3));
    cluster2->addChild(missile2_1);
    cluster2->addChild(missile2_2);
    cluster2->addChild(missile2_3);
    cluster2->addChild(missile2_4);
    cluster2->addChild(missile2_5);
    cluster2->addChild(missile2_6);

    registry.registerRootNode(group1);
    // group1->traverse(simple_visitor);
    std::cout << "\n=== 资源树结构 ===" << std::endl;
    registry.traverseRootNode(simple_visitor);

    std::shared_ptr<resource::ResourceNode> node_ptr;
    bool result_flag = false;
    std::cout << "\n=== 通过路径获取节点(group001/cluster002/m2-6) ===" << std::endl;
    node_ptr = registry.getNodeByPath("group001/cluster002/m2-6");
    node_ptr->traverse(simple_visitor);

    std::cout << "\n=== 通过路径获取节点(错误路径 group001/cluster002/m2-7) ===" << std::endl;
    if (registry.getNodeByPath("group001/cluster002/m2-7") == nullptr)
    {
        std::cout << "没有找到该节点" << std::endl;
    }
    else
    {
        std::cout << "找到该节点" << std::endl;
        node_ptr = registry.getNodeByPath("group001/cluster002/m2-7");
        node_ptr->traverse(simple_visitor);
    }
    
    std::cout << "\n=== 通过路径注册节点(group001/cluster002/m2-7) ===" << std::endl;
    result_flag = registry.registerNodeAtPath("group001/cluster002/m2-7", std::make_shared<ResourceNode>("弹13", "m2-7"));
    if (result_flag)
    {
        std::cout << "注册成功" << std::endl;
        // node_ptr = registry.getNodeByPath("group001/cluster002/m2-7");
        // node_ptr->traverse(simple_visitor);
        registry.traverseRootNode(simple_visitor);
    }
    else
    {
        std::cout << "注册失败" << std::endl;
    }

    std::cout << "\n=== 通过路径注册节点(错误路径 group001/cluster003/m3-1) ===" << std::endl;
    std::cout << "note: 前面路径必须存在节点" << std::endl;
    result_flag = registry.registerNodeAtPath("group001/cluster003/m3-1", std::make_shared<ResourceNode>("弹14", "m3-1"));
    if (result_flag)
    {
        std::cout << "注册成功" << std::endl;
        // node_ptr = registry.getNodeByPath("group001/cluster003/m3-1");
        // node_ptr->traverse(simple_visitor);
        registry.traverseRootNode(simple_visitor);
    }
    else
    {
        std::cout << "注册失败" << std::endl;
    }

    std::cout << "\n=== 通过路径删除节点(group001/cluster002/m2-7) ===" << std::endl;
    result_flag = registry.removeNodeByPath("group001/cluster002/m2-7");
    if (result_flag)
    {
        std::cout << "删除成功" << std::endl;
        // node_ptr->traverse(simple_visitor);
        registry.traverseRootNode(simple_visitor);
    }
    else
    {
        std::cout << "删除失败" << std::endl;
    }

    std::cout << "\n=== 直接创建路径(group001/cluster003/m3-1) ===" << std::endl;
    node_ptr = registry.createPath("group001/cluster003/m3-1");
    if (node_ptr)
    {
        std::cout << "创建成功" << std::endl;
        registry.traverseRootNode(simple_visitor);
    }
    else
    {
        std::cout << "创建失败" << std::endl;
    }


    // 这部分功能可能移动到索引器中
    // std::cout << "\n=== 根据属性查找簇首(bool/=) ===" << std::endl;
    // auto activeNodes = registry.findNodesByAttribute<bool>("簇首", true);
    // for (auto & node : activeNodes)
    // {
    //     node->traverse(simple_visitor);
    // }

    // std::cout << "\n=== 根据属性查找seeker(double/=) ===" << std::endl;
    // auto seekerNodes = registry.findNodesByAttribute<int>("seeker", 3);
    // for (auto & node : seekerNodes)
    // {
    //     node->traverse(simple_visitor);
    // }
    return 0;
}