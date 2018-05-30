'use strict';

var device = require('../lib/Device');

exports.getDevice = function (req, res, next) {
    next(new Error("Not implemented"))
};

exports.getDeviceStatus = function (req, res, next) {
    res.json(device.deviceDataStore.status);
};

exports.getDeviceRegularData = function (req, res, next) {
    res.json(device.deviceDataStore.regular);
};

exports.updateDeviceConfig = function (req, res, next) {

    var config = req.body.payload;

    device
        .sendConfigToDevice(config)
        .then(res.json.bind(res))
        .catch(next)
};

exports.getDeviceConfig = function (req, res, next) {
    device.getConfig(res, false)
};