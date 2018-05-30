'use strict';

const moment            = require('moment');
const Logger            = require('../../util/logger')(module);
const RedisConnector    = require('../RedisConnector');
const MqttConnector     = require('../MqttConnector');

const PRESENCE_MONITOR_INTERVAL_MS = 1000;
const FETCH_DEVICE_LIST_INTERVAL_MS = 1000 * 60 * 10;
const OFFLINE_INTERVAL_MS = 1000 * 60;

let registryThingsList = []; // All devices from AWS Device Registry

function fetchDevicesListTask() {

    function filterByName(devices) {
        if (process.env.NODE_ENV !== 'development') {
            return devices.filter((dev) => {
                return !(dev.thingName.startsWith("NGM") || dev.thingName.startsWith("TEST"))
               // return !(dev.thingName.startsWith("HV34"))
            })
        } else {
            return devices
        }
    }

    MqttConnector
        .getDeviceList()
        .then(filterByName)
        .then((devices) => registryThingsList = devices)
        .catch((error) => Logger.error("fetchDevicesList()", error));
}

function presenceMonitorTask() {

    const keyPattern = 'devices:*:presence';

    function updatePresenceState(presenceDeviceList) {

        const now = moment().valueOf();
        const keyValueArray = [];

        registryThingsList.map(function(thing) {
            const deviceOnlineStatus = presenceDeviceList.find((d) => thing.thingName.toUpperCase() === d.deviceId);
            const key = "devices:{deviceId}:presence".replace("{deviceId}", thing.thingName.toUpperCase());
            const value = {
                deviceId: thing.thingName.toUpperCase(),
                serverRecieveTime: 0,
                isOnline: false
            };

            if (deviceOnlineStatus) {
                value.isOnline = Math.abs(deviceOnlineStatus.serverRecieveTime - now) <= OFFLINE_INTERVAL_MS;
                value.serverRecieveTime = deviceOnlineStatus.serverRecieveTime;
            }

            keyValueArray.push(key);
            keyValueArray.push(JSON.stringify(value));
        });

        return keyValueArray;
    }

    if (registryThingsList && registryThingsList.length) {

        RedisConnector
            .keys(keyPattern)
            .then((keys) => RedisConnector.mget(keys))
            .then((reply) => reply.map((r) => JSON.parse(r)))
            .then(updatePresenceState)
            .then((keyValueArray) => RedisConnector.mset(keyValueArray))
            .catch((error) => Logger.error("presenceMonitorTask()", error))
    }
}

let interval = setInterval(presenceMonitorTask, PRESENCE_MONITOR_INTERVAL_MS);
let fetchDevicesInterval = setInterval(fetchDevicesListTask, FETCH_DEVICE_LIST_INTERVAL_MS);


function exit() {
    clearInterval(interval);
    clearInterval(fetchDevicesInterval);
    process.exit()
}

function StartMonitor() {
    Logger.info('DevicePresenceMonitor started.');
    fetchDevicesListTask()
}

// Handle CTRL+C Signal
process.on('SIGINT', function () {
    Logger.info('DevicePresenceMonitor. SIGINT Received.');
    exit();
});

// Handle KILL Signal
process.on('SIGTERM', function () {
    Logger.info('DevicePresenceMonitor. SIGTERM Received.');
    exit();
});


// Start Tasks.
StartMonitor();