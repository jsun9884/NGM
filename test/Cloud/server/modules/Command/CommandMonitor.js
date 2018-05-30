'use strict';

const config            = require('config');
const NR                = require("node-resque");
const Logger            = require('../../util/logger')(module);
const Command           = require('../../modules/Command/Command');
const RedisConnector    = require('../RedisConnector');

function CommandMonitor() {

    //region Init
    const connectionDetails = {
        pkg:       'ioredis',
        host:      config.AwsElastiCache.endpoint,
        password:  null,
        port:      config.AwsElastiCache.port,
        database:  config.AwsElastiCache.queueDB,
        namespace: 'resque'
    };

    let queue = new NR.queue({connection: connectionDetails});

    queue.connect(function() {
        Logger.info("CommandMonitor(). Resque queue connected.")
    });
    //endregion Init

    //region Queue Event Handlers
    queue.on('error', function(error){
        Logger.error("CommandMonitor()", error);
    });
    //endregion Queue Event Handlers

    //region Public Methods
    this.closeQueue = function() {
        return new Promise(function (resolve, reject) {
            queue.end(function (err) {
                err ? reject(err) : resolve(true)
            })
        })
    };

    this.createMonitorTask = function createTask(command) {
        Logger.debug("createMonitorTask()", { Command: command});

        return new Promise(function (resolve, reject) {
            let params = {
                deviceId: command.deviceId,
                commandId: command.commandId,
                commandCode: command.commandCode
            };
            queue.enqueue('CommandQueue', "monitor", [params], function (err) {
                err ? reject(err) : resolve(command)
            });
        });
    };
    //endregion
}

module.exports = new CommandMonitor();