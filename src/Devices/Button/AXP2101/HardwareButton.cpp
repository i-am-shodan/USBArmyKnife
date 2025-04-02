#ifdef LILYGO_T_WATCH_S3
#include "../HardwareButton.h"
#include "../../../Debug/Logging.h"

#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#define TAG "Button"

static XPowersPMU power;
static bool pmu_flag = false;

namespace Devices
{
  HardwareButton Button;
}

void setFlag(void)
{
  pmu_flag = true;
}

HardwareButton::HardwareButton()
{
}

void HardwareButton::begin(Preferences &prefs)
{
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

    // Clear all interrupt flags
    if (!power.enableIRQ(XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ))
    {
      Debug::Log.error(TAG, "Could not enable IRQs");
    }

    attachInterrupt(pmu_irq_pin, setFlag, FALLING);

  } while (false);
}

void HardwareButton::loop(Preferences &prefs)
{
  if (pmu_flag)
  {
    pmu_flag = false;
    (void)power.getIrqStatus();
    Debug::Log.error(TAG, "Got interrupted");

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