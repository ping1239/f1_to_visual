import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # 패키지 경로 설정
    package_name = 'racing_car_pkg'
    pkg_dir = get_package_share_directory(package_name)
    print(pkg_dir)

    # URDF 파일 경로 찾기
    urdf_file_path = os.path.join(pkg_dir, 'urdf', 'car.urdf')

    # URDF 파일 내용 읽기
    with open(urdf_file_path, 'r') as infp:
        robot_desc = infp.read()

    # RViz 설정 파일 경로
    rviz_config_path = os.path.join(pkg_dir, 'config', 'car_view.rviz')

    return LaunchDescription([
        # [공통] RViz2 실행
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen'
        ),

        # ================= [CAR 1] =================
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace='car1',
            parameters=[{
                'robot_description': robot_desc,
                'frame_prefix': 'car1/' # 좌표계 앞에 'car1/'을 붙임
            }]
        ),
        Node( # car1의 위치: (x=0, y=0)
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0', '0', '0', '0', '0', '0', 'map', 'car1/base_link']
        ),

        # ================= [CAR 2] =================
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace='car2',
            parameters=[{
                'robot_description': robot_desc,
                'frame_prefix': 'car2/' # 좌표계 앞에 'car2/'을 붙임
            }]
        ),
        Node( # car2의 위치: y축으로 1미터 이동 (x=0, y=1.0) -> 안 겹치게!
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0', '1.0', '0', '0', '0', '0', 'map', 'car2/base_link']
        )
    ])