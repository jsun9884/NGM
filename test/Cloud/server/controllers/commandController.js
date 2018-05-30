'use strict';

const Q               = require('q');
const uuid            = require('uuid');
const config          = require('config');

const Logger          = require('../util/logger')(module);
const MqttConnector   = require('../modules/MqttConnector');
const LambdaConnector = require('../modules/LambdaConnector');
const RedisConnector  = require('../modules/RedisConnector');
const CommandMonitor  = require('../modules/Command/CommandMonitor');
const CommandFactory  = require('../modules/Command/CommandFactory'); 

exports.sendDeviceCommand = function (req, res, next) {

    function validate(req) {
        return Q.Promise(function(resolve, reject){
            if (req.params.deviceId && req.body.commandCode) {
                resolve(req)
            } else {
                reject(new Error("Validation Failed. Invalid request."))
            }
        })
    }

    function createCommand(req) {
        return CommandFactory.create(req);
    }

    function createMonitorTask(command) {
        Logger.debug("createMonitorTask", {CommandId: command.commandId});
        return CommandMonitor
            .createMonitorTask(command)
            .then(() => command);
    }

    function sendCommandToDevice(command) {
        Logger.debug("sendCommandToDevice", {CommandId: command.commandId});
        let message = {
                commandId       : command.commandId,
                commandCode     : command.commandCode,
                params          : command.params || {}
            },
            topic = 'iot/device/' + command.deviceId + '/command';
        return MqttConnector
            .publishMessage(topic, message)
            .then(function(){
                return command
            })
    }

    function sendResponse(command){
        Logger.debug("sendResponse", {CommandId: command.commandId});
        res.header('Cache-Control', 'private, no-cache, no-store, must-revalidate');
        res.header('Expires', '-1');
        res.header('Pragma', 'no-cache');
        res.json({
            commandId : command.commandId
        })
    }

    function generalErrorHandler(error){
        Logger.error("generalErrorHandler", error);
        switch (error.type) {
            case 'ValidationError':
            case 'RedisUpdateError':
            case 'MqttPostError':
                res.status(400).json({
                    errorType : error.type,
                    message: error.message
                });
                break;
            default:
                next(error);
        }
    }

    function execute(command) {
        if (command.commandCode === 330) {
            // This is special command relevant, for UI purpose, request to prepare HR data in cloud.
            return LambdaConnector
                .invokePrepareHRData(command)
                .then(function(){return command})
        } else {
            return createMonitorTask(command)
                .catch(function (error) {
                        if (error.rethrow) {
                            throw error
                        } else {
                            error.type = 'CommandMonitorError';
                            throw error;
                        }
                    })
                .then(sendCommandToDevice)
                .catch(function(error){
                        if (error.rethrow) {
                            throw error
                        } else {
                            error.type = 'MqttPostError';
                            throw error;
                        }
                    })
        }
    }

    validate(req)
        .fail(function (error) {
            error.rethrow = true;
            error.type = 'ValidationError';
            throw error;
        })
        .then(createCommand)
        .fail(function(error){
            if (error.rethrow) {
                throw error
            } else {
                error.rethrow = true;
                error.type = 'CreateCommandError';
                throw error;
            }
        })
        .then(execute)
        .then(sendResponse)
        .catch(generalErrorHandler);
};

exports.getCommandStateById = function (req, res, next) {

    function validate(req) {
        return Q.Promise(function(resolve, reject){
            if (req.params.deviceId && req.query.commandId) {
                resolve(req)
            } else {
                reject(new Error("Validation Failed.Invalid request."))
            }
        })
    }

    function getCommandStateFromCache(req) {
        let key = 'devices:' + req.params.deviceId + ':commands:' + req.query.commandId;
        Logger.debug("getCommandStateFromCache()",{ Key: key});
        return RedisConnector.get(key)
    }

    validate(req)
        .fail(function(error){
            error.rethrow = true;
            error.type = 'ValidationError';
            throw error;
        })
        .then(getCommandStateFromCache)
        .then(function(state){
            Logger.debug("getCommandStateById", {State : state});
            res.header('Cache-Control', 'private, no-cache, no-store, must-revalidate');
            res.header('Expires', '-1');
            res.header('Pragma', 'no-cache');
            res.json(state);
        })
        .catch(next)
};