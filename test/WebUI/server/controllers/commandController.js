'use strict';

var uuid    = require('uuid');
var logger  = require('../util/logger');
var device  = require('../lib/Device');
var Command = require('../lib/Command');

exports.sendDeviceCommand = function (req, res, next) {

    var _command = null;

    function createCommand(req) {
        _command  = new Command({
            commandId       : uuid.v4(),
            commandCode     : req.body.commandCode,
            commandParams   : req.body.commandParams,
            params          : req.body.params || {},
            runState        : 'POSTED'
        });
    }

    function sendCommandToDevice() {
        return device.sendCommandToDevice(_command)
    }

    function sendResponse() {
        res.header('Cache-Control', 'private, no-cache, no-store, must-revalidate');
        res.header('Expires', '-1');
        res.header('Pragma', 'no-cache');
        res.json({
            commandId : _command.commandId
        })
    }

    createCommand(req);
    sendCommandToDevice();
    sendResponse();

    //createCommand(req)
    //    .then(sendCommandToDevice)
    //    .then(sendResponse)
    //    .catch(next)
};

exports.getCommandStateById = function (req, res, next) {
    next(new Error("Not implemented"))
};