
#ifndef MYTHREAD_H
#define PROBEREQSNIFFER_H
/*
 * Class for sniffing IEEE 802.11 Frames
 * Pre-Condition : This class sniff only Probe-Req(0x04) in defferent thread
 *                 and find STA MAC (random MAC is filtered).
 *                 using hash map, this class managed STA MAC(key) and count(value)
 * Post-Condition : if there is a any STA MAC, which is MAX count then
 *                  emit SIGNAL(packetCaptured) to main window with STA MAC
 *
 */
#include <QtCore>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <tins/tins.h>

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;
using namespace Tins;

enum TARGET{
    STATION = 1,
    AP = 2
};

enum SIGTYPE{
    NEWDEVICE = 1,
    MAXCHANGE = 2,
    NONE = 3
};

class ProbeReqSniffer : public QThread
{
    Q_OBJECT
public:
    explicit ProbeReqSniffer(QObject *parent = 0);

    //@Overriding
    void run();
    SIGTYPE isNewAddress(HWAddress<6>);

    void setStop(bool);
    bool getStop();

    void setInf(string);
    string getInf();

    void setSSI_MIN(int);
    int getSSI_MIN();

    void setTarget(TARGET);
    TARGET getTarget();

    int getMaxCount();

    int getStaCount(HWAddress<6>);

    void refreshAVG();
    void clearMap();

    bool isRandomized(Dot11ProbeRequest *);
    bool isAppleVendor(Dot11ProbeRequest *);

signals:
    void MaxDeviceChanged(QString);
    void NewDevice(QString);

private:
    typedef Dot11::address_type address_type;
    typedef unordered_map<address_type, int> sta_map_type;

    bool Stop;
    string inf;
    SnifferConfiguration *config;

    HWAddress<3> apple_oui;
    HWAddress<3> broadcom_oui;

    sta_map_type staMap;
    address_type sta;
    TARGET target = STATION;

    int maxCount = 0;
    int avgCount = 0;
    int SSI_MIN = -35;

};

#endif // PROBEREQSNIFFER_H
