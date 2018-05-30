/*global require, module */
'use strict';

var Q                               = require('q');
var bleno                           = require('bleno');
var uuid                            = require('uuid');
var path                            = require('path');
var fs                              = require('fs');
var xml2js                          = require('xml2js');
var Command                         = require('../Command');
var Device                          = require('../Device');
var logger                          = require('../../util/logger');
var CommandMonitor                  = require('../CommandMonitor');


var ReadOnlyCharacteristic          = require('./Charachteristics').MqttReadOnlyCharacteristic;
var ControlCharacteristic           = require('./Charachteristics').MqttDeviceControlCharacteristic;

function BleAdapter(opts) {

    //region Init
    var self = this;

    var DeviceDataServiceCfg        = opts.services.DeviceDataService;
    var DeviceControlServiceCfg     = opts.services.DeviceControlService;

    var BLE_STATES = {
        BLE_STATE_UNKNOWN:      "unknown",
        BLE_STATE_RESETTING:    "resetting",
        BLE_STATE_UNSUPPORTED:  "unsupported",
        BLE_STATE_UNAUTHORIZED: "unauthorized",
        BLE_STATE_POWERED_OFF:  "poweredOff",
        BLE_STATE_POWERED_ON:   "poweredOn"
    };

    var CONFIG_FILE_PATH = '/home/linaro/Configuration.xml';
    var BLE_DEVICE_NAME = opts.services.DeviceShortName;

    fs.readFile(CONFIG_FILE_PATH, function(err, data) {
        if (err) {
            logger.error("Error. Failed to read configuration XML file: ", err)
        } else {
            var parser = new xml2js.Parser();
            parser.parseString(data, function (err, result) {
                if (err) {
                    logger.error("Error. Failed to parse configuration XML file: ", err)
                } else {
                    try {
                        var json = JSON.parse(JSON.stringify(result));
                        logger.info("XML Config:", {Config: json});

                        BLE_DEVICE_NAME = json.Device.DeviceId || opts.services.DeviceShortName;
                    }
                    catch(err) {
                        logger.error("Error. Failed to convert XML configuration file to JSON:", {Config: json} )
                    }
                }
            });
        }
    });

    // TODO Add read BLE_DEVICE_NAME from other config file of ngm firmware

    var BLE_BASE_UUID           = opts.services.DeviceBaseUUID;
    var BLENO_DEVICE_NAME       = 'NGM_BLENO_DEVICE_NAME';
    var BLE_CUURRENT_STATE      = BLE_STATES.BLE_STATE_UNKNOWN;

    var DeviceDataPrimaryService = null;
    var DeviceControlService = null;

    var DeviceStatusCharacteristic              = null,
        DeviceSeismicDataCharacteristic         = null,
        DevicePowerPhase1Characteristic         = null,
        DevicePowerPhase2Characteristic         = null,
        DevicePowerPhase3Characteristic         = null,
        DeviceTemperatureDataCharacteristic     = null,
        DeviceDiscloseSensorDataCharacteristic  = null,
        DeviceLocationDataCharacteristic        = null,
        DeviceControlCharacteristic             = null;

    var commandMonitorFnInterval = null;

    var IsInitialized = false;
    //endregion

    //region Private Methods
    function Initialize() {
        return Q.Promise(function (resolve, reject) {
            if (!IsInitialized) {

                // Status CH
                DeviceStatusCharacteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.DeviceStatusCh);

                // Control CH
                DeviceControlCharacteristic = new ControlCharacteristic(DeviceControlServiceCfg.DeviceControlCh);

                // Data
                DevicePowerPhase1Characteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.DevicePowerDataPhase1Ch);
                DevicePowerPhase2Characteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.DevicePowerDataPhase2Ch);
                DevicePowerPhase3Characteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.DevicePowerDataPhase3Ch);

                DeviceSeismicDataCharacteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.SeismicDataCh);
                DeviceTemperatureDataCharacteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.TemperatureDataCh);
                DeviceLocationDataCharacteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.LocationDataCh);
                DeviceDiscloseSensorDataCharacteristic = new ReadOnlyCharacteristic(DeviceDataServiceCfg.DiscloseSensorDataCh);

                DeviceDataPrimaryService = new bleno.PrimaryService({
                    uuid: DeviceDataServiceCfg.ServiceUUID,
                    characteristics: [
                        DeviceStatusCharacteristic,
                        DevicePowerPhase1Characteristic,
                        DevicePowerPhase2Characteristic,
                        DevicePowerPhase3Characteristic,
                        DeviceSeismicDataCharacteristic,
                        DeviceTemperatureDataCharacteristic,
                        DeviceLocationDataCharacteristic,
                        DeviceDiscloseSensorDataCharacteristic
                    ]
                });

                DeviceControlService = new bleno.PrimaryService({
                    uuid: DeviceControlServiceCfg.ServiceUUID,
                    characteristics: [DeviceControlCharacteristic]
                });

                RegisterEventHandlers();
                IsInitialized = true;
            }
            resolve(true);
        })
    }

    function RegisterEventHandlers(){
        bleno.on('stateChange', stateChangeHandler);
        bleno.on('advertisingStart', advertisingStartHandler);
        bleno.on('advertisingStartError', advertisingStartErrorHandler);
        bleno.on('advertisingStop', advertisingStopHandler);
        bleno.on('servicesSet', servicesSetHandler);
        bleno.on('servicesSetError', servicesSetErrorHandler);
        bleno.on('disconnect', clientDisconnectHandler); // Linux only
        bleno.on('accept', clientConnAcceptHandler); // not available on OS X 10.9
        bleno.on('rssiUpdate', rssiUpdateHandler); // not available on OS X 10.9

        Device.on('STATUS_MSG', onStatusMsgHandler);
        Device.on('REGULAR_DATA_MSG', onRegularDataMsgHadnler);

        DeviceControlCharacteristic.on('BLE_CMD_RECEIVED', bleCommandReceivedHandler);
    }

    function BleCommandStateMonitor(commandId) {
        var state = CommandMonitor.getCommandStateById(commandId);
        logger.debug("BLE ADAPTER: BleCommandStateMonitor", {CommandState: state});

        if (state) {
            self.updateCommandStateValue(state)
        } else {
            logger.debug("BLE ADAPTER: BleCommandStateMonitor finished.");
            clearInterval(commandMonitorFnInterval);
            commandMonitorFnInterval = null;
        }
    }

    //endregion

    //region EventHandlers
    function stateChangeHandler(state) {
        logger.info("BLE ADAPTER stateChangeHandler. New State: ", {State: state});
        BLE_CUURRENT_STATE = state;
        switch (BLE_CUURRENT_STATE)
        {
            case BLE_STATES.BLE_STATE_POWERED_ON:
                bleno.startAdvertising(BLE_DEVICE_NAME, [BLE_BASE_UUID]);
                break;
            case BLE_STATES.BLE_STATE_POWERED_OFF:
                self.BleStop();
                break;
            default:
                break;
        }

    }

    function advertisingStartHandler(error) {
        logger.info("BLE ADAPTER: advertisingStartHandler.");

        if (!error) {
            bleno.setServices([DeviceDataPrimaryService, DeviceControlService]);
        }
    }

    function advertisingStartErrorHandler(error){
        logger.error("BLE ADAPTER: advertisingStartErrorHandler", error);
        // TODO Implement
    }

    function advertisingStopHandler(){
        logger.info("BLE ADAPTER: advertisingStopHandler");

        // TODO Implement
    }

    function servicesSetHandler(error){
        logger.info("BLE ADAPTER: servicesSetHandler");
        if (error) {
            logger.error("BLE ADAPTER: servicesSetHandler Failed", error);
        }
        // TODO Implement
    }

    function servicesSetErrorHandler(error){
        logger.error("BLE ADAPTER: servicesSetErrorHandler", error);
        // TODO Implement
    }

    function clientDisconnectHandler(clientAddress){
        logger.info("BLE ADAPTER: clientDisconnectHandler", { client : clientAddress });
        // TODO Implement
        // emit BLE_CLIENT_DISCONNECTED
    }

    function clientConnAcceptHandler(clientAddress){
        logger.info("BLE ADAPTER: clientConnAcceptHandler", {client: clientAddress});
        // TODO Implement
        // Emit BLE_CLIENT_CONNECTED
    }

    function rssiUpdateHandler(rssi){
        logger.info("BLE ADAPTER: rssiUpdateHandler", {RSSI: rssi});
        // TODO Implement
    }

    function bleCommandReceivedHandler(req) {
        logger.debug("BLE ADAPTER: bleCommandReceivedHandler", {Command: req});

        var command  = new Command({
            commandId: req.commandId,
            commandCode: req.commandCode,
            params: req.params || {}
        });

        Device.sendCommandToDevice(command);


        //
        // if (commandMonitorFnInterval) {
        //     logger.warn("BLE ADAPTER: bleCommandReceivedHandler." +
        //         " BLE command is running. Changing monitoring handler.");
        //     clearInterval(commandMonitorFnInterval)
        // }
        //
        // var _command  = new Command({
        //     commandId       : uuid.v4(),
        //     commandCode     : req.commandCode,
        //     commandParams   : req.commandParams || {},
        //     runState        : 'POSTED'
        // });
        //
        // // TODO promisify this
        // CommandMonitor.createMonitorTask(_command);
        // Device.sendCommandToDevice(_command);
        //
        // commandMonitorFnInterval = setInterval(BleCommandStateMonitor, 1000, _command.commandId);
    }

    function onStatusMsgHandler(msg) {
        self.updateStatusValue(msg)
    }

    function onRegularDataMsgHadnler(msg) {

        function getSeismicData(msg) {
            return {
                "deviceId": msg.deviceId,
                "payload": {
                    "sensors": msg.payload.sensors.filter(function (sensor) {
                            return sensor.name == 'extAcc' || sensor.name == 'obAcc'
                    })
                },
                "timestamp": msg.timestamp
            }
        }

        function getPowerData(msg) {

            function isObject(val) {
                return (typeof val === 'object');
            }

            function getPhase(msp430, phaseNumber) {

                var phase = {};

                var active_power_key = "active_power" + phaseNumber;
                var apparent_power_key = "apparent_power" + phaseNumber;
                var current_key = "current" + phaseNumber;
                var power_factor_key = "power_factor" + phaseNumber;
                var frequency_key = "frequency" + phaseNumber;
                var reactive_power_key = "reactive_power" + phaseNumber;
                var voltage_key = "voltage" + phaseNumber;

                phase["name"] = msp430["name"];
                phase["state"] = msp430["state"];
                phase["timestamp"] = msp430["timestamp"];
                phase[active_power_key] = msp430[active_power_key];
                phase[apparent_power_key] = msp430[apparent_power_key];
                phase[current_key] = msp430[current_key];
                phase[power_factor_key] = msp430[power_factor_key];
                phase[frequency_key] = msp430[frequency_key];
                phase[reactive_power_key] = msp430[reactive_power_key];
                phase[voltage_key] = msp430[voltage_key];

                return phase;
            }


            var msp430 = msg.payload.sensors.find(function (sensor) {
                return sensor.name == 'msp430'
            });

            Object.keys(msp430).map(function (key) {
                    if (isObject(msp430[key])) {
                        msp430[key] = msp430[key]["current_value"]
                    }
            });

            return {
                PHASE1: {
                    "deviceId": msg.deviceId,
                    "payload": {
                        "sensors": [getPhase(msp430, 1)]
                    },
                    "timestamp": msg.timestamp
                },
                PHASE2: {
                    "deviceId": msg.deviceId,
                    "payload": {
                        "sensors": [getPhase(msp430, 2)]
                    },
                    "timestamp": msg.timestamp
                },
                PHASE3: {
                    "deviceId": msg.deviceId,
                    "payload": {
                        "sensors": [getPhase(msp430, 3)]
                    },
                    "timestamp": msg.timestamp
                }
            }
        }

        function getLocationData(msg) {
            return {
                "deviceId": msg.deviceId,
                "payload": {
                    "sensors": msg.payload.sensors.filter(function (sensor) {
                        return sensor.name == 'gps'
                    })
                },
                "timestamp": msg.timestamp
            }
        }

        function getTemperatureData(msg) {
            return {
                "deviceId": msg.deviceId,
                "payload": {
                    "sensors": msg.payload.sensors.filter(function (sensor) {
                        return sensor.name == 'cpuTemp' || sensor.name == 'temperature'
                    })
                },
                "timestamp": msg.timestamp
            }
        }

        function getDiscloseSensorData(msg) {
            return {
                "deviceId": msg.deviceId,
                "payload": {
                    "sensors": msg.payload.sensors.filter(function (sensor) {
                        return sensor.name == 'disclosingSensor'
                    })
                },
                "timestamp": msg.timestamp
            }
        }

        var clone = JSON.parse(JSON.stringify(msg));

        const seismicData = getSeismicData(clone);
        const powerData = getPowerData(clone);
        const locationData = getLocationData(clone);
        const temperatureData = getTemperatureData(clone);
        const discloseSensorData = getDiscloseSensorData(clone);

        self.updatePowerDataValue(powerData.PHASE1, powerData.PHASE2, powerData.PHASE3);
        self.updateLocationDataValue(locationData);
        self.updateTemperatureValue(temperatureData);
        self.updateSeismicDataValue(seismicData);
        self.updateDiscloseSensorValue(discloseSensorData);
    }
    //endregion

    //region Public Methods
    this.BleStart = function () {
        logger.info("BLE ADAPTER:   BleStart.");
        Initialize()
            .delay(3000)
            .then(function(){
                if (BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
                    logger.debug("BLE ADAPTER:    startAdvertising.");
                    process.env['BLENO_DEVICE_NAME'] = BLENO_DEVICE_NAME; // TODO Check
                } else {
                    logger.debug("BLE ADAPTER:    Invalid BLE State. Not Starting.");
                }
            });
    };

    this.BleStop = function() {
        bleno.stopAdvertising();
    };

    this.updateStatusValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceStatusCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateStatusValue failed. BLE not initialized")
        }
    };

    this.updatePowerDataValue = function (phase1, phase2, phase3) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DevicePowerPhase1Characteristic.updateValue(phase1);
            DevicePowerPhase2Characteristic.updateValue(phase2);
            DevicePowerPhase3Characteristic.updateValue(phase3);
        } else {
            logger.debug("BleAdapter updateRegularDataValue failed. BLE not initialized.", {
                IsInitialized: IsInitialized,
                State: BLE_CUURRENT_STATE
            } )
        }
    };

    this.updateSeismicDataValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceSeismicDataCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateSeismicDataValue failed. BLE not initialized")
        }
    };

    this.updateLocationDataValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceLocationDataCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateLocationDataValue failed. BLE not initialized")
        }
    };

    this.updateTemperatureValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceTemperatureDataCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateTemperatureValue failed. BLE not initialized")
        }
    };

    this.updateDiscloseSensorValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceDiscloseSensorDataCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateTemperatureValue failed. BLE not initialized")
        }
    };

    this.updateCommandStateValue = function (value) {
        if (IsInitialized && BLE_CUURRENT_STATE == BLE_STATES.BLE_STATE_POWERED_ON) {
            DeviceControlCharacteristic.updateValue(value);
        } else {
            logger.debug("BleAdapter updateCommandStateValue failed. BLE not initialized")
        }
    };

    //endregion
}

module.exports = new BleAdapter({ services : require('./bleServiceConfig.json')});