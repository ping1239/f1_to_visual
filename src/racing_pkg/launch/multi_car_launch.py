import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
# 파라미터 설명
# csv_path: 최적 주행 궤적 파일 경로 (trajectory.csv)
# track_csv_path: 레이싱 서킷 구조 파일 경로 (handling_track.csv)
# start_index: 차량의 초기 시작 웨이포인트 인덱스

def generate_launch_description():
    # 패키지 경로 설정
    package_name = 'racing_pkg'
    pkg_dir = get_package_share_directory(package_name)
    # print(pkg_dir)

    # URDF 파일 경로 찾기
    TUM_urdf_path = os.path.join(pkg_dir, 'urdf', 'TUM_path_car.urdf')
    real_urdf_path = os.path.join(pkg_dir, 'urdf', 'real_path_car.urdf')
    ML_urdf_path = os.path.join(pkg_dir, 'urdf', 'ML_path_car.urdf')

    # URDF 파일 내용 읽기
    with open(TUM_urdf_path, 'r') as infp:
        car1_desc = infp.read()
    with open(real_urdf_path, 'r') as infp:
        car2_desc = infp.read()
    with open(ML_urdf_path, 'r') as infp:
        car3_desc = infp.read()

    # RViz 설정 파일 경로
    rviz_config_path = os.path.join(pkg_dir, 'config', 'car_view.rviz')
    
    # 패키지 내부의 csv 경로
    # 이상 경로
    ideal_path = os.path.join(pkg_dir, 'path_ideal', 'traj_Monza_TUM.csv')            # TUMFTM 이상적 경로

    # 실제 경로
    real_path = os.path.join(pkg_dir, 'path_real', 'traj_Monza_realistic.csv')      # 실제 경로 (red)
    # real_path = os.path.join(pkg_dir, 'path_real', 'traj_Monza_lab10.csv')      # 실제 경로 (red) - 욱희형 실제 경로

    # ML 보정 경로
    ML_path = os.path.join(pkg_dir, 'path_ML', 'traj_Monza_ML.csv')                 # ML보정 경로 (green)
    # ML_path = os.path.join(pkg_dir, 'path_ML', 'traj_Monza.csv')                 # ML보정 경로 (green) - 욱희형 보정 경로


    track_file_path = os.path.join(pkg_dir, 'tracks', 'Monza.csv')                  # 트랙

    return LaunchDescription([
        # RViz2 실행
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen'
        ),
        # ================= [TRACK] =================
        Node(
            package='racing_pkg',
            executable='track_publisher',
            name='track_publisher',
            parameters=[{'track_csv_path': track_file_path}]
        ),

        # ================= [CAR 1] =================
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace='car1',
            parameters=[{'robot_description': car1_desc, 'frame_prefix': 'car1/'}]
        ),
        Node(
            package='racing_pkg', 
            executable='car_node',
            namespace='car1', 
            parameters=[{'csv_path': ideal_path, 'start_index': 0}]   # TUMFTM 이상적 경로 
        ),

        # ================= [CAR 2] =================
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace='car2',
            parameters=[{'robot_description': car2_desc, 'frame_prefix': 'car2/'}]
        ),
        Node(
            package='racing_pkg',
            executable='car_node', 
            namespace='car2', 
            parameters=[{'csv_path': real_path, 'start_index': 0}]   # 실제 경로
        ),
        # ================= [CAR 3] =================
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace='car3',
            parameters=[{'robot_description': car3_desc, 'frame_prefix': 'car3/'}]
        ),
        Node(
            package='racing_pkg',
            executable='car_node',
            namespace='car3', 
            parameters=[{'csv_path': ML_path, 'start_index': 0}]   # ML보정 경로 
        ),
    ])