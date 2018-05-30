/*global require, module */
'use strict';

var util    = require('util');
var bleno   = require('bleno');
var logger  = require('../../util/logger');

function BleCharacteristic(config) {

    //region Init
    var self = this;
    var descriptors = [];

    BleCharacteristic.super_.call(this, {
        uuid: config.uuid,
        properties: config.properties,
        descriptors: descriptors || [],
        value: null,
        onReadRequest : onReadRequestHandler,
        onSubscribe: onSubscribeHandler,
        onUnsubscribe : onUnsubscribeHandler
    });

    this._value = new Buffer(0);
    this._updateValueCallback = null;
    //endregion

    //region BLE Handlers
    function onReadRequestHandler(offset, callback) {
        var value = self._value;
        //logger.debug("BleCharacteristic Read", { ValueLength: self._value.length, Offset: offset});
        if(offset){
            if(offset <= value.length){
                value = value.slice(offset);
                callback(this.RESULT_SUCCESS, value);
                //logger.debug('BleCharacteristic Read:', {Value: value.toString()});
            }
        }
        else {
            callback(this.RESULT_SUCCESS, value);
            // logger.debug('BleCharacteristic Read:', {Value: value.toString()});
        }
    }

    function onSubscribeHandler(maxValueSize, updateValueCallback) {
        logger.debug("Device subscribed. Max Value Size: ", {maxValueSize: maxValueSize});
        self._updateValueCallback = updateValueCallback;
    }

    function onUnsubscribeHandler() {
        logger.debug("Device unsubscribed");
        self._updateValueCallback = null;
    }
    //endregion BLE Handlers

    // region Public Methods
    this.updateValue = function(value) {

        var data = "";

        try {
            data = JSON.stringify(value)
        }
        catch (err) {
            logger.error("Failed to update value. Invalid JSON value.", err);
            return;
        }

        this._value = new Buffer(data);
        if (this._updateValueCallback) {
            this._updateValueCallback(new Buffer("MQTT"));
        }
    };
    //endregion
}

function DeviceControlCharacteristic(config) {

    //region Init
    var self = this;
    var descriptors = [];

    BleCharacteristic.super_.call(this, {
        uuid: config.uuid,
        properties: config.properties,
        descriptors: descriptors || [],
        value: null,
        onReadRequest : onReadRequestHandler,
        onWriteRequest : onWriteRequestHandler
    });

    this._value = new Buffer(0);
    this._updateValueCallback = null;
    //endregion

    //region BLE Handlers

    function onReadRequestHandler(offset, callback) {
        var value = self._value;
        //logger.debug("DeviceControlCharacteristic Read", { ValueLength: self._value.length, Offset: offset});
        if(offset){
            if(offset <= value.length){
                value = value.slice(offset);
                callback(this.RESULT_SUCCESS, value);
                //logger.debug('DeviceControlCharacteristic Read:', {Value: value.toString()});
            }
        }
        else {
            callback(this.RESULT_SUCCESS, value);
            //logger.debug('DeviceControlCharacteristic Read:', {Value: value.toString()});
        }
    }

    function onWriteRequestHandler(data, offset, withoutResponse, callback) {
        if (data.length == 0) {
            callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
        }
        else {
            try {
                var request = JSON.parse(data);
                //logger.debug("DeviceControlCharacteristic - onWriteRequestHandler", { Message : request });
                self.emit("BLE_CMD_RECEIVED", request)
            }
            catch (err) {
                logger.error("DeviceControlCharacteristic - onWriteRequestHandler Failed.", err)
            }
            callback(this.RESULT_SUCCESS);
        }
    }
    //endregion BLE Handlers

    // region Public Methods
    this.updateValue = function(value) {

        var data = "";

        try {
            data = JSON.stringify(value)
        }
        catch (err) {
            logger.debug("Failed to update value. Invalid JSON value.");
            return;
        }

        this._value = new Buffer(data);
        if (this._updateValueCallback) {
            this._updateValueCallback(new Buffer("MQTT"));
        }
    };
    //endregion

}

util.inherits(BleCharacteristic,            bleno.Characteristic);
util.inherits(DeviceControlCharacteristic,  bleno.Characteristic);

module.exports.MqttReadOnlyCharacteristic           = BleCharacteristic;
module.exports.MqttDeviceControlCharacteristic      = DeviceControlCharacteristic;