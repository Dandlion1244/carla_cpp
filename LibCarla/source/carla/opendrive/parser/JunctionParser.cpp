// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 路口解析器
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/JunctionParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  // 执行解析过程
  void JunctionParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    // 道路连接（开始道路ID + 结束道路 ID）
    struct LaneLink {
      road::LaneId from;
      road::LaneId to;
    };

    struct Connection {
      // id 表示连接的唯一标识符，类型为 road::ConId。这个字段用于唯一标识一条连接关系。
      road::ConId id;

      // incoming_road 表示进入连接的道路标识符，类型为 road::RoadId。
      // 该字段指向当前连接的起始道路，即车辆从哪个道路进入当前连接。
      road::RoadId incoming_road;

      // connecting_road 表示与连接道路相关的目标道路标识符，类型为 road::RoadId。
      // 该字段指向与当前连接相关的目标道路，即车辆从连接点驶向的道路。
      road::RoadId connecting_road;

      // lane_links 是一个 std::vector<LaneLink>，表示连接道路上的车道关系。
      // 每个 LaneLink 描述了如何从进入的道路的某个车道切换到连接道路的某个车道。
      std::vector<LaneLink> lane_links;
    };


    // 路口
    struct Junction {
      road::JuncId id;
      std::string name;
      std::vector<Connection> connections;
      std::set<road::ContId> controllers;
    };

    pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

    // 路口
    std::vector<Junction> junctions;
    for (pugi::xml_node junction_node : open_drive_node.children("junction")) {
      Junction junction;
      junction.id = junction_node.attribute("id").as_int();
      junction.name = junction_node.attribute("name").value();

      // 连接
      for (pugi::xml_node connection_node : junction_node.children("connection")) {

        Connection connection;
        connection.id = connection_node.attribute("id").as_uint();
        connection.incoming_road = connection_node.attribute("incomingRoad").as_uint();
        connection.connecting_road = connection_node.attribute("connectingRoad").as_uint();

        // Lane Links
        for (pugi::xml_node lane_link_node : connection_node.children("laneLink")) {

          LaneLink lane_link;
          lane_link.from = lane_link_node.attribute("from").as_int();
          lane_link.to = lane_link_node.attribute("to").as_int();

          connection.lane_links.push_back(lane_link);
        }

        junction.connections.push_back(connection);
      }

      // 控制器
      for (pugi::xml_node controller_node : junction_node.children("controller")) {
        const road::ContId controller_id = controller_node.attribute("id").value();
        // const std::string controller_name = controller_node.attribute("name").value();
        // const uint32_t controller_sequence = controller_node.attribute("sequence").as_uint();
        junction.controllers.insert(controller_id);
      }

      junctions.push_back(junction);
    }

    // 填充地图生成器
  // 遍历所有的路口（junctions）
for (auto &junction : junctions) {
  // 向地图构建器（map_builder）添加一个路口，包括路口的id和名称
  map_builder.AddJunction(junction.id, junction.name);
  // 遍历当前路口的所有连接
  for (auto &connection : junction.connections) {
    // 向地图构建器添加一个连接，包括连接的起点路口id、终点路口id、进入道路和连接道路
    map_builder.AddConnection(
        junction.id,
        connection.id,
        connection.incoming_road,
        connection.connecting_road);
    // 遍历当前连接的所有车道链接（lane_links）
    for (auto &lane_link : connection.lane_links) {
      // 向地图构建器添加一个车道链接，包括路口id、连接id、车道的起始位置和结束位置
      map_builder.AddLaneLink(junction.id,
          connection.id,
          lane_link.from,
          lane_link.to);
    }
  }
  // 向地图构建器添加一个路口控制器，并将当前路口的控制器移动到构建器中
  // 使用std::move是因为控制器在添加后不再需要，这样可以提高效率
  map_builder.AddJunctionController(junction.id, std::move(junction.controllers));
}
} // namespace parser
} // namespace opendrive
} // namespace carla
