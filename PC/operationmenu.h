#ifndef OPERATIONMENU_H
#define OPERATIONMENU_H

#include <QWidget>

namespace Ui {
class OperationMenu;
}

class OperationMenu : public QWidget
{
    Q_OBJECT

public:
    explicit OperationMenu(QWidget *parent = 0);
    ~OperationMenu();

private slots:
    void on_BackButtonOperation_clicked();

    void on_GoToButton_clicked();

private:
    Ui::OperationMenu *ui;
};

#endif // OPERATIONMENU_H
