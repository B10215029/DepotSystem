#ifndef PRODUCTMANAGEMENTFORM_H
#define PRODUCTMANAGEMENTFORM_H

#include <QWidget>
#include "form.h"
#include <QStandardItemModel>
#include "../Product/product.h"

namespace Ui {
class ProductManagementForm;
}

class ProductManagementForm : public Form
{
    Q_OBJECT

public:
    explicit ProductManagementForm(QWidget *parent = 0);
    ~ProductManagementForm();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void showProductManagementResult(QList<Product> products);

private:
    Ui::ProductManagementForm *ui;
};

#endif // PRODUCTMANAGEMENTFORM_H
