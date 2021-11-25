#include "interfacewidget.h"
#include "ui_interfacewidget.h"

InterfaceWidget::InterfaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterfaceWidget)
{
    ui->setupUi(this);
    //QWidget::showMaximized();
    getInfList();
    ui->SelectBtn->setDisabled(true);
}

InterfaceWidget::~InterfaceWidget()
{
    delete ui;
}

/*
 * Pre-Condition : none, called by widget, to get the interface list
 * Post-Condition : add interface name to infListWidget
 */

void InterfaceWidget::getInfList(){
    FILE *stream = popen("iwconfig", "r");
    char buff[255];
    QString str;
    QStringList list;
    QString dev;

    while(fgets(buff, sizeof(buff), stream)!=NULL){
        str.append(buff);
    }

    list = str.split(QRegExp("\\n\\s*\\n"));

    for(int i=0; i<list.size(); i++){
        if(list.at(i).contains("IEEE 802.11")){
            dev = (list.at(i).split(" ").at(0));

            if(canSetRfmon(dev.toStdString()))
                ui->infListWidget->addItem(dev);
        }
    }

    pclose(stream);

    if(ui->infListWidget->count() == 0)
        ui->infListWidget->addItem("No available device");
}

void InterfaceWidget::on_SelectBtn_clicked()
{
    this->close();
}

void InterfaceWidget::on_infListWidget_itemClicked(QListWidgetItem *item)
{
    ui->SelectBtn->setDisabled(false);
    inf = item->text();
}

void InterfaceWidget::on_CloseBtn_clicked()
{
    this->close();
}

QString InterfaceWidget::getInf(){
    return this->inf;
}

/*
 * Pre-Condition : Analysing received devcie can set monitor mode
 * Post-Condition : if device can set monitor mode, then return true
 *                  else return false
 */

bool InterfaceWidget::canSetRfmon(string dev){
    if((handle = pcap_create(dev.c_str(), errbuf)) == NULL)
        return false;

    if(pcap_can_set_rfmon(handle) == 0){
        pcap_close(handle);
        return false;
    }

    pcap_close(handle);
    return true;
}
