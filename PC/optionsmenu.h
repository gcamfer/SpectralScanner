#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include <QWidget>

namespace Ui {
class OptionsMenu;
}

class OptionsMenu : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsMenu(QWidget *parent = 0);
    ~OptionsMenu();

private slots:
    void on_exitButton_clicked();

    void on_operationButton_clicked();

    void on_installButton_clicked();

    void on_StartUPButton_clicked();

    void on_calibrationButton_clicked();

private:
    Ui::OptionsMenu *ui;
};

#endif // OPTIONSMENU_H
