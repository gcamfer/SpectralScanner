#ifndef AUTENTICATE_H
#define AUTENTICATE_H

#include <QWidget>

namespace Ui {
class Autenticate;
}

class Autenticate : public QWidget
{
    Q_OBJECT

public:
    explicit Autenticate(QWidget *parent = 0);
    ~Autenticate();


private slots:
    void on_CancellButton_clicked();

    void on_OKButton_clicked();

private:
    Ui::Autenticate *ui;
};

#endif // AUTENTICATE_H
