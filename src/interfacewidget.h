#ifndef INTERFACEWIDGET_H
#define INTERFACEWIDGET_H

/*
 * Class for choose interface
 *
 * Pre-Condition : none, called by MainWindow, and print out interface list(using ifconfig)
 * Post-Condition : return selected interface name
 */

#include <QWidget>
#include <QDebug>
#include <QListWidgetItem>
#include <pcap.h>

using std::string;

namespace Ui {
class InterfaceWidget;
}

class InterfaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceWidget(QWidget *parent = 0);
    ~InterfaceWidget();
    void getInfList();
    bool canSetRfmon(string);
    QString getInf();

private slots:
    void on_SelectBtn_clicked();
    void on_infListWidget_itemClicked(QListWidgetItem *item);
    void on_CloseBtn_clicked();

private:
    Ui::InterfaceWidget *ui;
    QString inf;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
};

#endif // INTERFACEWIDGET_H
