#ifndef STEINHART_H
#define STEINHART_H

#include <QWidget>

namespace Ui {
class SteinHart;
}

class SteinHart : public QWidget
{
    Q_OBJECT

public:
    explicit SteinHart(QWidget *parent = 0);
    ~SteinHart();

private slots:
    void on_pushButton_2_clicked();

    void on_CalculateButton_clicked();

private:
    Ui::SteinHart *ui;
};

#endif // STEINHART_H
