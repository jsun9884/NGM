/*global require, module */
'use strict';

var WebServer           = require('./lib/WebServer');
var BluetoothAdapter    = require('./lib/BluetoothAdapter');
var logger              = require('./util/logger');

// TODO Add Gracefull shutdown

function initErrorHandler(error) {
    logger.error("Failed to Start Application.", error);
    setTimeout(function () { process.exit(1); }, 1000);
}

// TODO App Start configure env variables;
// Application Start
var webServer = new WebServer({
    port : process.env.PORT || '80',
    host : '0.0.0.0'
});

BluetoothAdapter.BluetoothStart();

webServer
    .init()
    .catch(initErrorHandler);