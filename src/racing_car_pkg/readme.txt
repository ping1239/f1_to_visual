<!-- 매번 터미널을 열때마다 /ros2_ws에 
source install/setup.bash
을 입력하고 시작해야함x -->

터미널 1:
    ros2 run my_first_robot rotate_head

터미널 2:x
    ros2 topic echo /joint_states
- 숫자가 빠르게 올라간다면 잘 연결된거임


-----------------------------------------------
터미널 1: (로봇 정보 게시)
ros2 run robot_state_publisher robot_state_publisher --ros-args -p robot_description:="$(cat src/my_first_robot/urdf/box_robot.urdf)"
터미널 2: (방금 만든 C++ 노드에 각도 데이터 전송)
ros2 run my_first_robot rotate_head
터미널 3: (시각화)
rviz2
------------------------------------------------
ros2_ws/                              # 1. 작업 공간 최상위 (Workspace Root)
├── build/                           # 빌드 시 생성되는 중간 파일 (자동 생성)
├── install/                         # 빌드 후 실행 파일 및 설정이 모이는 곳 (자동 생성)
├── log/                             # 빌드/실행 로그 (자동 생성)
└── src/                          # 2. 소스 코드 폴더 (우리가 직접 작업하는 곳)
    └── my_rviz_pkg/              # 3. 사용자 패키지 (Python 기준)
        ├── package.xml              # 패키지 메타데이터 (의존성 등)
        ├── setup.py                 # 빌드 및 설치 스크립트 (중요!)
        ├── setup.cfg                # 설치 경로 설정
        ├── resource/                # 패키지 인덱스 파일
        │   └── my_rviz_pkg
        ├── my_rviz_pkg/      # 실제 Python 노드 코드가 들어가는 곳
        │   ├── __init__.py
        │   └── my_node.py    # (선택) 센서 데이터를 발행하는 노드 등
        ├── launch/           # 4. 실행 파일 (Launch files)
        │   ├── racing_launch.py # RViz2를 자동 실행해주는 스크립트
        │   └── car_launch.py    # (신규: 모델 로딩용)
        ├── config/           # 5. 설정 파일 (Configuration)
        │   ├── my_view.rviz  # RViz2 레이아웃 저장 파일
        │   └── car_view.rviz # 신규: 자동차가 보이는 화면 설정
        ├── urdf/             # 6. 로봇 모델 파일 (선택 사항)
        │   └── car.urdf    # RViz에서 시각화할 자동차 모델 정의
        └── test/             # 테스트 코드 폴더
------------------------------------------------------------
패키지를 새로 만들고 싶을 때
ros2 pkg create --build-type ament_python racing_car_pkg
: racing_car_pkg라는 패키지 만들기

colcon build --symlink-install
: 빌드 진행 (패키지를 만들고 등록하는 과정)
----------------------------------------------------------------------------
환경설정이 다 끝났으므로 이제 rviz2를 실행하고 싶을 때
source install/setup.bash
ros2 launch racing_car_pkg racing_launch.py (racing_launch.py를 런치)
ros2 launch racing_car_pkg car_launch.py (car_launch.py를 런치)
ros2 launch racing_car_pkg multi_car_launch.py (multi_car_launch.py를 런치)
이것만 치면 rviz2 바로 실행

안될 경우에는
rm -rf build/ install/ log/
로 이전 로그 기록 삭제 후 다시 빌드 후 실행