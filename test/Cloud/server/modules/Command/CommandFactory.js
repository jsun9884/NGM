'use strict';
/*global require, module */

const Q               = require('q');
const uuid            = require('uuid');
const config          = require('config');
const Command         = require('../Command/Command');
const RedisConnector  = require('../RedisConnector');
const S3Connector     = require('../S3Connector');
const DynamoDBConnector = require('../DynamoDbConnector');
const Logger          = require('../../util/logger')(module);

function CommandFactory() {

    //region Init
    let CMD_CODES = {
        FW_UPGRADE:     100,
        TI_RESET:       110,
        SOM_RESET:      120,
        COUNTER_RESET:  130,
        GET_HR_DATA:    300,
        GET_REG_DATA:   301,
        CALIBRATE:      302,
        SET_CFG_FILE:   310,
        GET_CFG_FILE:   320,
        PREPARE_HR_DATA: 330,
        SET_CFG_PARAMS: 410,
        GET_CFG_PARAMS: 420
    };
    //endregion

    //region Private Methods
    function createCommonCommand(req){
        Logger.debug("createCommonCommand()");

        return Q.Promise(function(resolve, reject){
            let command  = new Command({
                deviceId        : req.params.deviceId,
                commandId       : uuid.v4(),
                commandCode     : req.body.commandCode,
                params          : req.body.params || {},
                runState        : 'POSTED',
                cacheDbClient   : RedisConnector
            });

            command
                .saveToCache()
                .then(() => DynamoDBConnector.saveCommand(command))
                .then(() => {resolve(command)})
                .catch(reject)
        })
    }

    function createFwUpgradeCommand(req) {
        let opts = {
            operation: "DOWNLOAD",
            repository: "FIRMWARE",
            filename: req.body.params.filename
        };

        Logger.debug("createFwUpgradeCommand()", {Opts: opts});

        function createCommand(url) {
            let command = new Command({
                deviceId: req.params.deviceId,
                commandId: uuid.v4(),
                commandCode: req.body.commandCode,
                runState: 'POSTED',
                cacheDbClient: RedisConnector,
                params: { url: url }
            });

            return command
                .saveToCache()
                .then(() => DynamoDBConnector.saveCommand(command))
                .then(() => command)
        }

        return S3Connector
            .getSignedURL(opts)
            .then(createCommand)
    }

    function createGetHrDataCommand(req) {
        let opts = {
            operation: "DOWNLOAD",
            repository: "HR_DATA",
            filename: req.body.params.filename
        };

        Logger.debug("createGetHrDataCommand()", {Opts: opts});

        function createCommand(url) {
            let command = new Command({
                deviceId: req.params.deviceId,
                commandId: uuid.v4(),
                commandCode: req.body.commandCode,
                runState: 'POSTED',
                cacheDbClient: RedisConnector,
                params: { url: url }
            });

            return command
                .saveToCache()
                .then(() => DynamoDBConnector.saveCommand(command))
                .then(() => command)
        }

        return S3Connector
            .getSignedURL(opts)
            .then(createCommand)
    }

    function createSetCfgFileCommand(req) {
        let opts = {
            operation: "DOWNLOAD",
            repository: "CONFIG_FILES",
            filename: req.body.params.filename
        };
        Logger.debug("createSetCfgFileCommand()", {Opts: opts});

        function createCommand(url) {
            let command = new Command({
                deviceId: req.params.deviceId,
                commandId: uuid.v4(),
                commandCode: req.body.commandCode,
                runState: 'POSTED',
                cacheDbClient: RedisConnector,
                params: { url: url }
            });

            return command
                .saveToCache()
                .then(() => DynamoDBConnector.saveCommand(command))
                .then(() => command)
        }

        return S3Connector
            .getSignedURL(opts)
            .then(createCommand)
    }
    //endregion Private Methods

    //region Public Methods
    this.create = function(req) {
        return Q.Promise(function(resolve, reject){
            switch (req.body.commandCode) {
                case CMD_CODES.FW_UPGRADE:
                    createFwUpgradeCommand(req)
                        .then(resolve)
                        .catch(reject);
                    break;
                case CMD_CODES.SET_CFG_FILE:
                    createSetCfgFileCommand(req)
                        .then(resolve)
                        .catch(reject);
                    break;
                case CMD_CODES.CALIBRATE:
                case CMD_CODES.GET_REG_DATA:
                case CMD_CODES.GET_HR_DATA:
                case CMD_CODES.TI_RESET:
                case CMD_CODES.SOM_RESET:
                case CMD_CODES.GET_CFG_FILE:
                case CMD_CODES.PREPARE_HR_DATA:
                case CMD_CODES.COUNTER_RESET:
                case CMD_CODES.SET_CFG_PARAMS:
                case CMD_CODES.GET_CFG_PARAMS:
                    createCommonCommand(req)
                        .then(resolve)
                        .catch(reject);
                    break;
                default:
                    reject(new Error("Failed to create command.Unknown command code."));
            }
        })
    };
    //endregion
}

module.exports = new CommandFactory();