#include "userview.h"
#include "ui_userview.h"

UserView::UserView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserView)
{
    ui->setupUi(this);

    ///LogInForm
    setForm(Form::Login, new LogInForm(ui->widget));
    connect((LogInForm*) forms[Form::Login], SIGNAL(logInSignal(QString,QString)),
            this, SLOT(logInSlot(QString,QString)));
    connect(this, SIGNAL(logInResult(QString)),
            (LogInForm*) forms[Form::Login], SLOT(showLogInResult(QString)));

    ///CustomerMenu
    setForm(Form::CustomerMenu, new CustomerMenuForm(ui->widget));
    connect((CustomerMenuForm*) forms[Form::CustomerMenu], SIGNAL(getProductsInfoSignal()),
            this, SLOT(getProductsInfoSlot()));
    connect((CustomerMenuForm*) forms[Form::CustomerMenu], SIGNAL(getOrdersInfoSignal()),
            this, SLOT(getOrdersInfoSlot()));
    connect((CustomerMenuForm*) forms[Form::CustomerMenu], SIGNAL(logOutSignal()),
            this, SLOT(logOutSlot()));
    

    ///ManagerMenu
    setForm(Form::ManagerMenu, new ManagerMenuForm(ui->widget));
    connect((ManagerMenuForm*) forms[Form::ManagerMenu], SIGNAL(getProductsInfoSignal()),
            this, SLOT(getProductsInfoSlot()));
    connect((ManagerMenuForm*) forms[Form::ManagerMenu], SIGNAL(getOrdersInfoSignal()),
            this, SLOT(getOrdersInfoSlot()));
    connect((ManagerMenuForm*) forms[Form::ManagerMenu], SIGNAL(logOutSignal()),
            this, SLOT(logOutSlot()));

    ///ProductManagement
    setForm(Form::ProductManagement, new ProductManagementForm(ui->widget));
    connect(this,
            SIGNAL(productManagementResult(QList<Product>)),
            (ProductManagementForm*) forms[Form::ProductManagement],
            SLOT(showProductManagementResult(QList<Product>)));
    connect((ProductManagementForm*) forms[Form::ProductManagement],
            SIGNAL(updateProductsSignal(QList<Product>,QList<Product>,QList<Product>)),
            this,
            SLOT(updateProductsSlot(QList<Product>,QList<Product>,QList<Product>)));

    ///OrderManagement
    setForm(Form::OrderManagement, new OrderManagementForm(ui->widget));
    connect(this,
            SIGNAL(orderManagementResult(QList<Order>,QList<Order>,QList<Order>)),
            (OrderManagementForm*) forms[Form::OrderManagement],
            SLOT(showOrderManagementResult(QList<Order>,QList<Order>,QList<Order>)));
    connect((OrderManagementForm*) forms[Form::OrderManagement], SIGNAL(acceptOrderOrNotSignal(Order)),
            this, SLOT(acceptOrderOrNotSlot(Order)));

    ///CheckOrder
    setForm(Form::CheckOrder, new CheckOrderForm(ui->widget));
    connect(this, SIGNAL(showOrdersSignal(QList<Order>)),
            (CheckOrderForm*) forms[Form::CheckOrder], SLOT(showOrdersSlot(QList<Order>)));

    connect((CheckOrderForm*) forms[Form::CheckOrder], SIGNAL(putSignal(Order)),
            this, SLOT(putOrderSlot(Order)));

    

    connect(this, SIGNAL(checkOrderProductsResult(QList<Product>)),
            (CheckOrderForm*) forms[Form::CheckOrder], SLOT(productsInfoSlot(QList<Product>)));

    ///ConfirmOrder
    setForm(Form::ConfirmOrder, new ConfirmOrderForm(ui->widget)); 
    connect((ConfirmOrderForm*) forms[Form::ConfirmOrder], SIGNAL(postOrdersInfoSignal(QList<Item>)),
            this, SLOT(postOrdersInfoSlot(QList<Item>)));
    connect(this, SIGNAL(postValidSignal(bool)),
            (ConfirmOrderForm*) forms[Form::ConfirmOrder], SLOT(postValid(bool)));

    ///SingleOrder
    setForm(Form::SingleOrder, new SingleOrderForm(ui->widget));
    connect(this,
            SIGNAL(productSingleOrderResult(QList<Product>)),
            (SingleOrderForm*) forms[Form::SingleOrder],
            SLOT(showProductSingleOrderResult(QList<Product>)));
    connect((SingleOrderForm*) forms[Form::SingleOrder],
            SIGNAL(transferOrderSignal(QList<Item>,QList<QString>)),
            (ConfirmOrderForm*) forms[Form::ConfirmOrder],
            SLOT(transferOrderSlot(QList<Item>,QList<QString>)));

    ///ModifyOrder
//    setForm(Form::ModifyOrder, new ModifyOrderForm(ui->widget));
//    connect((CheckOrderForm*) forms[Form::CheckOrder], SIGNAL(transferOrderModify(Order)),
//            (ModifyOrderForm*) forms[Form::ModifyOrder], SLOT(transferOrderModifySlot(Order)));

    connector = new Connector();
    connect(connector, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    forms[Form::Login]->show();

    ui->widget->resize(378, 232);
    ui->label->setText("");
}

UserView::~UserView()
{
    delete ui;
}

void UserView::changeWindow(int from, int to)
{
    forms[from]->hide();
    forms[to]->show();
}

void UserView::showMessage(const QString &text, int timeout = 0)
{
    ui->statusBar->showMessage(text, timeout);
}

void UserView::replyFinished(QNetworkReply* reply) {
    reply->deleteLater();
    while (widgetsRecycleList.size() != 0) delete widgetsRecycleList.takeAt(0);
    setEnabled(true);

    QString response;

    if (reply->error() != QNetworkReply::NoError) { //失敗的話
        response = reply->readAll();
        qDebug() << "\n[UserView::replyFinished] @ ERROR]" << reply->errorString() << response;

        QJsonObject object = QJsonDocument::fromJson(response.toUtf8()).object();
        switch (whichFormCallIndex) {
        case Form::Login:
            emit logInResult(reply->errorString() + "\n" + object["message"].toString());
            break;
        case Form::CustomerMenu:
            qDebug() << reply->errorString();
            break;
        case Form::ManagerMenu:
            break;
        case Form::ProductManagement:
            emit showMessage(response, 10000);
            break;
        case Form::ConfirmOrder:
            emit postValidSignal(false);
            break;
        default:
            break;
        }
    }
    else if (reply->isReadable()) { //成功的話
        QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(
                    reply->header(QNetworkRequest::SetCookieHeader));
        qDebug() << "\n[UserView::replyFinished] @ COOKIES]" << cookies;

        if(cookies.count() != 0) connector->cookieJar()->setCookiesFromUrl(cookies,
                                                                           reply->request().url());

        response = reply->readAll();
        qDebug() << "\n[UserView::replyFinished @ DATA]" << response.toUtf8().data();

        QJsonObject object = QJsonDocument::fromJson(response.toUtf8()).object();
        switch (whichFormCallIndex) {
        case Form::Login:
            if (object["type"].toString().split(" ")[0] == "Admin") {
                emit changeWindow(Form::Login, Form::ManagerMenu);
                userName = "Admin:" + userName;
            }
            else {
                userName = "User:" + userName;
                emit changeWindow(Form::Login, Form::CustomerMenu);
            }
            ui->label->setText(userName);
            emit logInResult("");
            break;

        case Form::CustomerMenu:
            break;

        case Form::ConfirmOrder:
            emit postValidSignal(true);
            break;

        case Form::ManagerMenu: {
            if (subFunc == 0) {
                QJsonArray array = QJsonDocument::fromJson(response.toUtf8()).array();
                QList<Product> products;
                for (int i = 0; i < array.size(); i++) {
                    Product product(array[i].toObject()["_id"].toString());
                    product.setName(array[i].toObject()["name"].toString());
                    product.setStock(array[i].toObject()["stock"].toInt());
                    product.setPrice(array[i].toObject()["price"].toInt());
                    products.append(product);
                }
                emit productManagementResult(products);
            }
            else if (subFunc == 1) {
                QJsonArray array_taken = object["I_TAKE"].toArray(),
                        array_notTaken = object["NOT_TAKEN"].toArray(),
                        array_notITake = object["NOT_MY_BUSINESS"].toArray();
                QList<Order> orders_taken, orders_notTaken, orders_notITake;

                for (int i = 0; i < array_taken.size(); i++) {
                    Order order(array_taken[i].toObject()["_id"].toString());
                    order.setState(array_taken[i].toObject()["state"].toString());
                    order.setWhoOrdered(array_taken[i].toObject()["ordered_by"].toString());

                    QJsonArray items = array_taken[i].toObject()["items"].toArray();
                    for (int j=0; j<items.size(); ++j) {
                        order.addItem(items[j].toObject()["productId"].toString(),
                                items[j].toObject()["amount"].toInt());
                    }

                    orders_taken.append(order);
                }

                for (int i = 0; i < array_notTaken.size(); i++) {
                    Order order(array_notTaken[i].toObject()["_id"].toString());
                    order.setState(array_notTaken[i].toObject()["state"].toString());
                    order.setWhoOrdered(array_notTaken[i].toObject()["ordered_by"].toString());

                    QJsonArray items = array_notTaken[i].toObject()["items"].toArray();
                    for (int j=0; j<items.size(); ++j) {
                        order.addItem(items[j].toObject()["productId"].toString(),
                                items[j].toObject()["amount"].toInt());
                    }

                    orders_notTaken.append(order);
                }

                for (int i = 0; i < array_notITake.size(); i++) {
                    Order order(array_notITake[i].toObject()["_id"].toString());
                    order.setState(array_notITake[i].toObject()["state"].toString());
                    order.setWhoOrdered(array_notITake[i].toObject()["ordered_by"].toString());

                    QJsonArray items = array_notITake[i].toObject()["items"].toArray();
                    for (int j=0; j<items.size(); ++j) {
                        order.addItem(items[j].toObject()["productId"].toString(),
                                items[j].toObject()["amount"].toInt());
                    }

                    orders_notITake.append(order);
                }

                //orders_taken += orders_notITake;
                emit orderManagementResult(orders_taken, orders_notTaken, orders_notITake);
            }
            break;
        }

        case Form::ProductManagement:
            emit showMessage("成功更新" + response, 10000);
            break;

        case Form::OrderManagement:
            emit showMessage("成功更新" + response, 10000);
            getOrdersInfoSlot();
            break;

        case Form::SingleOrder: {
            if(subFunc == 0){
                QJsonArray array = QJsonDocument::fromJson(response.toUtf8()).array();
                QList<Product> products;
                for (int i = 0; i < array.size(); i++) {
                    Product product(array[i].toObject()["_id"].toString());
                    product.setName(array[i].toObject()["name"].toString());
                    product.setStock(array[i].toObject()["stock"].toInt());
                    product.setPrice(array[i].toObject()["price"].toInt());
                    products.append(product);
                }
                emit productSingleOrderResult(products);
            }else{
                response = response.right(response.size() - response.indexOf(":") - 1);
                response = response.left(response.size() - 1);
                QJsonArray array = QJsonDocument::fromJson(response.toUtf8()).array();
                QList<Order> orders;
                for (int i = 0; i < array.size(); i++) {
                    Order order(array[i].toObject()["_id"].toString());
                    order.setState(array[i].toObject()["state"].toString());
                    order.setWhoOrdered(array[i].toObject()["ordered_by"].toString());
                    order.setSubmitted(array[i].toObject()["_v"].toBool());

                    //Dealing with items
                    for(int j = 0;j<array[i].toObject()["items"].toArray().size();j++){
                        order.addItem(array[i].toObject()["items"].toArray()[j].toObject()["productId"].toString(),
                                array[i].toObject()["items"].toArray()[j].toObject()["amount"].toInt());
                    }
                    orders.append(order);
//                    QList<Item> tmp = orders[i].getItems();
//                    qDebug() << "@@@@@@@@@@@@@@@@";

//                    for(int j = 0;j<tmp.size();j++){
//                        qDebug() << tmp.at(j).product;
//                        qDebug() << tmp.at(j).amount;

//                    }
                    
                }
               emit showOrdersSignal(orders);

               //Get the product list
               whichFormCallIndex = Form::CheckOrder;

               connector->getProductsInfo();
            }
            break;
        }

        case Form::CheckOrder:{
            
               QJsonArray array = QJsonDocument::fromJson(response.toUtf8()).array();
                QList<Product> products;
                for (int i = 0; i < array.size(); i++) {
                    Product product(array[i].toObject()["_id"].toString());
                    product.setName(array[i].toObject()["name"].toString());
                    product.setStock(array[i].toObject()["stock"].toInt());
                    product.setPrice(array[i].toObject()["price"].toInt());
                    products.append(product);
                }
                emit checkOrderProductsResult(products);

            break;
        }

        default:
            break;
        }
    }
    qDebug() << "\n=======================================================================";
}

void UserView::showLoadingDialog() {
    setEnabled(false);
    QDialog *dialog = new QDialog(this, Qt::CustomizeWindowHint);
    QLabel *content = new QLabel("Loading..." ,dialog);
    dialog->show();

    widgetsRecycleList.append(content);
    widgetsRecycleList.append(dialog);
}

void UserView::logInSlot(QString username, QString password) { //Form::Login
    showLoadingDialog();

    whichFormCallIndex = Form::Login;
    userName = username;

    connector->logIn(username, password);
}

void UserView::logOutSlot() {
    whichFormCallIndex = -1;

    connector->logOut();
    ui->label->setText("");
    userName = "";
}

void UserView::getProductsInfoSlot() {
    showLoadingDialog();

    if (userName.at(0) == 'A') {
        whichFormCallIndex = Form::ManagerMenu;
        subFunc = 0;
    }
    else {
        whichFormCallIndex = Form::SingleOrder;
        subFunc = 0;
    }

    connector->getProductsInfo();
}

void UserView::acceptOrderOrNotSlot(Order order) {
    showLoadingDialog();

    whichFormCallIndex = Form::OrderManagement;

    connector->putOrderTaken(order);
}

void UserView::getOrdersInfoSlot() {
    showLoadingDialog();

    if (userName.at(0) == 'A') {
        whichFormCallIndex = Form::ManagerMenu;
        subFunc = 1;
    }
    else {
        whichFormCallIndex = Form::SingleOrder;
        subFunc = 1;
    }

    connector->getOrdersInfo();
}

void UserView::postOrdersInfoSlot(QList<Item> items){
    if(userName.at(0) != 'A'){
        whichFormCallIndex = Form::ConfirmOrder;
    }
    showLoadingDialog();
    connector->postNewOrders(items);
}

void UserView::putOrderSlot(Order order){
    connector->putOrder(order);

}

void UserView::updateProductsSlot(QList<Product> newProducts,
                                  QList<Product> changedProducts,
                                  QList<Product> deletedProducts) {
    whichFormCallIndex = Form::ProductManagement;

    if (newProducts.size() > 0) connector->postNewProducts(newProducts);
    if (changedProducts.size() > 0) connector->putEditedProducts(changedProducts);
    if (deletedProducts.size() > 0) connector->deleteProducts(deletedProducts);
}

void UserView::resizeEvent(QResizeEvent* event)
{
    for (int i = 0; i < Form::FORM_COUNT; i++) {
        forms[i]->resize(ui->widget->size());
    }
}

void UserView::setForm(int formIndex, Form *form)
{
    forms[formIndex] = form;
    forms[formIndex]->hide();
    connect(forms[formIndex], SIGNAL(changeWindow(int,int)),
            this, SLOT(changeWindow(int,int)));
    connect(forms[formIndex], SIGNAL(showMessage(QString,int)),
            this, SLOT(showMessage(QString,int)));
}
