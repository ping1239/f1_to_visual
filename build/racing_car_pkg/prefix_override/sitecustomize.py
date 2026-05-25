import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/seojun/바탕화면/ros2_ws/install/racing_car_pkg'
