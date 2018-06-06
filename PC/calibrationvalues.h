#ifndef CALIBRATIONVALUES_H
#define CALIBRATIONVALUES_H

#include <QWidget>

namespace Ui {
class CalibrationValues;
}

class CalibrationValues : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationValues(QWidget *parent = 0);
    ~CalibrationValues();

private slots:
    void on_ExitButton_clicked();

    void on_SaveButton_clicked();

private:
    Ui::CalibrationValues *ui;
};

#endif // CALIBRATIONVALUES_H
