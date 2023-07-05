# Modify this file to decide which drivers are compiled

DRIVERS = [
    'imu/icm20689.c',
    'imu/icm20948.c',
    'imu/bmi055.c',
    'mag/ist8310.c',
    'barometer/ms5611.c',
    'gps/gps_m8n.c',
    'gps/gps_dronecan.c',
    'rgb_led/rgb_dronecan.c',
    'mtd/ramtron.c',
]

DRIVERS_CPPPATH = []