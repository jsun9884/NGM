'use strict';

const moment    = require('moment');
const Device    = require('../Device');
const logger    = require('../../util/logger');
const BTSP      = require('bluetooth-serial-port');
const Command   = require('../Command');

function BluetoothAdapter(opts) {

    //region Init
    var self = this;
    var server = new BTSP.BluetoothSerialPortServer();

    self.clientConnected = false;
    self.maxWriteError = 3;
    self.writeErrors = 0;

    const BT_PRESENCE_EVENT_TYPE = {
        CLIENT_CONNECTED: "connected",
        CLIENT_DISCONNECTED: "disconnected"
    };

    const BT_COMMAND = {
        POST_DEVICE_CONFIG: 4100, // Send config to ngm device
        GET_DEVICE_CONFIG: 4200, // Read config from ngm device
        GET_DEVICE_NETWORKS: 5200, // Get list of available networks and saved connections
        POST_DELETE_SAVED_CONNECTION: 5300, // Send delete saved connection
        POST_ADD_CONNECTION: 5400 // Add wifi network to saved connections
    };

    RegisterEventHandlers();
    //endregion

    //region Private Methods

    /**
     * Assigned event handlers to BT and Device events
     * @private
     */
    function RegisterEventHandlers() {
        Device.on(Device.DATA_EVENTS.MSG_STATUS_RECIEVED, onStatusMsgHandler);
        Device.on(Device.DATA_EVENTS.MSG_REGDATA_RECIEVED, onRegularDataMsgHandler);
        Device.on(Device.DATA_EVENTS.MSG_READ_CONFIG_REPLY_RECIEVED, onReadConfigReplyHandler);
        Device.on(Device.DATA_EVENTS.MSG_READ_NETWORKS_REPLY_RECEIVED, onReadNetworksReplyHandler);

        // Device.on('MSG_CMD_REJECTED', onRejectedCommandMsgHadnler);
        // Device.on('MSG_CMD_ACCEPTED', onAcceptedCommandMsgHadnler);

        // Emitted when data is read from the serial port connection.
        server.on('data', onBluetoothDataHandler);

        //Emitted when a connection was closed either by the user (i.e. calling close or remotely).
        server.on('closed', onBtServerClosedHandler);

        // Emitted when reading from the serial port connection results in an error. The connection is closed.
        server.on('failure', onBtServerFailureHandler);
    }

    /**
     * Write json message to BT Serial Port
     * @param msg
     * @return {Promise}
     */
    function bluetoothWriteMessage(msg) {
        return new Promise(function (resolve, reject) {
            var data = JSON.stringify(msg).concat("\r");
            server.write(new Buffer(data), function (err, bytesWritten) {
                err ? reject(err) : resolve(bytesWritten)
            });
        })
    }

    /**
     *
     * @param json
     */
    function handlePostDeviceConfigBTCommand(json) {

        function extractWifiSSID(json) {
            return json.WifiSSID || ""
        }

        function extractWifiPassword(json) {
            return json.WifiPassword || ""
        }

        var config = {
            payload: {
                state: {
                    desired: {
                        General: {
                            WifiSSID: extractWifiSSID(json),
                            WifiPassword: extractWifiPassword(json)
                        }
                    }
                }
            }
        };

        Device.sendConfigToDevice(config);
    }

    /**
     *
     * @param json
     */
    function handleGetDeviceConfigBtCommand(json) {
        Device.getConfig(null, true)
    }

    /**
     *
     * @param json
     */
    function handleGetDeviceNetworks(json) {
        Device.requestDeviceNetworks();
    }

    /**
     *
     * @param json
     */
    function handlePostDeleteSavedConnection(json) {

        var msg = {
            NetworkUUID: json.NetworkUUID || ""
        };

        Device.sendMessage(Device.MQTT_DEVICE_TOPCIS.REQUEST_DELETE_WIFI_NETWORK, msg)
    }

    /**
     *
     * @param json
     */
    function handlePostAddConnection(json) {

        var msg = {
            WifiSSID: json.WifiSSID || "",
            WifiPassword: json.WifiPassword || ""
        };

        Device.sendMessage(Device.MQTT_DEVICE_TOPCIS.REQUEST_ADD_WIFI_NETWORK, msg);
    }

    /**
     * Error handler to call if error occurred during writing data to BT socket.
     * @param error
     */
    function bluetoothWriteErrorHandler(error) {
        logger.error("Bluetooth write error occurred.", error);

        if (++self.writeErrors > self.maxWriteError)
        {
            self.clientConnected = false;
            logger.debug("Max bluetooth write errors limit reached. Restarting Bluetooth...");

            if (server) { server.close() }

            const event = {
                timestamp: moment().unix(),
                eventType: BT_PRESENCE_EVENT_TYPE.CLIENT_DISCONNECTED
            };

            Device.sendBtEventToDevice(event);
            self.BluetoothStart();
        }
    }

    /**
     * Check if can write to data to Bluetooth
     */
    function canWriteToBluetooth() {
        return server.isOpen() && self.clientConnected;
    }
    //endregion

    //region EventHandlers
    /**
     * BT Server 'Failure' event handler
     * @param error
     */
    function onBtServerFailureHandler(error) {
        self.clientConnected = false;
        logger.error("BT failure event: ", error)
    }

    /**
     * BT Server 'Close' event handler
     */
    function onBtServerClosedHandler() {
        logger.info("Bluetooth 'closed' event.");
        self.clientConnected = false;

        const event = {
            timestamp: moment().unix(),
            eventType: BT_PRESENCE_EVENT_TYPE.CLIENT_DISCONNECTED
        };

        Device.sendBtEventToDevice(event);
    }

    function onReadConfigReplyHandler(event){
        logger.info("onReadConfigReplyHandler().", { Event: event });

        if (canWriteToBluetooth()) {
            bluetoothWriteMessage(event.Message)
                .catch(bluetoothWriteErrorHandler)
        }
    }

    function onReadNetworksReplyHandler(event) {
        logger.info("onReadNetworksReplyHandler().", { Event: event });

        /*
        Event object :
        {
            MqttRequestId : "92a91e0f-e823-4573-8aea-499e591ce94f"
            Message: {
                AvailableNetworks : "String",
                Connections : "String"
            }
        }
        */

        function parseAvailableNetworks(networksStr) {

            var networks_str_list = networksStr.trim().split("\n").filter(Boolean);
            networks_str_list.shift(); // remove line with titles

            function parseLine(line) {
                var lineNoSpaces = line.trim();
                // SSID     MODE   CHAN  RATE   SIGNAL  BARS  SECURITY
                // (9) ["*", "EmgWorkHaifa10", "Infra", "2", "54 Mbit/s", "51", "__", "WPA2"]
                // ["BezeqFree", "Infra", "2", "54 Mbit/s", "45", "__", "WPA2"]

                // var splited = lineNoSpaces.split(/\s+\W+/);

                var splited = lineNoSpaces.split(/\s+Infra/);

                var network = {
                    SSID: "",
                    IsActive: false
                };

                if (splited && splited.length > 1) {
                    if (splited[0].startsWith("*")) {
                        network.IsActive = true;
                        network.SSID = splited[0].split(/\s+/)[1];
                    } else {
                        network.IsActive = false;
                        network.SSID = splited[0];
                    }
                }
                return network
            }

            logger.debug("parseAvailableNetworks()", {networks_str_list: networks_str_list});
            return networks_str_list.map(parseLine)
        }

        function parseConnections(connectionsStr) {

            var uuidRegex           = /[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}/i;
            var connections_str_list = connectionsStr.trim().split("\n").filter(Boolean);
            connections_str_list.shift(); // remove line with titles

            function transform(line) {
                var connection = {
                    name: "",
                    uuid: "",
                    type: "",
                    device: ""
                };

                // find the network uuid
                var uuid = line.match(uuidRegex);

                // name_type_device must be array of 2 elements
                // first element contains networks name
                // seconds element contains type and device
                var name_type_device_array = line.trim().split(uuidRegex);

                if (uuid && uuid.length > 0 && name_type_device_array.length > 1)
                {
                    connection.uuid = uuid[0] || "";
                    connection.name = name_type_device_array[0].trim() || "";

                    var type_device_array = name_type_device_array[1].trim().split(/\s+/, 2);

                    logger.debug("parseConnections() - transform()", {
                        name_type_device_array: name_type_device_array,
                        type_device_array: type_device_array
                    });

                    connection.type = type_device_array[0].trim() || "";
                    connection.device = type_device_array[1].trim() || "";
                } else {
                    logger.debug("parseConnections() - transform(). Failed to parse line.", { Line: line })
                }

                return connection;
            }

            logger.debug("parseConnections()", {connections_str_list: connections_str_list});
            return connections_str_list
                .map(transform)
                .filter(function (conn) {
                    // "type":"802-11-wireless"
                    return conn.type.indexOf("wireless") !== -1
                })
        }

        try {
            var reply = {
                AvailableWifiNetworks: parseAvailableNetworks(event.Message.AvailableNetworks) || [],
                SavedWifiConnections: parseConnections(event.Message.Connections) || []
            };

            logger.debug("onReadNetworksReplyHandler", {Reply: reply});

            if (canWriteToBluetooth()) {
                bluetoothWriteMessage(reply)
                    .catch(bluetoothWriteErrorHandler)
            }
        }
        catch (err) {
            logger.error("onReadNetworksReplyHandler(). Error occurred.", err)
        }
    }

    /**
     * BT Server 'data' event handler
     * @param buffer
     */
    function onBluetoothDataHandler(buffer) {

        function extractBTCommand (json) {
            return json.commandId
        }

        try {
            var json = JSON.parse(buffer.toString());
            logger.info('onBluetoothDataHandler() ', { BufferAsJson: json });

            var btCommand = extractBTCommand(json);

            switch (btCommand) {
                case BT_COMMAND.GET_DEVICE_CONFIG:
                    handleGetDeviceConfigBtCommand(json);
                    break;
                case BT_COMMAND.POST_DEVICE_CONFIG:
                    handlePostDeviceConfigBTCommand(json);
                    break;
                case BT_COMMAND.POST_DELETE_SAVED_CONNECTION:
                    handlePostDeleteSavedConnection(json);
                    break;
                case BT_COMMAND.POST_ADD_CONNECTION:
                    handlePostAddConnection(json);
                    break;
                case BT_COMMAND.GET_DEVICE_NETWORKS:
                    handleGetDeviceNetworks(json);
                    break;
                default:
                    logger.debug('onBluetoothDataHandler(). Unknown BT Command.', { Command: btCommand});
                    break;
            }
        }
        catch (err) {
            logger.error("onBluetoothDataHandler(). Failed to parse client buffer.", err)
        }
    }

    /**
     * Device 'Status' message received event handler
     * @param msg
     */
    function onStatusMsgHandler(msg) {
        var clone = JSON.parse(JSON.stringify(msg));
    }

    function onRegularDataMsgHandler(msg) {

        var clone = JSON.parse(JSON.stringify(msg));

        function errorHandler(error) {
            logger.error("onRegularDataMsgHandler(). Bluetooth write error occurred." , error);

            if (++self.writeErrors > self.maxWriteError)
            {
                self.clientConnected = false;

                logger.debug("onRegularDataMsgHandler(). Max write errors limit reached. Restarting BT.");

                if (server) {
                    server.close()
                }

                const event = {
                    timestamp: moment().unix(),
                    eventType: BT_PRESENCE_EVENT_TYPE.CLIENT_DISCONNECTED
                };

                Device.sendBtEventToDevice(event);

                self.BluetoothStart();
            }
        }

        if (server.isOpen() && self.clientConnected) {
            bluetoothWriteMessage(clone)
                .catch(errorHandler)
        }
    }

    function onRejectedCommandMsgHadnler(msg) {
        var clone = JSON.parse(JSON.stringify(msg));

        if (server.isOpen()) {
            bluetoothWriteMessage(clone)
                .then(function (bytesWritten) {})
                .catch(function (error) { logger.error("BT Write error: " , error)})
        }
    }

    function onAcceptedCommandMsgHadnler(msg) {
        var clone = JSON.parse(JSON.stringify(msg));

        if (server.isOpen()) {
            bluetoothWriteMessage(clone)
                .then(function (bytesWritten) {})
                .catch(function (error) { logger.error("BT Write error: " , error)})
        }
    }
    //endregion

    //region Public Methods
    this.BluetoothStart = function () {

        logger.info("BluetoothStart()");
        self.writeErrors = 0;

        function btConnectHandler(clientAddress) {
            logger.info("btConnectHandler()" , { Address: clientAddress });

            self.clientConnected = true;

            const event = {
                clientMacAddress: clientAddress,
                timestamp: moment().unix(),
                eventType: BT_PRESENCE_EVENT_TYPE.CLIENT_CONNECTED
            };

            Device.sendBtEventToDevice(event)
        }

        function btErrorHandler(error) {
            logger.error("btErrorHandler()", error);
        }

        server.listen(btConnectHandler, btErrorHandler);
    };

    this.BluetoothStop = function() {
        logger.info("BluetoothStop()");
        server.close();
    };

    //endregion
}

module.exports = new BluetoothAdapter({ services : require('./bluetoothConfig.json')});
