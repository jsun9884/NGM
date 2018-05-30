#ifndef LSM6DS3_H
#define LSM6DS3_H

//
// Private Declarations
//

// LSM6DS3_ACC_SENSITIVITY Accelero sensitivity values based on selected full scale
#define LSM6DS3_ACC_SENSITIVITY_FOR_FS_2G   0.061  /**< Sensitivity value for 2 g full scale [mg/LSB] */
#define LSM6DS3_ACC_SENSITIVITY_FOR_FS_4G   0.122  /**< Sensitivity value for 4 g full scale [mg/LSB] */
#define LSM6DS3_ACC_SENSITIVITY_FOR_FS_8G   0.244  /**< Sensitivity value for 8 g full scale [mg/LSB] */
#define LSM6DS3_ACC_SENSITIVITY_FOR_FS_16G  0.488  /**< Sensitivity value for 16 g full scale [mg/LSB] */

// LSM6DS3_GYRO_SENSITIVITY Gyro sensitivity values based on selected full scale
#define LSM6DS3_GYRO_SENSITIVITY_FOR_FS_125DPS   04.375  /**< Sensitivity value for 125 dps full scale [mdps/LSB] */
#define LSM6DS3_GYRO_SENSITIVITY_FOR_FS_245DPS   08.750  /**< Sensitivity value for 245 dps full scale [mdps/LSB] */
#define LSM6DS3_GYRO_SENSITIVITY_FOR_FS_500DPS   17.500  /**< Sensitivity value for 500 dps full scale [mdps/LSB] */
#define LSM6DS3_GYRO_SENSITIVITY_FOR_FS_1000DPS  35.000  /**< Sensitivity value for 1000 dps full scale [mdps/LSB] */
#define LSM6DS3_GYRO_SENSITIVITY_FOR_FS_2000DPS  70.000  /**< Sensitivity value for 2000 dps full scale [mdps/LSB] */

// Max G force readable.  Can be: 2, 4, 8, 16
#define ACCEL_Range             2

// SPI Communication Retries
#define IMU_RETRIES		10

#define IMU_TIME_DIVIDER	1000.0		// for IMU 25uS timestamp: 40000.0f

/* Registers addresses */
typedef enum _LSM6DS3_REG
{
        LSM6DS3_REG_FUNC_CFG_ACCESS = 0x01,
        LSM6DS3_REG_FIFO_CTRL5      = 0x0A,
        LSM6DS3_REG_ORIENT_CFG_G  	= 0x0B,
        LSM6DS3_REG_WHO_AM_I        = 0x0F,
        LSM6DS3_REG_CTRL1_XL        = 0x10,
        LSM6DS3_REG_CTRL2_G         = 0x11,
        LSM6DS3_REG_CTRL3_C         = 0x12,
        LSM6DS3_REG_CTRL7_G         = 0x16,
        LSM6DS3_REG_CTRL8_XL        = 0x17,

        LSM6DS3_REG_OUTX_G  		= 0x22,
        LSM6DS3_REG_OUTY_G  		= 0x24,
        LSM6DS3_REG_OUTZ_G  		= 0x26,

        LSM6DS3_REG_OUTX_XL  		= 0x28,
        LSM6DS3_REG_OUTY_XL  		= 0x2A,
        LSM6DS3_REG_OUTZ_XL  		= 0x2C,

        LSM6DS3_REG_TIMESTAMP0      = 0x40,
        LSM6DS3_REG_TIMESTAMP1      = 0x41,
        LSM6DS3_REG_TIMESTAMP2      = 0x42,
        LSM6DS3_REG_TAP_CFG         = 0x58,
        LSM6DS3_REG_WAKE_UP_DUR     = 0x5C,

        LSM6DS3_REG_READ_FLAG		= 0x80,
} LSM6DS3_REG;

/* Output Data Rate For CTRL1_XL and CTRL1_G registers */
typedef enum _LSM6DS3_ODR
{
        LSM6DS3_ODR_POWER_DOWN = 0x00,
        LSM6DS3_ODR_13_HZ      = 0x10,
        LSM6DS3_ODR_26_HZ      = 0x20,
        LSM6DS3_ODR_52_HZ      = 0x30,
        LSM6DS3_ODR_104_HZ     = 0x40,
        LSM6DS3_ODR_208_HZ     = 0x50,
        LSM6DS3_ODR_416_HZ     = 0x60,
        LSM6DS3_ODR_833_HZ     = 0x70,
        LSM6DS3_ODR_1660_HZ    = 0x80,
} LSM6DS3_ODR;

/* CTRL3_C Register flags */
typedef enum _LSM6DS3_CTRL3_C_FLAGS
{
        LSM6DS3_CTRL3_C_BOOT      = 0x80,	// Reboot memory content. Default value: 0
                                                                                // 0: normal mode; 1: reboot memory content
        LSM6DS3_CTRL3_C_BDU       = 0x40,	// Block Data Update. Default value: 0
                                                                                // 0: continuous update; 1: output registers not updated until MSB and LSB have been read
        LSM6DS3_CTRL3_C_H_LACTIVE = 0x20,	// Interrupt activation level. Default value: 0
                                                                                // 0: interrupt output pads active high; 1: interrupt output pads active low
        LSM6DS3_CTRL3_C_PP_OD     = 0x10,	// Push-pull/open-drain selection on INT1 and INT2 pads. Default value: 0
                                                                                // 0: push-pull mode; 1: open-drain mode
        LSM6DS3_CTRL3_C_SIM       = 0x08,	// Serial Interface Mode selection. Default value: 0
                                                                                // 0: 4-wire interface; 1: 3-wire interface
        LSM6DS3_CTRL3_C_IF_INC    = 0x04,	// Register address automatically incremented during a multiple byte access with a
                                                                                // serial interface (I2C or SPI). Default value: 1
                                                                                // 0: disabled; 1: enabled
        LSM6DS3_CTRL3_C_BLE       = 0x02,	// Big/Little Endian Data selection. Default value 0
                                                                                // 0: data LSB @ lower address; 1: data MSB @ lower address
        LSM6DS3_CTRL3_C_SW_RESET  = 0x01,	// Software reset. This bit is cleared by hardware after next flash boot. Default value: 0
                                                                                // 0: normal mode; 1: reset device
        LSM6DS3_CTRL3_C_DEFAULT   = LSM6DS3_CTRL3_C_IF_INC
} LSM6DS3_CTRL3_C_FLAGS;

/* WAKE_UP_DUR Register flags */
typedef enum _LSM6DS3_WAKE_UP_DUR_FLAGS
{
        LSM6DS3_WAKE_UP_DUR_TIMER_HR = 0x10,	// Time stamp register resolution setting. Default value: 0
                                                                                        // 0: 1LSB = 6.4 mS; 1: 1LSB = 25 uS
} LSM6DS3_WAKE_UP_DUR_FLAGS;

/* TAP_CFG Register flags */
typedef enum _LSM6DS3_TAP_CFG_FLAGS
{
        LSM6DS3_TAP_CFG_TIMER_EN = 0x80,		// Time stamp count enable, output data are collected in TIMESTAMP0_REG (40h),
                                                                                        // TIMESTAMP1_REG (41h), TIMESTAMP2_REG (42h)register. Default: 0
                                                                                        // 0: time stamp count disabled; 1: time stamp count enabled
} LSM6DS3_TAP_CFG_FLAGS;


#endif // LSM6DS3_H
