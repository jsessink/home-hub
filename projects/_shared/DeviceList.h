#ifndef DeviceList_H
#define DeviceList_H

namespace DeviceList
{
  enum Device
  {
    HUB,
    BREEZE_ENTRY_CLOSET
  };

  // No idea how else to do this when uploading
  const Device activeDevice = 
      // HUB;
      BREEZE_ENTRY_CLOSET;
}

#endif
