디렉토리 구조
ros2_ws/                        # 워크스페이스 최상위 폴더
├── build/                      # 컴파일된 목적 파일(Object files) 생성 (자동)
├── install/                    # 최종 실행 파일 및 설정 파일 복사 (자동)
├── log/                        # 빌드 및 실행 로그 (자동)
└── src/
    └── racing_pkg/         # 사용자가 생성한 C++ 패키지 폴더
        ├── CMakeLists.txt      # 빌드 규칙 및 폴더 설치(Install) 경로 설정 파일
        ├── package.xml         # 패키지 정보 및 의존성 명세서
        │
        ├── include/            # 1. C++ 헤더 파일 폴더 (.h, .hpp)
        │   └── racing_pkg/
        │       └── my_node.hpp # 클래스 선언부
        │
        ├── src/                # 2. C++ 소스 코드 폴더 (.cpp)
        │   └── car_node.cpp     # 실제 동작 로직
        │
        ├── launch/             # 3. 런치 파일 폴더 (C++ 패키지도 주로 Python으로 작성)
        │   └── multi_car_launch.py   # 여러 노드 동시 실행
        │
        ├── urdf/               # 4. 로봇 모델 폴더
        │   └── car.urdf        # XML 기반의 로봇 형상 정의
        │
        │── config/             # 5. 설정 파일 폴더
        │    └── car_view.rviz   # RViz2 레이아웃 및 화면 세팅 저장 파일
        │
        │    
        ├── path_ideal/         # TUMFTM으로 생성한 경로 csv  
        │   └── traj_<트랙명>_TUM.csv
        │
        ├── path_ML/            # 머신러닝 보정 경로 csv
        │   └── traj_<트랙명>_ML.csv
        │
        ├── path_real/          # openf1 실제 경로
        │   └── traj_<트랙명>_realistic.csv
        │
        └── tracks/             # 트랙 데이터 csv
             └── <트랙명>.csv
 
----------------------------------------------------------------------------------
# racing_pkg (C++ 기반 ROS 2 패키지)

## 개요
- rclcpp(C++)를 사용하여 자동차 노드를 구현

## 실행 방법
1. 빌드:
cd ~/바탕화면/ros2_ws
colcon build --symlink-install
source install/setup.bash

2. 런치 실행:
ros2 launch racing_pkg multi_car_launch.py

3. 실행 후:
Add -> Robotmodel : Displays에 Robotmodel생성
Robotmodel -> Description Topic에 /car1/robot_description, /car2/robot_description, /car3/robot_description 입력 혹은 선택
TF Prefix에 car1, car2, car3 입력

4. 기타
에러 시 기존 빌드 제거: 
rm -rf build/ install/ log/



## 주요 구성
- urdf/: 자동차 모델 정의
- src/: 자동차 제어 노드 구현 (C++)
- launch/: 멀티 로봇 실행 스크립트 (Python)