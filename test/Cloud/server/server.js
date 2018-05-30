'use strict';

const AWS               = require('aws-sdk');
const WebServer         = require('./modules/WebServer');
const CommandMonitor    = require('./modules/Command/CommandMonitor');
const RedisConnector    = require('./modules/RedisConnector');
const Logger            = require('./util/logger')(module);

/* AWS Service Config */
AWS.config.update({region: 'us-west-2'});       // Globally set region

const webServer = new WebServer({
    port: process.env.PORT || '8080',
    host: 'localhost'
});

process.on('uncaughtException', function (err) {
    Logger.error('An uncaughtException, the program will shutdown.', err);
    shutdown(err);
});

//Handle ctrl+c event
process.on('SIGINT', function () {
    Logger.info('SIGINT Received.');
    shutdown();
});

// Handle KILL Signal
process.on('SIGTERM', function () {
    Logger.info('SIGTERM Received.');
    shutdown();
});

function shutdown(err) {
    Logger.info("Shutdown.", {pid: process.pid});
    webServer
        .shutdown();

    CommandMonitor
        .closeQueue()
        .then(function () {
            Logger.info("Resque queue closed.");
            return RedisConnector.quit();
        })
        .then(function () {
            Logger.info("Redis connection closed.");
            err ? process.exit(1) : process.exit(0)
        })
        .catch(function (error) {
            Logger.error("Error during shutdown.", error);
            process.exit(1)
        })
}

// Application Start
Logger.info("NGM Server Application start-up.");

webServer
    .init()
    .catch(function (error) {
        Logger.error("Failed to start application.", error);
        shutdown(error);
    });