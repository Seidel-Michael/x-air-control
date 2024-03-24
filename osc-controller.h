// OscController.h

#ifndef OSC_CONTROLLER_H
#define OSC_CONTROLLER_H

#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <map>

struct DeviceInfo {
    char serverVersion[255];
    char name[255];
    char model[255];
    char version[255];
};

typedef void (deviceInfoCallback_t)(DeviceInfo deviceInfo);
typedef void (muteCallback_t)(String channelPath, bool state);
typedef void (faderCallback_t)(String channelPath, float value);

class OscController {
private:
    WiFiUDP udp;
    IPAddress ip;
    unsigned int targetPort;
    unsigned int localPort;

public:
    deviceInfoCallback_t* DeviceInfoCallback = 0x00;
    muteCallback_t* MuteCallback = 0x00;
    faderCallback_t* FaderCallback = 0x00;

    OscController(const IPAddress ip, const unsigned int targetPort, const unsigned int localPort);

    void Connect();
    void Disconnect();


    void SendOscMessage(String command, float value );
    void SendOscMessage(String command, int value );
    void SendOscMessage(String command);
    void ProcessMessages();

private:
    void deviceInfoHandler(OSCMessage &msg);
    void channelMuteHandler(OSCMessage &msg);
    void channelFaderHandler(OSCMessage &msg);
};

#endif // OSC_CONTROLLER_H