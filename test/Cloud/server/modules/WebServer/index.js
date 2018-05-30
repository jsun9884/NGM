"use strict";
/*global require, module */

const   express         = require('express'),
        helmet          = require('helmet'),
        Q               = require('q'),
        bodyParser      = require('body-parser'),
        morgan          = require('morgan'),
        expressJwt      = require('express-jwt'),
        config          = require('config'),
        apiRouter       = require('../../routes/ApiRouter'),
        loginRouter     = require('../../routes/LoginRouter'),
        Logger          = require('../../util/logger')(module);


module.exports = function WebServer(opts) {

    //region Init
    const app   = express();
    let server  = null;

    app.use(morgan('dev'));
    app.use(helmet());
    app.use(bodyParser.urlencoded({extended: false}));
    app.use(bodyParser.json());
    app.use('/api', loginRouter);
    app.use('/api', expressJwt(config.JWT_SECRET), apiRouter);
    app.all('*', function (req, res, next) { res.redirect('/#' + req.originalUrl); });

    //General error handler
    // TODO Implement better error handling
    app.use(function (err, req, res, next) {
        Logger.error('Express general error:', err);

        if (req.app.get('env') !== 'development') {
            delete err.stack;
        }

        switch (err.name) {
            case 'ArgumentException':
                err.statusCode = 404;
                break;
            case 'InsufficientRightsException':
                err.statusCode = 403;
                break;
            default:
                break;
        }

        res.status(err.code === 'invalid_token' ? 401 : (err.statusCode ? err.statusCode : 500)).json(err);
    });
    //endregion Init

    //region Public
    this.shutdown = function () {
        if (server) {server.close(() => Logger.info("WebServer Closed."))}
    };

    this.init = function () {
        return Q.Promise(function (resolve, reject) {
            server = app.listen(opts.port, opts.host, function(error) {
                if (error) {
                    reject(error)
                } else {
                    Logger.info('App listening at http://%s:%s',
                        server.address().address, server.address().port);
                    resolve(true);
                }
            });
        });
    };
    //endregion Public
};