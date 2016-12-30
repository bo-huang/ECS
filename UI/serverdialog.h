#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <QDialog>

namespace Ui {
class ServerDialog;
}

class ServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDialog(QWidget *parent = 0);
    ~ServerDialog();
    void SetServer(QString server);
private slots:
    void on_serverOkButton_clicked();
    void on_testConButton_clicked();
    void on_serverLineEdit_textChanged(const QString &arg1);

private:
    bool IsConnection(QString server);
public:
    bool isOK;
    QString serverName;
private:
    Ui::ServerDialog *ui;
};

#endif // SERVERDIALOG_H
