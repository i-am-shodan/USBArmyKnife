#ifdef LILYGO_T_WATCH_S3
#include "TWatchS3.h"
#include "../../../Debug/Logging.h"
#include "../../Button/HardwareButton.h"
#define TAG "Board"

#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

static XPowersPMU power;
static bool pmu_flag = false;

namespace Devices
{
    TWatchS3 Board;
}

TWatchS3::TWatchS3()
{
}

void setFlag(void)
{
    pmu_flag = true;
}

void TWatchS3::begin(Preferences &prefs)
{
    ESP32BoardSupport::begin(prefs);

    const uint8_t i2c_sda = BOARD_I2C_SDA;
    const uint8_t i2c_scl = BOARD_I2C_SCL;
    const uint8_t pmu_irq_pin = BOARD_PMU_INT;

    do
    {
        // Force add pull-up
        pinMode(pmu_irq_pin, INPUT_PULLUP);

        if (!Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL))
        {
            Debug::Log.error(TAG, "Error setting up i2c");
            break;
        }

        if (!power.begin(Wire, AXP2101_SLAVE_ADDRESS, i2c_sda, i2c_scl))
        {
            Debug::Log.error(TAG, "Could not find AXP2101 device");
            break;
        }
        Debug::Log.info(TAG, "Found AXP2101 device");

        power.clearIrqStatus();

        // Disable all interrupts
        if (!power.disableIRQ(XPOWERS_AXP2101_ALL_IRQ))
        {
            Debug::Log.error(TAG, "Could not disable IRQs");
        }

        // Set the minimum common working voltage of the PMU VBUS input,
        // below this value will turn off the PMU
        power.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

        // Set the maximum current of the PMU VBUS input,
        // higher than this value will turn off the PMU
        power.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_900MA);

        // Set VSY off voltage as 2600mV , Adjustment range 2600mV ~ 3300mV
        power.setSysPowerDownVoltage(2600);

        // ! ESP32S3 VDD, Don't change
        // setDC1Voltage(3300);

        //! RTC VBAT , Don't change
        power.setALDO1Voltage(3300);

        //! TFT BACKLIGHT VDD , Don't change
        power.setALDO2Voltage(3300);

        //! Screen touch VDD , Don't change
        power.setALDO3Voltage(3300);

        //! Radio VDD , Don't change
        power.setALDO4Voltage(3300);

        //! DRV2605 enable
        power.setBLDO2Voltage(3300);

        //! GPS Power
        //power.setDC3Voltage(3300);
        //power.enableDC3();

        //! No use
        power.disableDC2();
        power.disableDC3(); // gps
        power.disableDC4();
        power.disableDC5();
        power.disableBLDO1();
        power.disableCPUSLDO();
        power.disableDLDO1();
        power.disableDLDO2();

        power.enableALDO1(); //! RTC VBAT
        power.enableALDO2(); //! TFT BACKLIGHT   VDD
        power.enableALDO3(); //! Screen touch VDD
        //power.enableALDO4(); //! Radio VDD
        //power.enableBLDO2(); //! haptics drv2605 enable

        // Set the time of pressing the button to turn off
        power.setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);

        // Set the button power-on press time
        power.setPowerKeyPressOnTime(XPOWERS_POWERON_128MS);

        // It is necessary to disable the detection function of the TS pin on the board
        // without the battery temperature detection function, otherwise it will cause abnormal charging
        power.disableTSPinMeasure();

        // Enable internal ADC detection
        power.enableBattDetection();
        power.enableVbusVoltageMeasure();
        power.enableBattVoltageMeasure();
        power.enableSystemVoltageMeasure();

        // t-watch no chg led
        power.setChargingLedMode(XPOWERS_CHG_LED_OFF);

        // Set the precharge charging current
        power.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
        // Set constant current charge current limit
        power.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_300MA);
        // Set stop charging termination current
        power.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

        // Set charge cut-off voltage
        power.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V35);

        // Set RTC Battery voltage to 3.3V
        power.setButtonBatteryChargeVoltage(3300);

        power.enableButtonBatteryCharge();

        // Clear all interrupt flags
        if (!power.enableIRQ(XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ))
        {
            Debug::Log.error(TAG, "Could not enable IRQs");
        }

        attachInterrupt(pmu_irq_pin, setFlag, FALLING);
    } while (false);
}

void TWatchS3::loop(Preferences &prefs)
{
    ESP32BoardSupport::loop(prefs);

    if (pmu_flag)
    {
        pmu_flag = false;
        (void)power.getIrqStatus();

        if (power.isPekeyShortPressIrq())
        {
            Devices::Button.setButtonPressState(true, false);
        }
        else if (power.isPekeyLongPressIrq())
        {
            Devices::Button.setButtonPressState(true, true);
        }
        power.clearIrqStatus();
    }
}

#endif