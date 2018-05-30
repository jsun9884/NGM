'use strict';

var router = require('express').Router();

var loginController     = require('../controllers/loginController');
var commandController   = require('../controllers/commandController');
var deviceController    = require('../controllers/deviceController');
var fileController      = require('../controllers/fileManagerController');

router.route('/device')
    .get(deviceController.getDevice);

router.route('/device/status')
    .get(deviceController.getDeviceStatus);

router.route('/device/regular')
    .get(deviceController.getDeviceRegularData);

router.route('/device/command')
    .post(commandController.sendDeviceCommand)
    .get(commandController.getCommandStateById);

router.route('/device/config')
    .get(deviceController.getDeviceConfig)
    .post(deviceController.updateDeviceConfig);

router.route('/files/upload')
    .post(fileController.fileUploadHandler);

router.route('/authenticate')
    .post(loginController.authenticate);

module.exports = router;