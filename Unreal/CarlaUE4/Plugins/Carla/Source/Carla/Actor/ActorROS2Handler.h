// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <compiler/enable-ue4-macros.h>

/// 访问者类
// 定义一个名为ActorROS2Handler的类，用于处理与ROS 2相关的Actor操作
class ActorROS2Handler
{
public:
    // 构造函数是被删除的，这意味着不能默认构造ActorROS2Handler对象
    ActorROS2Handler() = delete;

    // 带参数的构造函数，用于创建ActorROS2Handler对象
    // 参数：
    // AActor* Actor：指向Unreal Engine中Actor的指针
    // std::string RosName：ROS 2节点的名称
    ActorROS2Handler(AActor *Actor, std::string RosName) : _Actor(Actor), _RosName(RosName) {};

    // 重载函数调用运算符()，允许对象像函数一样被调用
    // 参数：
    // carla::ros2::VehicleControl& Source：一个引用，指向车辆控制信息的ROS 2消息
    void operator()(carla::ros2::VehicleControl &Source);

private:
    // 成员变量，存储指向Unreal Engine中Actor的指针
    AActor *_Actor {nullptr};

    // 成员变量，存储ROS 2节点的名称
    std::string _RosName;
};
