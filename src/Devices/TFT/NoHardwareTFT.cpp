#ifdef NO_TFT
#include "HardwareTFT.h"

namespace Devices
{
    HardwareTFT TFT;
}

void HardwareTFT::display(const int &x, const int &y, const std::string &str)
{
}

void HardwareTFT::clearScreen()
{
}

void HardwareTFT::powerOff()
{
}

void HardwareTFT::powerOn()
{
}

void HardwareTFT::displayPng(HardwareStorage &storage, const std::string &filename)
{
}

HardwareTFT::HardwareTFT()
{
}

void HardwareTFT::loop(Preferences &prefs)
{
}

void HardwareTFT::begin(Preferences &prefs)
{
}
#endif