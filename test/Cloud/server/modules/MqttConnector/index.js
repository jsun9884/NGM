'use strict';

const AWS         = require('aws-sdk');
const Config      = require('config');
const Q           = require('q');

function MqttConnector() {

    //region Init
    AWS.config.update({region: 'us-west-2'});
    const iot = new AWS.Iot();
    const iotDataService = new AWS.IotData(Config.AwsIotData);
    const DEFAULT_QOS = 0;
    //endregion

    //region Private Methods
    function extractValue(value) {
        return function (object) {
            return object[value] || false;
        };
    }

    function getThingShadow(opts) {
        return new Promise(function (resolve, reject) {
            let params = opts || {};
            iotDataService.getThingShadow(params, function (err, data) {
                err ? reject(err) : resolve(data)
            });
        })
    }
    //endregion

    //region Public Methods
    this.getDeviceList = function (opts) {
        //TODO Fix listThings is iterative query, if more then 250 devices,need to iterate.
        return new Promise(function (resolve, reject) {
            let params = opts || {
                    maxResults: 250
                };
            iot.listThings(params, function(err, data) {
                err ? reject(err) : resolve(extractValue('things')(data))
            });
        })
    };

    this.updateDeviceShadow = function (opts) {
        return new Promise(function(resolve, reject){
            let params = opts || {};
            iotDataService.updateThingShadow(params, function (err, data) {
                err ? reject(err) : resolve(data)
            });
        })
    };

    this.getDeviceShadow = function (opts) {
        return getThingShadow(opts)
            .then(extractValue('payload'))
            .then((payload) => JSON.parse(payload))
    };

    this.publishMessage = function (topic, message) {
        return Q.Promise(function (resolve, reject) {
            let params = {
                topic: topic,
                payload: JSON.stringify(message),
                qos: DEFAULT_QOS
            };

            iotDataService.publish(params, function (err, data) {
                err ? reject(err) : resolve(data)
            });
        });
    };
    //endregion Public Methods
}

module.exports = new MqttConnector();