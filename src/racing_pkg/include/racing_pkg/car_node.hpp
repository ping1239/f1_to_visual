#ifndef RACING_PKG__CAR_NODE_HPP_
#define RACING_PKG__CAR_NODE_HPP_

#include <vector>
#include <string>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.h"

namespace racing_pkg
{

// 물리 기반 경로 추종을 위한 웨이포인트 구조체 선언
struct Waypoint {
  double s;   // 누적 주행 거리 (m)
  double x;   // X 좌표 (m)
  double y;   // Y 좌표 (m)
  double yaw; // 헤딩 방향 각도 (rad)
  double vx;  // 목표 속도 (m/s)
  double ax;  // 목표 가속도 (m/s^2)
};

class CarNode : public rclcpp::Node
{
public:
  // 생성자
  explicit CarNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  
  // 소멸자
  virtual ~CarNode() = default;

private:
  // 내부 핵심 메서드
  void load_trajectory_csv(const std::string & file_path);
  void update_position();

  // ROS 2 타이머 및 TF 브로드캐스터
  rclcpp::TimerBase::SharedPtr timer_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  // 물리 기반 주행 데이터 저장 변수
  std::vector<Waypoint> waypoints_;
  double current_s_{0.0}; // 인덱스 대신 실제 물리 이동 거리(s)를 추적

  int lap_count_{0};       // 현재 주행 바퀴 수
  bool is_first_lap_{true};// 첫 바퀴 진입 감지 플래그
  rclcpp::Time start_time_;// 노드 시작(출발) 시간
};

}  // namespace racing_pkg

#endif  // RACING_PKG__CAR_NODE_HPP_