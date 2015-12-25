#include "userview.h"
#include "ui_userview.h"

UserView::UserView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserView)
{
    ui->setupUi(this);

    //LogInForm
    setForm(Form::Login, new LogInForm(this));
    connect((LogInForm*) forms[Form::Login], SIGNAL(logInSignal(QString,QString)),
            this, SLOT(logInSlot(QString,QString)));
    connect(this, SIGNAL(logInResult(QString)),
            (LogInForm*) forms[Form::Login], SLOT(showLogInResult(QString)));

    //CustomerMenu
    setForm(Form::CustomerMenu, new CustomerMenuForm(this));
    connect((CustomerMenuForm*) forms[Form::CustomerMenu], SIGNAL(logOutSignal()),
            this, SLOT(logOutSlot()));

    //ManagerMenu
    setForm(Form::ManagerMenu, new ManagerMenuForm(this));
    connect((ManagerMenuForm*) forms[Form::ManagerMenu], SIGNAL(getProductsInfoSignal()),
            this, SLOT(getProductsInfoSlot()));
    connect((ManagerMenuForm*) forms[Form::ManagerMenu], SIGNAL(logOutSignal()),
                this, SLOT(logOutSlot()));

    //ProductManagement
    setForm(Form::ProductManagement, new ProductManagementForm(this));
    connect(this,
            SIGNAL(productManagementResult(QList<Product>)),
            (ProductManagementForm*) forms[Form::ProductManagement],
            SLOT(showProductManagementResult(QList<Product>)));
    connect((ProductManagementForm*) forms[Form::ProductManagement],
            SIGNAL(updateProductsSignal(QList<Product>,QList<Product>,QList<Product>)),
            this,
            SLOT(updateProductsSlot(QList<Product>,QList<Product>,QList<Product>)));

    //OrderManagement
    setForm(Form::OrderManagement, new OrderManagementForm(ui->widget));
    forms[Form::Login]->show();

    connector = new Connector();
    connect(connector, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    setForm(Form::CheckOrder, new CheckOrderForm(ui->widget));
    setForm(Form::ConfirmOrder, new ConfirmOrderForm(ui->widget));
    setForm(Form::SingleOrder, new SingleOrderForm(ui->widget));
    ui->widget->resize(378, 232);
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

    if (reply->error() != QNetworkReply::NoError) {
        response = reply->readAll();
        qDebug() << "\n[UserView::replyFinished] @ ERROR]" << reply->errorString() << response;

        QJsonObject object = QJsonDocument::fromJson(response.toUtf8()).object();
        switch (whichFormCallIndex) {
        case Form::Login:
            emit logInResult(reply->errorString() + "\n" + object["message"].toString());
            break;
        case Form::CustomerMenu:
            break;
        case Form::ManagerMenu:
            break;
        case Form::ProductManagement:
            emit showMessage(response, 10000);
            break;
        default:
            break;
        }
    }
    else if (reply->isReadable()) {
        QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(
                    reply->header(QNetworkRequest::SetCookieHeader));
        qDebug() << "\n[UserView::replyFinished] @ COOKIES]" << cookies;

        if(cookies.count() != 0) connector->cookieJar()->setCookiesFromUrl(cookies,
                                                                           reply->request().url());

        response = reply->readAll();
        printf("\n[UserView::replyFinished @ DATA] %s\n", response.toUtf8().data());
        fflush(stdout);

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
        case Form::ManagerMenu: {
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
            break;
        }
        case Form::ProductManagement:
            emit showMessage("成功更新" + response, 10000);
            break;
        default:
            break;
        }
    }
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
}

void UserView::getProductsInfoSlot() { //Form::ManagerMenu
    showLoadingDialog();

    if (userName.at(0) == 'A') whichFormCallIndex = Form::ManagerMenu;
    else whichFormCallIndex = -1;

    connector->getProductsInfo();
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
