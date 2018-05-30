/*global require, module */

'use strict';
var express         = require('express'),
    Q               = require('q'),
    moment          = require('moment'),
    bodyParser      = require('body-parser'),
    apiRouter       = require('../routes/ApiRouter'),
    logger          = require('../util/logger'),
    fileUpload      = require('express-fileupload');

module.exports = function WebServer(opts) {

    var self    = this,
        server  = null,
        app     = express();

    app.use(bodyParser.json({limit: "50mb"}));
    app.use(bodyParser.urlencoded({ limit: "50mb", parameterLimit: 50000000, extended: false}));
    app.use(fileUpload());
    app.use(express.static(__dirname + '/../../frontend/'));
    app.use('/api',  apiRouter);
    app.all('*', function (req, res, next) {
        res.redirect('/#' + req.originalUrl);
    });

    //General error handler
    app.use(function (err, req, res, next) {
        logger.error("Express general error occurred.", err);

        if (req.app.get('env') !== 'development') {
            delete err.stack;
        }
        res.status(err.statusCode || 500).json(err);
    });

    //region Public Methods
    this.init = function () {
        return Q.Promise(function (resolve, reject) {
            server = app.listen(opts.port, opts.host, function(error) {
                if (error) {
                    reject(error)
                } else {
                    logger.info('Http server start listening. ', {
                        address: server.address().address,
                        port: server.address().port
                    });
                    resolve(true);
                }
            });
        });
    };
    //endregion
};