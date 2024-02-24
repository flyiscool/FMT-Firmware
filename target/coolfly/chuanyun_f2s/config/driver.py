# Modify this file to decide which drivers are compiled

DRIVERS = [
    # 'imu/icm20600.c',
    # 'imu/icm20689.c',
    # 'imu/bmi055.c',
    'imu/bmi088.c',
    # 'mag/ist8310.c',
    'mag/mmc5983ma.c',
    'mag/qmc5883l.c',
    # 'barometer/ms5611.c',
    'barometer/spl06.c',
    'gps/gps_ubx.c',
    # 'rgb_led/ncp5623c.c',
    'rgb_led/aw2023.c',
    # 'mtd/ramtron.c',
    # 'mtd/w25qxx.c',
    'mtd/spi_tfcard.c',
    # 'vision_flow/pmw3901_fl04.c',
    'vision_flow/tf0850.c',
    # 'airspeed/ms4525.c',
    
    
]

DRIVERS_CPPPATH = []