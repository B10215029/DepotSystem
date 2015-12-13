#include "managermenuform.h"
#include "ui_managermenuform.h"

ManagerMenuForm::ManagerMenuForm(QWidget *parent) :
    Form(parent),
    ui(new Ui::ManagerMenuForm)
{
    ui->setupUi(this);
}

ManagerMenuForm::~ManagerMenuForm()
{
    delete ui;
}

void ManagerMenuForm::on_pushButton_3_clicked()
{
    changeWindow(ManagerMenu, Login);
}
