#ifndef STARTUPVAL_H
#define STARTUPVAL_H

#include <QWidget>

namespace Ui {
class StartUPVal;
}

class StartUPVal : public QWidget
{
    Q_OBJECT

public:
    explicit StartUPVal(QWidget *parent = 0);
    ~StartUPVal();

private slots:
    void on_ExitButton_clicked();

    void on_SaveButton_clicked();

private:
    Ui::StartUPVal *ui;
};

#endif // STARTUPVAL_H
