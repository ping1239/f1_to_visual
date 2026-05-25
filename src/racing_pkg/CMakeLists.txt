cmake_minimum_required(VERSION 3.8)
project(racing_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 의존성 패키지들
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

# 실행파일 생성
add_executable(car_node src/car_node.cpp)
add_executable(track_publisher src/track_publisher.cpp)

# 패키지 내부의 include 폴더
target_include_directories(car_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(track_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 의존성 패키지 연결 (car_node)
ament_target_dependencies(car_node 
  rclcpp 
  tf2_ros 
  geometry_msgs
)

# 의존성 패키지 연결 (track_publisher)
ament_target_dependencies(track_publisher
  rclcpp
  nav_msgs
)

# 실행 파일을 설치 경로로 복사
install(TARGETS
  car_node
  track_publisher
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  urdf
  config
  path_ideal
  path_ML
  path_real
  tracks  
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()