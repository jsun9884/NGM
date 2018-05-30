"use strict";

const router = require('express').Router();
const validators = require('../modules/Validators');

const deviceController = require('../controllers/deviceController');
const commandController = require('../controllers/commandController');
const historyController = require('../controllers/historyController');

router.route('/devices')
    .get(deviceController.getDevices);

router.route('/devices/status')
    .get(deviceController.getDevicesStatus);

router.route('/devices/location')
    .get(deviceController.getDevicesLocation);

router.route('/devices/presence')
    .get(deviceController.getDevicesPresence);

router.route('/devices/:deviceId/presence')
    .get(deviceController.getDevicePresenceById);

router.route('/devices/:deviceId/status')
    .get(deviceController.getDeviceStatusById);

router.route('/devices/:deviceId/regular')
    .get(deviceController.getDeviceRegularDataById);

router.route('/devices/:deviceId/config')
    .get(deviceController.getDeviceShadow)
    .post(validators.updateDeviceShadowValidator, deviceController.updateDeviceShadow);

router.route('/devices/:deviceId/command')
    .post(validators.sendCommandValidator, commandController.sendDeviceCommand)
    .get(commandController.getCommandStateById);

router.route('/devices/:deviceId/files')
    .post(historyController.fileAccepted);

module.exports = router;
