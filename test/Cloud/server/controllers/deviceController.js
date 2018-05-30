'use strict';

const MqttConnector     = require('../modules/MqttConnector');
const RedisConnector    = require('../modules/RedisConnector');

exports.getDevices = function (req, res, next) {

    function filterByName(devices) {
        if (req.app.get('env') !== 'development') {
            return devices.filter((dev) => {
                return !(dev.thingName.startsWith("NGM") || dev.thingName.startsWith("TEST"))
            })
        } else {
            return devices
        }
    }

    MqttConnector
        .getDeviceList()
        .then(filterByName)
        .then(res.json.bind(res))
        .catch(next);
};

exports.getDeviceStatusById = function (req, res, next) {
    //TODO Fetching only from cache now!
    let key = "devices:" + req.params.deviceId + ":status";

    RedisConnector
        .get(key)
        .then(res.json.bind(res))
        .catch(next);
};

exports.getDeviceRegularDataById = function(req, res, next) {
    //TODO Fetching only from cache now!
    let key = "devices:" + req.params.deviceId + ":regular";

    RedisConnector
        .get(key)
        .then(res.json.bind(res))
        .catch(next);
};

exports.getDevicesStatus = function (req, res, next) {
    //TODO Fetching only from cache now
    let keyPattern = 'devices:*:status';

    RedisConnector
        .keys(keyPattern)
        .then((keys) => RedisConnector.mget(keys))
        .then((reply) => reply.map((status) => JSON.parse(status)))
        .then(res.json.bind(res))
        .catch(next)
};

exports.getDevicesLocation = function (req, res, next) {

    const keyPattern = 'devices:*:regular';

    function filterLocation(data) {
        return data.map(function (device) {
            return {
                deviceId: device.deviceId,
                location: device.payload.sensors.find((sensor) => sensor.name.toLowerCase() === "gps")
            }
        })
    }
    
    RedisConnector
        .keys(keyPattern)
        .then((keys) => RedisConnector.mget(keys))
        .then((reply) => reply.map((regular) => JSON.parse(regular)))
        .then(filterLocation)
        .then(res.json.bind(res))
        .catch(next)
};

exports.getDevicesPresence = function (req, res, next) {

    const keyPattern = 'devices:*:presence';

    RedisConnector
        .keys(keyPattern)
        .then((keys) => RedisConnector.mget(keys))
        .then((reply) => reply.map((presence) => JSON.parse(presence)))
        .then(res.json.bind(res))
        .catch(next)
};

exports.getDevicePresenceById = function (req, res, next) {

    const key = "devices:" + req.params.deviceId + ":presence";

    RedisConnector
        .get(key)
        .then(res.json.bind(res))
        .catch(next);
};

exports.updateDeviceShadow = function (req, res, next) {

    let params = {
        thingName: req.params.deviceId,
        payload: JSON.stringify(req.body.payload)
    };

    MqttConnector
        .updateDeviceShadow(params)
        .then(res.json.bind(res))
        .catch(next)
};

exports.getDeviceShadow = function (req, res, next) {

    let params = {
        thingName: req.params.deviceId
    };

    MqttConnector
        .getDeviceShadow(params)
        .then(res.json.bind(res))
        .catch(next);
};