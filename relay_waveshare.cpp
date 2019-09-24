#include "wiringPi.h"
#include "relay_waveshare.h"

relay_waveshare::relay_waveshare()
{

    wiringPiSetupGpio();

    pullUpDnControl(relay_channel::DETECT, PUD_UP);
    pullUpDnControl(relay_channel::WORK, PUD_UP);
    pullUpDnControl(relay_channel::THIRD, PUD_UP);

    pinMode(relay_channel::DETECT, OUTPUT);
    pinMode(relay_channel::WORK, OUTPUT);
    pinMode(relay_channel::THIRD, OUTPUT);

    digitalWrite(relay_channel::DETECT, HIGH);         //DETECT^M
    digitalWrite(relay_channel::WORK, HIGH);         //DETECT^M
    digitalWrite(relay_channel::THIRD, HIGH);         //DETECT^M

    //Meter Initialize ^M
    digitalWrite(relay_channel::DETECT, LOW);          //DETECT^M
    delay(500);
    digitalWrite(relay_channel::DETECT, HIGH);         //DETECT^M
    delay(500);
}

relay_waveshare::~relay_waveshare()
{

}

void relay_waveshare::measure_port_reset()
{
    digitalWrite(relay_channel::DETECT, HIGH);         //DETECT^M
    digitalWrite(relay_channel::WORK, HIGH);         //DETECT^M
    digitalWrite(relay_channel::THIRD, HIGH);         //DETECT^M
}

void relay_waveshare::measure_work(relay_waveshare::relay_channel channel, quint8 OnOff)
{
    digitalWrite(channel, OnOff);
}
