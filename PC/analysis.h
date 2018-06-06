#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QWidget>

namespace Ui {
class Analysis;
}


class Analysis : public QWidget
{
    Q_OBJECT

public:
    explicit Analysis(QWidget *parent = 0);
    ~Analysis();

private slots:
    void on_pushButton_clicked();

    void on_reanalyzeButton_clicked();

    void on_LenOnButton_clicked();

    void on_LedOffButton_clicked();

    double getEFF(float lambda);

    void getRGB(float lambdax);


private:
    Ui::Analysis *ui;
};

#endif // ANALYSIS_H
