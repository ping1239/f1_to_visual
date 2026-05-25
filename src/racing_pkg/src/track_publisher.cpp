#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

using namespace std::chrono_literals;

namespace racing_pkg {

class TrackPublisher : public rclcpp::Node {
public:
  TrackPublisher() : Node("track_publisher") {
    // 트랙 csv 경로 파라미터 받기
    this->declare_parameter("track_csv_path", "");
    std::string track_csv_path = this->get_parameter("track_csv_path").as_string();

    // 중앙선, 좌측선, 우측선 퍼블리셔 선언
    centerline_pub_ = this->create_publisher<nav_msgs::msg::Path>("/track_centerline", 10);
    left_bound_pub_ = this->create_publisher<nav_msgs::msg::Path>("/track_left_bound", 10);
    right_bound_pub_ = this->create_publisher<nav_msgs::msg::Path>("/track_right_bound", 10);

    // 트랙 가공 및 로드
    load_and_prepare_track(track_csv_path);

    // rviz2가 1초마다 트랙 라인 무한 방송
    timer_ = this->create_wall_timer(1s, std::bind(&TrackPublisher::publish_track, this));
  }

private:
  void load_and_prepare_track(const std::string & file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
      RCLCPP_ERROR(this->get_logger(), "트랙 파일을 열 수 없음 %s", file_path.c_str());
      return;
    }

    std::string line;
    struct RawPoint { double x; double y; double wl; double wr; };
    std::vector<RawPoint> raw_points;

    // CSV 데이터 파싱
    while (std::getline(file, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty() || line.front() == '#') continue;

      std::stringstream ss(line);
      std::string val;
      std::vector<double> row;

      while (std::getline(ss, val, ',')) {
        val.erase(0, val.find_first_not_of(" \t"));
        val.erase(val.find_last_not_of(" \t") + 1);
        if (!val.empty()) {
          try { row.push_back(std::stod(val)); } catch (...) { break; }
        }
      }
      if (row.size() >= 4) {
        raw_points.push_back({row[0], row[1], row[2], row[3]});
      }
    }
    file.close();

    centerline_path_.header.frame_id = "map";
    left_bound_path_.header.frame_id = "map";
    right_bound_path_.header.frame_id = "map";

    // 진행 방향 벡터의 법선 벡터를 구해서 좌우 경계선 좌표를 확장 계산
    size_t n = raw_points.size();
    for (size_t i = 0; i < n; ++i) {
      double dx, dy;
      if (i < n - 1) {
        dx = raw_points[i+1].x - raw_points[i].x;
        dy = raw_points[i+1].y - raw_points[i].y;
      } else {
        dx = raw_points[0].x - raw_points[i].x;
        dy = raw_points[0].y - raw_points[i].y;
      }

      double len = std::hypot(dx, dy);
      double nx = -dy / len; // 법선 벡터 X
      double ny = dx / len;  // 법선 벡터 Y

      geometry_msgs::msg::PoseStamped p_center, p_left, p_right;
      p_center.header.frame_id = "map";
      p_left.header.frame_id = "map";
      p_right.header.frame_id = "map";

      // 중앙선
      p_center.pose.position.x = raw_points[i].x;
      p_center.pose.position.y = raw_points[i].y;

      // 좌측 경계선 (중앙 + 좌측폭 * 법선)
      p_left.pose.position.x = raw_points[i].x + raw_points[i].wl * nx;
      p_left.pose.position.y = raw_points[i].y + raw_points[i].wl * ny;

      // 우측 경계선 (중앙 - 우측폭 * 법선)
      p_right.pose.position.x = raw_points[i].x - raw_points[i].wr * nx;
      p_right.pose.position.y = raw_points[i].y - raw_points[i].wr * ny;

      centerline_path_.poses.push_back(p_center);
      left_bound_path_.poses.push_back(p_left); 
      right_bound_path_.poses.push_back(p_right);
    }
    RCLCPP_INFO(this->get_logger(), "%zu개의 인덱스로 트랙 그리기 성공", n);
  }

  void publish_track() {
    auto now = this->get_clock()->now();
    centerline_path_.header.stamp = now;
    left_bound_path_.header.stamp = now;
    right_bound_path_.header.stamp = now;

    centerline_pub_->publish(centerline_path_);
    left_bound_pub_->publish(left_bound_path_);
    right_bound_pub_->publish(right_bound_path_);
  }

  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr centerline_pub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr left_bound_pub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr right_bound_pub_;
  nav_msgs::msg::Path centerline_path_;
  nav_msgs::msg::Path left_bound_path_;
  nav_msgs::msg::Path right_bound_path_;
  rclcpp::TimerBase::SharedPtr timer_;
};

} // namespace racing_pkg

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<racing_pkg::TrackPublisher>());
  rclcpp::shutdown();
  return 0;
}