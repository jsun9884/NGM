/*global require, module */
'use strict';

var mqtt                = require('mqtt');
var util                = require('util');
var Q                   = require('q');
var uuid                = require('uuid');
var EventEmitter        = require('events').EventEmitter;
var logger              = require('../util/logger');

function Device() {

    //region Variable declaration and Initialization
    var DEVICE_TOPICS = {
            STATUS_MSG: "device/status",
            REGULAR_DATA_MSG: "device/regular",
            SEND_COMMAND: 'device/command',
            ACCEPTED_COMMAND: 'device/command/accepted',
            REJECTED_COMMAND: 'device/command/rejected',
            EVENT_BT_PRESENCE: 'device/events/bluetooth/presence',
            UPDATE_CONFIG: 'device/config/update',
            REQUEST_ADD_WIFI_NETWORK: 'device/networks/add',
            REQUEST_DELETE_WIFI_NETWORK: 'device/networks/delete',
            REQUEST_DEVICE_CONFIG: 'device/config/get',
            REPLY_DEVICE_CONFIG: 'device/config/get/#',
            REQUEST_DEVICE_NETWORKS: 'device/networks/get',
            REPLY_DEVICE_NETWORKS: 'device/networks/get/#'
        };

    var REPLY_DEVICE_CONFIG_TOPIC_RE = /^device\/config\/get\/.*/;
    var REPLY_DEVICE_NETWORKS_TOPIC_RE = /^device\/networks\/get\/.*/;

    var pendingConfigRequests = [];
    var pendingNetworkRequests = [];

    var self = this,
        client  = mqtt.connect('mqtt://localhost'),
        connected = false;

    self.MQTT_DEVICE_TOPCIS = DEVICE_TOPICS;

    self.deviceDataStore = {
        status:     {},
        regular:    {}
    };

    self.COMMAND_EVENTS = {
        MSG_CMD_REJECTED: 'MSG_CMD_REJECTED',
        MSG_CMD_ACCEPTED: 'MSG_CMD_ACCEPTED'
    };

    self.DATA_EVENTS = {
        MSG_STATUS_RECIEVED: 'STATUS_MSG',
        MSG_REGDATA_RECIEVED: 'REGULAR_DATA_MSG',
        MSG_READ_CONFIG_REPLY_RECIEVED: 'READ_CONFIG_REPLY',
        MSG_READ_NETWORKS_REPLY_RECEIVED: 'READ_NETWORKS_REPLY'
    };

    EventEmitter.call(this);
    //endregion

    //region Event Handlers
    client.on('connect', function() {
        connected = true;
        logger.info('Device connected to local MQTT Broker.');

        var topics = Object.keys(DEVICE_TOPICS).map(function (key) { return DEVICE_TOPICS[key]});

        client.subscribe(topics, function(err){
            if (err) {
                logger.debug("MQTT subscribe failed.", err)
            } else {
                logger.debug("Device Subscribed to MQTT topics.", {Topics: topics});
            }
        });
    });

    client.on('message', function(topic, message) {

        if (topic.match(REPLY_DEVICE_CONFIG_TOPIC_RE)) {
            handleDeviceConfigReply(topic, message)
        }

        if (topic.match(REPLY_DEVICE_NETWORKS_TOPIC_RE)) {
            handleDeviceNetworksReply(topic, message)
        }

        switch (topic) {
            case DEVICE_TOPICS.STATUS_MSG:
                handlerDeviceStatusMsg(message);
                break;
            case DEVICE_TOPICS.REGULAR_DATA_MSG:
                handleDeviceRegularMsg(message);
                break;
            case DEVICE_TOPICS.ACCEPTED_COMMAND:
                handleAcceptedCommandMsg(message);
                break;
            case DEVICE_TOPICS.REJECTED_COMMAND:
                handleRejectedCommandMsg(message);
                break;
            default:
                break;
        }
    });

    client.on('error', function(error) {
        logger.error('MQTT client error occurred. Error: ', error)
    });

    client.on('reconnect', function(){
        logger.info("MQTT reconnect started.");
    });
    
    client.on('close', function () {
        logger.info("MQTT connection closed.");
        connected = false;
    });

    client.on('offline', function() {
        logger.info("Mqtt client offline.");
        connected = false
    });
    //endregion Event Handlers

    //region Private Methods
    function handlerDeviceStatusMsg(message) {
        try {
            self.deviceDataStore.status = JSON.parse(message);
            // logger.debug("MQTT Status message received.", {MqttMessage: self.deviceDataStore.status});
            self.emit(self.DATA_EVENTS.MSG_STATUS_RECIEVED, self.deviceDataStore.status);
        }
        catch (err) {
            logger.error("handlerDeviceStatusMsg. Failed to parse message. Error: ", err)
        }
    }

    function handleDeviceConfigReply(topic, message) {

        function findPendingRequest(mqttRequestId) {
            var index = pendingConfigRequests.findIndex(function (pendingReq) {
                return pendingReq.MqttRequestId === mqttRequestId
            });

            if(index !== -1) {
                var pendingRequest = pendingConfigRequests[index];
                pendingConfigRequests.splice(index, 1);
                return pendingRequest;
            } else {
                logger.debug("handleDeviceConfigReply() Pending Request not found", {MqttRequestId : mqttRequestId});
                return null;
            }
        }

        try {
            var msg = JSON.parse(message);
            var mqttRequestId = topic.split("/").pop();
            
            logger.debug("handleDeviceConfigReply(). Received message:", {
                Message: msg,
                MqttRequestId : mqttRequestId});

            var pendingReq = findPendingRequest(mqttRequestId);

            // Send Reply to WebClient via Express and not fire an event
            if (pendingReq && pendingReq.IsBTRequest === false) {
                pendingReq.ExpressResponse.send(msg)
            }

            // Send Reply to BT Client - fire an event.
            if (pendingReq && pendingReq.IsBTRequest === true) {
                self.emit(self.DATA_EVENTS.MSG_READ_CONFIG_REPLY_RECIEVED, {
                    Message: msg,
                    MqttRequestId : mqttRequestId
                })
            }
        }
        catch (err) {
            logger.error("handleDeviceConfigReply(). Error occurred: ", err)
        }
    }

    function handleDeviceNetworksReply(topic, message){

        function findPendingNetworkRequest(mqttRequestId) {
            var index = pendingNetworkRequests.findIndex(function (pendingReq) {
                return pendingReq.MqttRequestId === mqttRequestId
            });

            if(index !== -1) {
                var pendingRequest = pendingNetworkRequests[index];
                pendingNetworkRequests.splice(index, 1);
                return pendingRequest;
            } else {
                logger.debug("findPendingNetworkRequest() Pending request not found", {
                    MqttRequestId : mqttRequestId
                });
                return null;
            }
        }

        try {
            var msg = JSON.parse(message);
            var mqttRequestId = topic.split("/").pop();

            logger.debug("handleDeviceNetworksReply(). Received message:", {
                Message: msg,
                MqttRequestId : mqttRequestId
            });

            var pendingReq = findPendingNetworkRequest(mqttRequestId);

            // Send Reply to Bluetooth Client - fire an event.
            if (pendingReq) {
                self.emit(self.DATA_EVENTS.MSG_READ_NETWORKS_REPLY_RECEIVED, {
                    Message: msg,
                    MqttRequestId : mqttRequestId
                })
            }
        }
        catch (err) {
            logger.error("handleDeviceNetworksReply(). Error occurred: ", err)
        }
    }

    function handleDeviceRegularMsg(message) {
        try {
            self.deviceDataStore.regular = JSON.parse(message);
            // logger.debug("MQTT Device Data message received.", {MqttMessage: self.deviceDataStore.regular});
            self.emit(self.DATA_EVENTS.MSG_REGDATA_RECIEVED, self.deviceDataStore.regular);
        }
        catch (err) {
            logger.debug("handleDeviceRegularMsg. Failed to parse message. Error: ", err)
        }
    }

    function handleAcceptedCommandMsg(message) {
        var msg = null;
        try {
            msg = JSON.parse(message);
            logger.debug("handleAcceptedCommandMsg. ", {MqttMessage : msg});
            self.emit(self.COMMAND_EVENTS.MSG_CMD_ACCEPTED, msg);
        }
        catch (err) {
            logger.debug("handleAcceptedCommandMsg. Failed to parse message. Error: ", err)
        }
    }

    function handleRejectedCommandMsg(message) {
        var msg = null;
        try {
            msg = JSON.parse(message);
            logger.debug("handleRejectedCommandMsg. ", {MqttMessage : msg});
            self.emit(self.COMMAND_EVENTS.MSG_CMD_REJECTED, msg);
        }
        catch (err) {
            logger.debug("handleRejectedCommandMsg. Failed to parse message. Error: ", err)
        }
    }

    /**
     * Publish JSON message to MQTT Local Broker.
     * @private
     */
    function publishMessage(topic, message, callback) {
        if (connected) {
            try {
                client.publish(topic, JSON.stringify(message))
            }
            catch (err) {
                if (callback && typeof(callback) === typeof(Function)) {
                    callback(err)
                }
            }
        } else {
            if (callback && typeof(callback) === typeof(Function)) {
                callback(new Error("Failed to send message. Not connected to MQTT Broker."));
            }
        }
    }
    //endregion Private Methods

    //region Public Methods

    /**
     * Publish Command message to MQTT Local Broker.
     */
    this.sendCommandToDevice = function (command) {
         var message = {
            commandId       : command.commandId || "",
            commandCode     : command.commandCode || 0,
            params          : command.params || {}
        };

        publishMessage(DEVICE_TOPICS.SEND_COMMAND, message);
    };

    /**
     * Publish Configuration JSON to Device
     * @param config
     * @return {*}
     */
    this.sendConfigToDevice = function(config) {
        logger.debug("sendConfigToDevice()", { Config: config});
        return Q.Promise(function (resolve, reject) {
            if (config) {
                publishMessage(DEVICE_TOPICS.UPDATE_CONFIG, config);
                resolve(true);
            }
            else {
                reject(new Error("Invalid configuration object."))
            }
        })
    };

    /**
     * Send request to device via Mqtt to get device configuration
     * @param {Object} res Exress.js response object
     * @param {Boolean} isBTRequest Is a request came from Bluetooth Service
     */
    this.getConfig = function(res, isBTRequest) {

        var mqttReqId = uuid.v4();
        logger.debug("getConfig()", { MqttRequestId: mqttReqId });

        pendingConfigRequests.push({
            MqttRequestId: mqttReqId,
            ExpressResponse: res,
            IsBTRequest: isBTRequest || false
        });

        publishMessage(DEVICE_TOPICS.REQUEST_DEVICE_CONFIG, {
            MqttRequestId: mqttReqId
        });
    };

    /**
     * Send request to device via Mqtt to get Network Manager connections and available wifi networks list.
     */
    this.requestDeviceNetworks = function () {

        var mqttReqId = uuid.v4();
        logger.debug("requestDeviceNetworks()", { MqttRequestId: mqttReqId });

        pendingNetworkRequests.push({ MqttRequestId: mqttReqId });
        publishMessage(DEVICE_TOPICS.REQUEST_DEVICE_NETWORKS, { MqttRequestId: mqttReqId });
    };

    /**
     * Publish bluetooth client presence event to device via Mqtt
     * @param event
     */
    this.sendBtEventToDevice = function (event) {
        logger.debug("sendBtEventToDevice()", { Event: event });
        publishMessage(DEVICE_TOPICS.EVENT_BT_PRESENCE, event);
    }

    /**
     * Send message to MQTT
     * @param topic
     * @param message
     */
    this.sendMessage = function (topic, message) {
        return publishMessage(topic, message)
    }
    //endregion
}

util.inherits(Device, EventEmitter);

module.exports = new Device();
