//db.getCollectionNames()
//db.accounts.find()
//db.products.remove({"name":"toastA"})
var restify  = require('restify');
var server   = restify.createServer();
server.use(restify.bodyParser());

var sessions = require("client-sessions");
server.use(sessions({
  cookieName: 'depotSession', // cookie name dictates the key name added to the request object
  secret: 'DolanGooby', // should be a large unguessable string
  duration: 3 * 24 * 60 * 60 * 1000 // how long the session will stay valid in ms
}));

var mongoose = require('mongoose');
var Schema   = mongoose.Schema;
mongoose.connect('mongodb://localhost/test1');
//mongoose.connect('mongodb://localhost/depot');

var AccountSchema = new Schema({
    username: String,
    password: String,
    orders  : [Schema.Types.ObjectId],
    orders_taken: [Schema.Types.ObjectId],
    type:String
});

var OrderSchema = new Schema({
    state: String,
    items: [{
        product:Schema.Types.ObjectId,
        amount:Number
    }],
    submitted: Boolean,
    ordered_by: [Schema.Types.ObjectId],
    taken_by: [Schema.Types.ObjectId]
});

var ProductSchema = new Schema({
    name:String,
    stock:Number,
    price:Number
});

//AccountSchema.static('findByUsername', function (name, callback) {
//    return this.find({ username: name }, callback);
//});

var Account = mongoose.model('Accounts', AccountSchema);
var Order   = mongoose.model('Order', OrderSchema);
var Product = mongoose.model('Product', ProductSchema);

server.pre(function(req, res, next) {
    req.headers.accept = 'application/json';
    return next();
});

server.post('/register', function(request, response, next) {

    console.log("username = " + request.params.username);
    console.log("password = " + request.params.password);

    if(request.params.username && request.params.password){
        Account.findOne({username: request.params.username}, function (err, user) {
            if(err)
            {
                console.log("error:" + err);
                response.send(500, {message: "sorry gooby, I don't know what's going on. contact me pls"});

            }else{
                console.log(" user = " + user);
                if(user){
                    response.send(500, {message: "fuck you gooby, this username is already used!!"});
                }

                else
                {
                    var newUser = new Account({
                        username: request.params.username,
                        password: request.params.password,
                        type:"customer"
                    });
                    newUser.save(function(error){
                        if(error){
                            response.send(500, {message: "sorry gooby, database server is down!!"});
                        }else{
                            response.send(200, {message: "Successful, very good gooby, that's my good dog."});
                        }
                    });
                }
            }
        });

    }else{
        response.send(400, {message: "fuck you gooby, input the right format!!!"});
    }

    return next();
});

server.post('/login', function(request, response, next) {

    console.log("username = " + request.params.username);
    console.log("password = " + request.params.password);

    if(request.params.username && request.params.password){
        Account.findOne({username: request.params.username}, function (err, user) {
            if(err)
            {
                console.log("error:" + err);
                response.send(500, {message: "sorry gooby, I don't know what's going on. contact me pls"});
            }else{
                console.log(" user = " + user);
                if(user){
                    if(user.username == request.params.username && user.password == request.params.password){
                        request.depotSession.username = user.username;
                        console.log(request.depotSession.username);
                        response.send(200, {message: "Login successfully, gooby!!"});
                    }else{
                        response.send(500, {message: "Wrong password, stupid gooby!!"});
                    }
                }else{
                    response.send(500, {message: "fuck you gooby, this username doesn't exist!!"});
                }
            }
        });

    }else{
        response.send(400, {message: "fuck you gooby, input the right format!!!"});
    }

    return next();
});

server.get('/logout', function(request, response, next){
    if(request.depotSession.username){
        request.depotSession.destroy();
    }
    return next();
});

server.post('/products', function(request, response, next){
    if (!request.depotSession.username) {
        response.send(400, {message: "fuck you gooby!!!, you didn't login or you are not an admin!!"});
    } else {
        Account.findOne({username: request.depotSession.username}, function (err, user) {
            if(err){
                response.send(400, {message: "Sorry gooby!!!, Account Error!!!"});
            }
            if(user.type == "admin"){
                if(request.params.productname){
                    Product.findOne({name: request.params.productname}, function (err, product) {
                        if(err)
                        {
                            console.log("error:" + err);
                            response.send(500, {message: "sorry gooby, I don't know what's going on. contact me pls"});
                        }else{
                            console.log(" product = " + product);
                            if(product){
                                response.send(400, {message: "fuck you gooby!!!, this product has already been added!!"});
                            }else{
                                var product = new Product({
                                    name:request.params.productname,
                                    stock:request.params.stock,
                                    price:request.params.price
                                });
                                product.save(function(error){
                                    if(error){
                                        response.send(500, {message: "Sorry gooby, database server is down!!"});
                                    }else{
                                        response.send(200, {message: "Successful, very good gooby, You add a product."});
                                    }
                                });
                            }
                        }
                    })
                }else{
                    response.send(400, {message: "fuck you gooby, send the right format!!!"});
                }
//                Product.findOne({name: request.params.productname}, function (err, product){
//                    if(err){
//                        console.log("error");
//                        response.send(400, {message: "Sorry gooby!!!, Product Error!!!"});
//                    }
//                    if(product){
//                        response.send(400, {message: "fuck you gooby!!!, this product has already been added!!"});
//                    }else{
//                        console.log(request.params.productname);
//                        if(request.params.productname && request.params.stock && request.params.price){
//                            var product = new Product({
//                                name:request.params.productname,
//                                stock:request.params.stock,
//                                price:request.params.price
//                            });
//                            product.save(function(error){
//                                if(error){
//                                    response.send(500, {message: "Sorry gooby, database server is down!!"});
//                                }else{
//                                    response.send(200, {message: "Successful, very good gooby, You add a product."});
//                                }
//                            });
//                        }else{
//                            response.send(500, {message: "fuck you gooby, send the right format!!!"});
//                        }
//                    }
//                });
            }else if(user.type == "customer"){
                response.send(500, {message: "fuck you gooby, you are not a admin!!!"});
            }else{
                response.send(500, {message: "fuck you gooby, sth wrong in user accounts type!!!"});
            }
        });
    }
    return next();
});

server.get('/products', function(request, response, next){
    Product.find({}, function (err, products) {
        response.send(200, products);
    });
    return next();
});

server.put('/products', function(request, response, next){
    if (!request.depotSession.username) {
        response.send(400, {message: "fuck you gooby!!!, you didn't login or you are not an admin!!"});
    } else {
        Account.findOne({username: request.depotSession.username}, function (err, user) {
            if(user.type == "admin"){
                if(request.params.productname){
                    Product.findOne({name: request.params.productname}, function (err, product) {
                        if(err){
                            console.log("error:" + err);
                            response.send(500, {message: "sorry gooby, I don't know what's going on. contact me pls"});
                        }else{
                            console.log("product = " + product);
                            if(product){
                                if(request.params.stock){
                                    var stock = parseInt(product.stock) + parseInt(request.params.stock);
                                    product.stock = stock.toString();
                                }
                                if(request.params.price){
                                    product.price = request.params.price;
                                }
                                product.save(function(error) {
                                    if(error){
                                        response.send(500, {message: "Sorry gooby, database server is down!!"});
                                    }else{
                                        response.send(200, {message: "Successful, very good gooby, You update a product."});
                                    }
                                });
                            }
                        }
                    })
                }else{
                    response.send(400, {message: "fuck you gooby, send the right format!!!"});
                }
            }else if(user.type == "customer"){
                response.send(400, {message: "fuck you gooby, you are not a admin!!!"});
            }else{
                response.send(500, {message: "fuck you gooby, sth wrong in user accounts type!!!"});
            }
        });
    }
    return next();
});

server.get('/orders', function(req, res, next) {
    if (! req.depotSession.username) {
        res.send(400, { message: "fuck you gooby!!!, you didn't login or you are not an admin!!" });
    } else {
        Account.findOne({ username: req.depotSession.username }, function (err, user) {
            var orders = [];
            for (order of user.orders) {
                Order.findOne({ _id: mongoose.Types.ObjectId(order._id) }, function(err, order) {
                    orders.push(order);
                })
            }
            res.send(200, orders);
        });
    }
})

server.post('/orders', function(req, res, next) {
    if (! req.depotSystem.username) {
        res.send(400, { message: "fuck you gooby!!!, you didn't login or you are not an admin!!" });
    } else {
        Account.findOne({ username: req.depotSession.username }, function (err, user) {

        });
    }
});

server.listen(80, function() {
    console.log(server.name + ' listening at ' +  server.url);
});
