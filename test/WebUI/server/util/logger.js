"use strict";

var winston = require('winston');
var path    = require('path');

winston.emitErrs = true;

var Logger = new winston.Logger({
    transports: [
        new winston.transports.File({
            level: 'debug',
            filename: path.join(__dirname, "../", "node_ngm_server.log"),
            handleExceptions: true,
            json: true,
            maxsize: 5242880 * 2, //10 MB
            maxFiles: 3,
            colorize: false
        }),
        /*
        new winston.transports.Console({
            level: 'debug',
            handleExceptions: true,
            json: false,
            prettyPrint: true,
            colorize: true
        })
        */
    ],
    exitOnError: false
});

module.exports = Logger;