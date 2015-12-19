#include "Order.h"

typedef unsigned int ProductID;

// Order getOrder(int num){
// 	Order tmp;

// 	/*access order database*/

// 	//tmp = OrderDataBaseManager::find(num);

// 	return tmp;
// }

// Order getAllOrders(){

// }
Order::Order() {

}

Product getProductWithID(ProductID id){
	Product p;
/*	Getting Product*/
	return p;
}

int Order::getNumber(){
	return number;
}

State Order::getState(){
	return state;
}

void Order::setState(State stateParam){
	state = stateParam;
}

QVector<Item> Order::getItems(){
	return items;
}

void Order::addItem(QString name, int amount = 1){
    ProductID requiredProduct;// = find(name); TODO

	items.append(Item(requiredProduct, amount));

}

int Order::getItemAmount(QString name){
	int i = 0, itemSize = items.size();
	while(i < itemSize){
        if(getProductWithID(items[i].product).getName() == name){
			return items[i].amount;
		}
		i++;
	}
	return -1;
}

void Order::setItemAmount(QString name, int amount){
	int i = 0, itemSize = items.size();
	while(i < itemSize){
        if(getProductWithID(items[i].product).getName() == name){
			items[i].amount = amount;
			break;
		}
		i++;
	}
}

void Order::deleteItem(QString name){
	QVectorIterator<Item> iter(items);
	while (iter.hasNext()){
		Item tmpItem = iter.next();
        if(getProductWithID(tmpItem.product).getName() == name){
//            items.erase(iter.previous()); TODO
			break;
		}
	}
}

void Order::submit(){

}

bool Order::isSubmitted(){
    return true; //TODO
}

QString Order::getWhoOrdered(){
	return orderedBy;
}

void Order::setWhoOrdered(QString id){
	orderedBy = id;
}

QString Order::getWhoTaken(){
	return takenBy;
}

void Order::setWhoTaken(QString id){
	takenBy = id;
}
