from launch import LaunchDescription
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 내 패키지의 설치 경로 찾기
    pkg_dir = get_package_share_directory('racing_car_pkg')
    # rviz 파일 경로 지정 (나중에 저장할 파일 이름)
    rviz_config_path = os.path.join(pkg_dir, 'config', 'car_view.rviz')

    return LaunchDescription([
        # 로봇 상태 퍼블리셔: URDF를 시스템에 등록
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_desc}]
        ), 
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
        ), 
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0', '0', '0', '0', '0', '0', 'map', 'base_link']
        )
    ])