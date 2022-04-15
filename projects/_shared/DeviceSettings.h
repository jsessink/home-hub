
#ifndef DeviceSettings_H
#define DeviceSettings_H

class DeviceSettings
{
  public:
    String getActiveDeviceName()
    {
      switch (activeDevice)
      {
        case HUB:
          return "Home Hub";
        case BREEZE_ENTRY_CLOSET:
          return "Breezeway Entry Closet";
        default:
          return "";
      }
    }

    enum Device
    {
      HUB,
      BREEZE_ENTRY_CLOSET
    };

    // No idea how else to do this when uploading
    static const Device activeDevice =
        // HUB;
        BREEZE_ENTRY_CLOSET;
};

#endif
