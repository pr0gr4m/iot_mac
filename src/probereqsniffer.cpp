#include "probereqsniffer.h"

ProbeReqSniffer::ProbeReqSniffer(QObject *parent) : QThread(parent)
{
    config = new SnifferConfiguration;
    config->set_promisc_mode(true);
    config->set_rfmon(true);
    config->set_timeout(1);

    apple_oui = HWAddress<3>("00:17:f2");
    broadcom_oui = HWAddress<3>("00:10:18");
}

/*
 * @Overriding
 *
 * Pre-Condition : QThread.start()
 *                 rebooting interface and convert moniter mode,
 *                 and sniff Probe-Req[fitering Randomized MAC].
 *                 if there is any frame then, update map
 *
 * Post-Condition : if some MAC's value in map greater then maxCount then, emit SIGNAL to MainWindow
 *
 */

void ProbeReqSniffer::run(){
    std::system(("ifconfig " + inf + " down").c_str());
    std::system(("ifconfig " + inf + " up").c_str());

    Stop = false;

    if(target == STATION)
        config->set_filter("type mgt subtype probe-req || type mgt subtype auth || type mgt subtype assoc-req || type data");
    else
        config->set_filter("type mgt subtype beacon  || type mgt subtype auth || type mgt subtype assoc-resp || type data");

    Sniffer sniffer(inf, *config);
    RadioTap *radio;
    Dot11 *dot11;
    Dot11ProbeRequest *req;
    Dot11Beacon *beacon;
    Dot11Data *data;

    HWAddress<6> addr;

    while(!Stop){
        Packet packet = sniffer.next_packet();

        if(packet.pdu() == 0)
            continue;

        radio = packet.pdu()->find_pdu<RadioTap>();

        if(static_cast<int>(radio->dbm_signal()) < SSI_MIN)
            continue;

        dot11 = packet.pdu()->find_pdu<Dot11>();

        if(dot11->type() == Dot11::MANAGEMENT){

            if(dot11->subtype() == Dot11::AUTH)
                addr = dot11->find_pdu<Dot11Authentication>()->addr2();

            else if(target == STATION){
                if(dot11->subtype() == Dot11::PROBE_REQ){
                    req = packet.pdu()->find_pdu<Dot11ProbeRequest>();
                    //if(isAppleVendor(req)){
                    if(isRandomized(req)){
                        continue;
                    }
                    //}
                    addr = req->addr2();
                }else if(dot11->subtype() == Dot11::ASSOC_REQ){
                    addr = dot11->find_pdu<Dot11AssocRequest>()->addr2();
                }else{
                    continue;
                }
            }else if(target == AP){
                if(dot11->subtype() == Dot11::BEACON){
                    beacon = dot11->find_pdu<Dot11Beacon>();

                    if(beacon->ssid() != "" && beacon->ssid().at(0) == 0x00)
                        continue;

                    addr = beacon->addr3();
                }else if(dot11->subtype() == Dot11::ASSOC_RESP){
                    addr = dot11->find_pdu<Dot11AssocResponse>()->addr2();
                }else{
                    continue;
                }
            }
        }else if(dot11->type() == Dot11::DATA){
            data = dot11->find_pdu<Dot11Data>();

            if(dot11->subtype() != Dot11::QOS_DATA_DATA && dot11->subtype() != Dot11::QOS_DATA_NULL
                    && dot11->subtype() != Dot11::DATA_DATA && dot11->subtype() != Dot11::DATA_NULL)
                continue;     

            if(target == STATION && (data->to_ds() && !data->from_ds())){
            //To_DS
            }else if(target == AP && (!data->to_ds() && data->from_ds())){
            //From_DS
            }else if(data->to_ds() && data->from_ds()){
            //BRIDGE
            }else{
                continue;
            }
            addr = data->addr2();
        }
        else{
            continue;
        }

        SIGTYPE sig = isNewAddress(addr);

        if(sig == NEWDEVICE){
            emit NewDevice(sta.to_string().c_str());
        }else if(sig == MAXCHANGE){
            emit MaxDeviceChanged(sta.to_string().c_str());
            refreshAVG();
        }
    }
    clearMap();
}

void ProbeReqSniffer::refreshAVG(){
    int tmp = 0;
    for(auto it = staMap.begin(); it != staMap.end(); ++it){
        tmp += (*it).second;
    }

    avgCount = tmp / staMap.size();
}

SIGTYPE ProbeReqSniffer::isNewAddress(HWAddress<6> addr){
    sta_map_type::iterator it = staMap.find(addr);
    if(it == staMap.end()){
        staMap.insert(std::make_pair(addr, 1));
    }else{
        (it->second)++;
        if(it->second > maxCount){
            maxCount = it->second;
            if(it->first != sta){
                sta = it->first;
                return MAXCHANGE;
            }
        }else if(it->second > avgCount)
            return NEWDEVICE;
    }
    return NONE;
}


void ProbeReqSniffer::setStop(bool flag){
    this->Stop = flag;
}

bool ProbeReqSniffer::getStop(){
    return this->Stop;
}

void ProbeReqSniffer::setInf(string inf_){
    this->inf = inf_;
}

string ProbeReqSniffer::getInf(){
    return inf;
}

/*
 * Author : Famous
 * Pre-Condition : receive Probe-Req frame and analyze frame
 * Post-Condition : if frame is Randomize Probe-Req return true,
 *                  else return false
 *
 */

bool ProbeReqSniffer::isRandomized(Dot11ProbeRequest *req){
    bool flag = false;
    for(const auto &opt : req->options()){
        if(opt.option() == Dot11::VENDOR_SPECIFIC){
            if(opt.data_size() >= 4){
                HWAddress<3> oui = opt.data_ptr();
                if(oui != apple_oui && !flag)
                    break;
                if(oui == apple_oui && static_cast<int>(opt.data_ptr()[3]) == 10){
                    flag = true;
                }
                else if(oui == broadcom_oui && static_cast<int>(opt.data_ptr()[3]) == 2 && flag){
                    if(opt.data_ptr()[4] == 0x00 && req->ssid() == "")
                        return true;
                }
            }
        }
    }
    return false;
}

/*
 * Author : Famous
 * Pre-Condition : receive Probe-Req frame and analyze frame
 * Post-Condition : if there is the fist VENDOR_SPEIFIC Opions OUI is
 *                  Apple_Vendor_OUI then return true, else return false
 *
 */

bool ProbeReqSniffer::isAppleVendor(Dot11ProbeRequest *req){
    for(const auto &opt : req->options()){
        if(opt.data_size() >= 4){
            if(opt.option() == Dot11::VENDOR_SPECIFIC){
                HWAddress<3> oui = opt.data_ptr();
                if(oui == apple_oui)
                    return true;
                else{
                    return false;
                }
            }

        }
    }
    return false;
}

void ProbeReqSniffer::setSSI_MIN(int SSI){
    if(SSI < 0)
        this->SSI_MIN = SSI;
    else
        qDebug() << "Wrong SSI value : " << SSI;
}

int ProbeReqSniffer::getSSI_MIN(){
    return this->SSI_MIN;
}

void ProbeReqSniffer::setTarget(TARGET tar){
    this->target = tar;
}

TARGET ProbeReqSniffer::getTarget(){
    return target;
}

int ProbeReqSniffer::getMaxCount(){
    return this->maxCount;
}

int ProbeReqSniffer::getStaCount(HWAddress<6> sta){
    if(staMap.find(sta) == staMap.end()){
        return 0;
    }

    return staMap.at(sta);
}

void ProbeReqSniffer::clearMap(){
    for(auto it = staMap.begin(); it != staMap.end(); ++it)
        cout << (*it).first << " - " << (*it).second << endl;

    staMap.clear();
    sta = "00:00:00:00:00:00";
    maxCount = 0;
}
