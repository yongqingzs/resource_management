#include "resource_api.h"
#ifdef _WIN32
#include <windows.h>
#endif 

/* 
导弹代理模型

(状态参数层)
1. 导弹类型
2. 导弹编号
3. 导弹群号
4. 导弹角色（是否领弹）
5. 经度
6. 纬度
7. 高度

(能力模型层)
- 机动能力
    1. 最大航程
    2. 机动速度
    3. 飞行高度
    4. 爬升率
    5. 转弯半径
    6. 最大切向加速度
    7. 最大法向加速度
- 毁伤能力
    - 弹药0
        1. 成员弹药数量
        2. TNT当量
        3. 毁伤半径
- 感知能力
    - 光学0
        1. 谱段
        2. 探测距离
        3. 俯仰范围
    - 射频1
        1. 谱段
        2. 探测距离
        3. 航向范围
- 对抗能力
    1. 对抗波段
    2. 对抗目标数
*/
// 感知能力子系统
struct PerceptionSystem {
    // 光学传感器
    struct OpticalSensor {
        std::string spectrumBand = "visible_light";  // 谱段
        double detectionRange = 20.0;        // 探测距离
        double pitchRange = 45.0;            // 俯仰范围

        OpticalSensor() = default;
        OpticalSensor(const std::string& band, double range, double pitch)
            : spectrumBand(band), detectionRange(range), pitchRange(pitch) {}
    };
    
    // 射频传感器
    struct RFSensor {
        std::string spectrumBand = "X";   // 谱段
        double detectionRange = 100.0;       // 探测距离
        double headingRange = 90.0;          // 航向范围

        RFSensor() = default;
        RFSensor(const std::string& band, double range, double heading)
            : spectrumBand(band), detectionRange(range), headingRange(heading) {}
    };

    OpticalSensor opticalSensor;  // 默认光学传感器
    RFSensor rfSensor;            // 默认射频传感器
};

// 毁伤能力子系统
struct DamageSystem {
    struct Warhead {
        int quantity = 1;              // 成员弹药数量
        double tntEquivalent = 100.0;  // TNT当量
        double damageRadius = 50.0;    // 毁伤半径

        Warhead() = default;
        Warhead(int qty, double tnt, double radius)
            : quantity(qty), tntEquivalent(tnt), damageRadius(radius) {}
    };

    Warhead warhead;  // 默认弹头
};

// 机动能力子系统
struct ManeuverSystem {
    double maxRange = 300.0;           // 最大航程
    double speed = 3.0;                // 机动速度
    double flightAltitude = 10000.0;   // 飞行高度
    double climbRate = 150.0;          // 爬升率
    double turningRadius = 5.0;        // 转弯半径
    double maxTangentialAccel = 4.0;   // 最大切向加速度
    double maxNormalAccel = 20.0;      // 最大法向加速度

    ManeuverSystem() = default;
    ManeuverSystem(double range, double spd, double alt, double climb, 
                   double turn, double tangAccel, double normAccel)
        : maxRange(range), speed(spd), flightAltitude(alt), 
          climbRate(climb), turningRadius(turn),
          maxTangentialAccel(tangAccel), maxNormalAccel(normAccel) {}
};

// 对抗能力子系统 - 修正countermeasureBands类型为字符串
struct CountermeasureSystem {
    std::string countermeasureBands = "infrared";  // 对抗波段
    int maxTargets = 2;                           // 对抗目标数

    CountermeasureSystem() = default;
    CountermeasureSystem(const std::string& bands, int targets)
        : countermeasureBands(bands), maxTargets(targets) {}
};

// 导弹代理模型主结构体
struct AgentModel {
    // 状态参数层
    std::string missileType = "ait_to_air";  // 导弹类型
    std::string missileId = "M001";       // 导弹编号
    std::string groupId = "G001";         // 导弹群号
    bool isLeader = false;                // 导弹角色（是否领弹）
    double longitude = 116.3;             // 经度
    double latitude = 39.9;               // 纬度
    double altitude = 5000.0;             // 高度
    
    // 能力模型层
    ManeuverSystem maneuverCapability;          // 机动能力
    DamageSystem damageCapability;              // 毁伤能力
    PerceptionSystem perceptionCapability;      // 感知能力
    CountermeasureSystem countermeasureCapability;  // 对抗能力

    AgentModel() = default;
    // 可以添加自定义构造函数，但由于参数众多，可以使用链式设置或构建器模式更灵活
};

// =====================结构体适配器=====================
class AgentModelConverter : public resource::StructConverter {
public:
    std::shared_ptr<resource::ResourceNode> convert(const void* structPtr, const std::string& nodeName) const override {
        const AgentModel& agent = *static_cast<const AgentModel*>(structPtr);
        
        auto node = std::make_shared<resource::ResourceNode>(nodeName, agent.missileId);
        
        // 添加基本属性
        node->setAttribute("missileType", agent.missileType);
        node->setAttribute("missileId", agent.missileId);
        node->setAttribute("groupId", agent.groupId);
        node->setAttribute("isLeader", agent.isLeader);
        node->setAttribute("longitude", agent.longitude);
        node->setAttribute("latitude", agent.latitude);
        node->setAttribute("altitude", agent.altitude);
        
        // 添加机动能力子节点
        auto maneuverNode = std::make_shared<resource::ResourceNode>("maneuver", agent.missileId + "maneuver");
        maneuverNode->setAttribute("maxRange", agent.maneuverCapability.maxRange);
        maneuverNode->setAttribute("speed", agent.maneuverCapability.speed);
        maneuverNode->setAttribute("flightAltitude", agent.maneuverCapability.flightAltitude);
        maneuverNode->setAttribute("climbRate", agent.maneuverCapability.climbRate);
        maneuverNode->setAttribute("turningRadius", agent.maneuverCapability.turningRadius);
        maneuverNode->setAttribute("maxTangentialAccel", agent.maneuverCapability.maxTangentialAccel);
        maneuverNode->setAttribute("maxNormalAccel", agent.maneuverCapability.maxNormalAccel);
        node->addChild(maneuverNode);
        
        // 毁伤能力子节点
        auto damageNode = std::make_shared<resource::ResourceNode>("damage", agent.missileId + "_damage");
        auto warheadNode = std::make_shared<resource::ResourceNode>("warhead", agent.missileId + "_warhead");
        warheadNode->setAttribute("quantity", agent.damageCapability.warhead.quantity);
        warheadNode->setAttribute("tntEquivalent", agent.damageCapability.warhead.tntEquivalent);
        warheadNode->setAttribute("damageRadius", agent.damageCapability.warhead.damageRadius);
        damageNode->addChild(warheadNode);
        node->addChild(damageNode);

        // 感知能力子节点
        auto perceptionNode = std::make_shared<resource::ResourceNode>("perception", agent.missileId + "_perception");

        // 光学传感器
        auto opticalNode = std::make_shared<resource::ResourceNode>("optical", agent.missileId + "_optical");
        opticalNode->setAttribute("spectrumBand", agent.perceptionCapability.opticalSensor.spectrumBand);
        opticalNode->setAttribute("detectionRange", agent.perceptionCapability.opticalSensor.detectionRange);
        opticalNode->setAttribute("pitchRange", agent.perceptionCapability.opticalSensor.pitchRange);
        perceptionNode->addChild(opticalNode);

        // 射频传感器
        auto rfNode = std::make_shared<resource::ResourceNode>("rf", agent.missileId + "_rf");
        rfNode->setAttribute("spectrumBand", agent.perceptionCapability.rfSensor.spectrumBand);
        rfNode->setAttribute("detectionRange", agent.perceptionCapability.rfSensor.detectionRange);
        rfNode->setAttribute("headingRange", agent.perceptionCapability.rfSensor.headingRange);
        perceptionNode->addChild(rfNode);

        node->addChild(perceptionNode);

        // 对抗能力子节点
        auto countermeasureNode = std::make_shared<resource::ResourceNode>("countermeasure", agent.missileId + "_countermeasure");
        countermeasureNode->setAttribute("countermeasureBands", agent.countermeasureCapability.countermeasureBands);
        countermeasureNode->setAttribute("maxTargets", agent.countermeasureCapability.maxTargets);
        node->addChild(countermeasureNode);
        
        return node;
    }

    // 实现克隆方法
    StructConverter* clone() const override {
        return new AgentModelConverter(*this);
    }
};

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    resource::ResourceRegistry registry;
    AgentModelConverter converter;
    AgentModel agent1;
    
    agent1.missileType = "air_to_ground";
    agent1.isLeader = true;
    agent1.maneuverCapability.maxRange = 500.0;
    
    auto node = registry.registerDynamicStruct(agent1, "", converter, "missile1");

    registry.traverseRootNode(resource::simple_visitor);

    int count = 1000;

    auto dynamic_func = [&]() {for (int i = 0; i < count; i++) {
        // 更新结构体
        agent1.longitude += 0.1;
        agent1.latitude += 0.05;
        agent1.altitude += 10.0;
        
        // 更新资源树
        registry.updateAllDynamicObjects();
        
        // 打印更新后的位置
        // std::cout << "周期 " << i << ": 经度=" << node->getAttribute<double>("longitude")
        //           << ", 纬度=" << node->getAttribute<double>("latitude")
        //           << ", 高度=" << node->getAttribute<double>("altitude") << std::endl;

        // std::cout << "\n==========Update==========" << std::endl;
        // registry.traverseRootNode(resource::simple_visitor);
        
        // 模拟时间间隔
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }};

    auto increment_func = [&]() {for (int i = 0; i < count; i++) {
        // 更新结构体
        agent1.longitude += 0.1;
        agent1.latitude += 0.05;
        agent1.altitude += 10.0;
        
        // 更新资源树
#if 1
        registry.batchUpdateAttributes(
            node,
            "longitude", agent1.longitude,
            "latitude", agent1.latitude,
            "altitude", agent1.altitude,
            "maneuver.turningRadius", agent1.maneuverCapability.turningRadius
        );
#endif

#if 0
        registry.batchUpdateAttributes(
            node,
            // 基本属性
            "missileType", agent1.missileType,
            "missileId", agent1.missileId,
            "groupId", agent1.groupId,
            "isLeader", agent1.isLeader,
            "longitude", agent1.longitude,
            "latitude", agent1.latitude,
            "altitude", agent1.altitude,
            
            // 机动能力属性
            "maneuver.maxRange", agent1.maneuverCapability.maxRange,
            "maneuver.speed", agent1.maneuverCapability.speed,
            "maneuver.flightAltitude", agent1.maneuverCapability.flightAltitude,
            "maneuver.climbRate", agent1.maneuverCapability.climbRate,
            "maneuver.turningRadius", agent1.maneuverCapability.turningRadius,
            "maneuver.maxTangentialAccel", agent1.maneuverCapability.maxTangentialAccel,
            "maneuver.maxNormalAccel", agent1.maneuverCapability.maxNormalAccel,
            
            // 毁伤能力属性
            "damage.warhead.quantity", agent1.damageCapability.warhead.quantity,
            "damage.warhead.tntEquivalent", agent1.damageCapability.warhead.tntEquivalent,
            "damage.warhead.damageRadius", agent1.damageCapability.warhead.damageRadius,
            
            // 感知能力 - 光学传感器属性
            "perception.optical.spectrumBand", agent1.perceptionCapability.opticalSensor.spectrumBand,
            "perception.optical.detectionRange", agent1.perceptionCapability.opticalSensor.detectionRange,
            "perception.optical.pitchRange", agent1.perceptionCapability.opticalSensor.pitchRange,
            
            // 感知能力 - 射频传感器属性
            "perception.rf.spectrumBand", agent1.perceptionCapability.rfSensor.spectrumBand,
            "perception.rf.detectionRange", agent1.perceptionCapability.rfSensor.detectionRange,
            "perception.rf.headingRange", agent1.perceptionCapability.rfSensor.headingRange,
            
            // 对抗能力属性
            "countermeasure.countermeasureBands", agent1.countermeasureCapability.countermeasureBands,
            "countermeasure.maxTargets", agent1.countermeasureCapability.maxTargets
        );
#endif

        // 打印更新后的位置
        // std::cout << "周期 " << i << ": 经度=" << node->getAttribute<double>("longitude")
        //           << ", 纬度=" << node->getAttribute<double>("latitude")
        //           << ", 高度=" << node->getAttribute<double>("altitude") << std::endl;

        // std::cout << "\n==========Update==========" << std::endl;
        // registry.traverseRootNode(resource::simple_visitor);
        
        // 模拟时间间隔
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }};

    std::cout << "\n==========Dynamic Update==========" << std::endl;
    long long normalTime1 = measureTime(dynamic_func);
    std::cout << "动态更新耗时: " << normalTime1 / 1000 << " ms" << std::endl;
    std::cout << "\n==========Increment Update==========" << std::endl;
    long long normalTime2 = measureTime(increment_func);
    std::cout << "增量更新耗时: " << normalTime2 / 1000 << " ms" << std::endl;
    return 0;
}