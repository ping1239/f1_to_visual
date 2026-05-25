#include "racing_pkg/car_node.hpp"
#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#include <cmath> // 삼각함수에 사용
#include <vector>
#include <fstream>
#include <sstream>
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

namespace racing_pkg {

// 생성자: 클래스 범위 지정 연산자(CarNode::) 추가 및 초기화 매칭
CarNode::CarNode(const rclcpp::NodeOptions & options) 
: Node("car_node", options){
  
  // 파라미터 선언 (csv 파일 경로, 인덱스)
  this->declare_parameter("csv_path", ""); // 경로
  this->declare_parameter("start_index", 0);  // 단위시간마다 업데이트할 인덱스
  
  std::string csv_path = this->get_parameter("csv_path").as_string();
  int start_idx = this->get_parameter("start_index").as_int(); 

  // csv파일 로드 및 파싱
  load_trajectory_csv(csv_path);

  // 시작 인덱스의 실제 s(누적거리) 값으로 초기화
  if (!waypoints_.empty()) {
    if (start_idx < 0 || start_idx >= static_cast<int>(waypoints_.size())) {
      start_idx = 0;
    }
    current_s_ = waypoints_[start_idx].s;
  } else {
    current_s_ = 0.0;
  }

  // 노드 시작 시간
  start_time_ = this->get_clock()->now();

  // TF 브로드캐스터 초기화
  tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

  // 0.01초마다 업데이트되는 타이머 생성
  timer_ = this->create_wall_timer(
    10ms, std::bind(&CarNode::update_position, this));
}

// csv파일 로딩 및 파싱 구현부 매칭
void CarNode::load_trajectory_csv(const std::string & file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    RCLCPP_ERROR(this->get_logger(), "csv파일 열 수 없음: %s", file_path.c_str());
    return;
  }
  std::string line;

  char delimiter = ','; // 기본값은 쉼표(,)로 설정
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty() || line.front() == '#') continue;

    // 첫 번째 유효한 데이터 라인에서 세미콜론이 있으면 ';', 없으면 ',' 사용
    if (line.find(';') != std::string::npos) {
      delimiter = ';';
    } else if (line.find(',') != std::string::npos) {
      delimiter = ',';
    }
    break; 
  }

  // 파일 스트림 상태를 초기화하고 포인터를 다시 맨 처음으로 되돌림
  file.clear();
  file.seekg(0, std::ios::beg);

  // 데이터가 감지된 구분자 기반이므로 한 줄씩 읽어서 파싱
  while (std::getline(file, line)) {
    // Windows 줄바꿈 문자 (\r)가 맨 끝에 있다면 잘라내기
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    // 완전히 빈 줄이나 #으로 시작하는 주석/헤더 라인은 전부 무시
    if (line.empty() || line.front() == '#') continue; 

    std::stringstream ss(line);
    std::string val;
    std::vector<double> row;
    bool row_success = true; // 현재 행의 파싱 성공 여부 플래그

    // 세미콜론(;) 혹은 쉼표(,) 자동 감지된 구분자로 분리
    while (std::getline(ss, val, delimiter)) {
      // 문자열 앞뒤에 쓸데없이 붙은 공백(개행, 탭 등) 잘라내기 (Trim)
      val.erase(0, val.find_first_not_of(" \t"));
      val.erase(val.find_last_not_of(" \t") + 1);

      if (!val.empty()) {
        try {
          row.push_back(std::stod(val)); 
        }
        catch (const std::exception & e) {
          row_success = false;
          break;
        }
      }
    }

    if (row_success && row.size() >= 7) {
      Waypoint wp;
      wp.s = row[0];    // s_m
      wp.x = row[1];    // x_m
      wp.y = row[2];    // y_m
      wp.yaw = row[3];  // psi_rad (차량 방향 각도)
      wp.vx = row[5];   // vx_mps
      wp.ax = row[6];   // ax_mps
      waypoints_.push_back(wp);
    }
  }
  file.close();

  // 데이터가 정상적으로 들어왔는지 검증 로그
  if (waypoints_.empty()) {
    RCLCPP_ERROR(this->get_logger(), "csv 파싱 실패");
  } else {
    RCLCPP_INFO(this->get_logger(), "%zu개 로드 성공 (감지된 구분자: [%c])", waypoints_.size(), delimiter);
  }
}

// 실시간 위치 업데이트 및 연속함수 보간 구현부 매칭
void CarNode::update_position() {
  if (waypoints_.empty()) return;
  
  // 주행 거리가 트랙의 총 길이를 넘어가면 바퀴 수 리셋 및 랩타임 출력
  double max_s = waypoints_.back().s;
  if (current_s_ >= max_s){
    // 트랙을 한 바퀴 돌았으므로 거리 리셋
    current_s_ = std::fmod(current_s_, max_s);
    
    // 바퀴 수 증가
    lap_count_++;

    // 시작 시간 기준 경과 시간(초) 계산
    auto now = this->get_clock()->now();
    double elapsed_time = (now - start_time_).seconds();

    // 터미널에 랩타임 출력
    RCLCPP_INFO(this->get_logger(), "[LAP %d] Time : %.4f 초", lap_count_, elapsed_time);
  }
  if (current_s_ < 0.0) current_s_ = 0.0;

  // 이산 데이터를 연속 데이터로 보간
  // 1. 현재 주행거리 s가 위치한 데이터 사이의 인덱스(i, i+1) 탐색
  size_t i = 0;
  for (size_t idx = 0; idx < waypoints_.size() - 1; ++idx){
    if (waypoints_[idx].s <= current_s_ && current_s_ < waypoints_[idx+1].s){
      i = idx;
      break;
    }
  }

  // 2. 두 정밀 좌표 사이의 비율 계산
  double s0 = waypoints_[i].s;
  double s1 = waypoints_[i+1].s;
  double ratio = 0.0;
  if (s1 > s0){
    ratio = (current_s_ - s0) / (s1 - s0);
  }

  // 3. 보간된 데이터의 정밀 좌표 x, y 계산
  double x = waypoints_[i].x + ratio * (waypoints_[i+1].x - waypoints_[i].x);
  double y = waypoints_[i].y + ratio * (waypoints_[i+1].y - waypoints_[i].y);
  
  // 4. 보간된 데이터의 정밀 헤딩 각도 yaw 계산+ 각도 단절 예방
  double yaw0 = waypoints_[i].yaw;
  double yaw1 = waypoints_[i+1].yaw;
  double dyaw = yaw1 - yaw0;
  while(dyaw < -M_PI) dyaw += 2.0 * M_PI; // PI를 넘어 도는 순간 각도 뒤집힘 방지
  while(dyaw > M_PI) dyaw -= 2.0 * M_PI;
  double yaw = yaw0 + ratio*dyaw;

  // 5. 보간된 데이터의 최적 속도와 가속도 계산
  double vx = waypoints_[i].vx + ratio * (waypoints_[i+1].vx - waypoints_[i].vx);
  double ax = waypoints_[i].ax + ratio * (waypoints_[i+1].ax - waypoints_[i].ax);

  geometry_msgs::msg::TransformStamped t;
  t.header.stamp = this->get_clock()->now();
  t.header.frame_id = "map";

  // TF2의 실행을 위해 슬래시(/) 제거
  std::string ns = this->get_namespace();
  if (!ns.empty() && ns.front() == '/') {
    ns = ns.substr(1);
  }
  
  // child_frame_id 생성
  t.child_frame_id = ns.empty() ? "base_link" : ns + "/base_link";

  // 좌표 및 회전 값 대입
  t.transform.translation.x = x;
  t.transform.translation.y = y;
  t.transform.translation.z = 0.0;

  t.transform.rotation.x = 0.0;
  t.transform.rotation.y = 0.0;
  t.transform.rotation.z = std::sin(yaw / 2.0);
  t.transform.rotation.w = std::cos(yaw / 2.0);

  tf_broadcaster_->sendTransform(t);

  double dt = 0.01;
  current_s_ += (vx*dt + 0.5*ax*dt*dt);
}

} // namespace racing_pkg

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  auto options = rclcpp::NodeOptions();
  rclcpp::spin(std::make_shared<racing_pkg::CarNode>(options));
  rclcpp::shutdown();
  return 0;
}