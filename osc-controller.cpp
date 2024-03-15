#include "osc-controller.h"

OscController::OscController(const IPAddress ip, const unsigned int targetPort, const unsigned int localPort)
{
    this->ip = ip;
    this->targetPort = targetPort;
    this->localPort = localPort;
}

void OscController::Connect()
{
    udp.begin(localPort);
}

void OscController::Disconnect()
{
    udp.stop();
}

void OscController::SendOscMessage(String command, float value)
{
    OSCMessage msg(command.c_str());
    msg.add(value);

    udp.beginPacket(ip, targetPort);
    msg.send(udp);
    udp.endPacket();
}

void OscController::SendOscMessage(String command, int value)
{
    OSCMessage msg(command.c_str());
    msg.add(value);

    udp.beginPacket(ip, targetPort);
    msg.send(udp);
    udp.endPacket();
}

void OscController::SendOscMessage(String command)
{
    OSCMessage msg(command.c_str());

    udp.beginPacket(ip, targetPort);
    msg.send(udp);
    udp.endPacket();
}

void OscController::ProcessMessages()
{
    // Processes incoming OSC messages
    OSCMessage msg;
    int size = udp.parsePacket();

    if (size > 0)
    {
        while (size--)
        {
            msg.fill(udp.read());
        }
        if (!msg.hasError())
        {
            if (msg.fullMatch("/info"))
            {
                deviceInfoHandler(msg);
            }
            else if (String(msg.getAddress()).endsWith("/mix/on"))
            {
                channelMuteHandler(msg);
            }
            
        }
        else
        {
            OSCErrorCode error = msg.getError();
            Serial.print("error: ");
            Serial.println(error);
        }
    }
}

void OscController::RegisterMuteCallback(String channelPath, std::function<void(bool)> callback)
{
    MuteCallbacks[channelPath] = callback;
}

void OscController::deviceInfoHandler(OSCMessage &msg)
{
    if (msg.size() == 4)
    {
        DeviceInfo deviceInfo;

        msg.getString(0, deviceInfo.serverVersion, 255);
        msg.getString(1, deviceInfo.name, 255);
        msg.getString(2, deviceInfo.model, 255);
        msg.getString(3, deviceInfo.version, 255);

        if (DeviceInfoCallback != 0x00)
        {
            DeviceInfoCallback(deviceInfo);
        }
    }
}

void OscController::channelMuteHandler(OSCMessage &msg)
{
    boolean state = msg.getInt(0) == 0;

    if (MuteCallbacks.find(msg.getAddress()) != MuteCallbacks.end())
    {
        MuteCallbacks[msg.getAddress()](state);
    }
}
