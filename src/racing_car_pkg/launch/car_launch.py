import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # 패키지 경로 설정
    package_name = 'racing_car_pkg'
    pkg_dir = get_package_share_directory(package_name)

    # URDF 파일 경로 찾기
    urdf_file_path = os.path.join(pkg_dir, 'urdf', 'car.urdf')

    # URDF 파일 내용 읽기
    with open(urdf_file_path, 'r') as infp:
        robot_desc = infp.read()

    # RViz 설정 파일 경로
    rviz_config_path = os.path.join(pkg_dir, 'config', 'car_view.rviz')

    return LaunchDescription([
        # [노드 1] 로봇 상태 퍼블리셔: URDF를 시스템에 등록
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            parameters=[{'robot_description': robot_desc}]
        ),
        
        # [노드 2] RViz2 실행
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen'
        ),

        # [노드 3] Static TF: map과 base_link를 연결 (상자가 좌표계에 고정되게 함)
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0', '0', '0', '0', '0', '0', 'map', 'base_link']
        )
    ])