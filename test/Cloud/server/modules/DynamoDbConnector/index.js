'use strict';

const AWS           = require('aws-sdk');
const Joi           = require('joi');
const dynogels      = require('dynogels');
const util          = require('util');
const moment        = require('moment');
const config        = require('config');
const Logger        = require('../../util/logger')(module);
const json2csv      = require('json2csv');
const fs            = require('fs');
const PromiseLoop   = require('../../util/promiseLoop');

function DynamoDbConnector(opts) {

    //region Init
    const AWS_DYNAMODB_REGION = opts.region || "us-west-2";

    const dynamodb = new AWS.DynamoDB({region: AWS_DYNAMODB_REGION});

    const TABLE_RANGE_KEYS_MAP = {
        DEVICE_EVENTS: "eventCreationTime",
        DEVICE_COMMANDS: "createdAt",
        DEVICE_REGULAR_DATA: "timestamp"
    };

    const TABLES = {
        DEVICE_EVENTS: opts.eventsTable,
        DEVICE_COMMANDS: opts.commandsTable,
        DEVICE_REGULAR_DATA: opts.regularDataTable,
        DEVICE_STATUS: opts.statusDataTable
    };

    const SCAN_DIRECTION = {
        FORWARD : 0,
        BACK : 1
    };

    dynogels.dynamoDriver(dynamodb);

    let EventModel = dynogels.define('Event', {
        hashKey : "deviceId",
        rangeKey : "eventCreationTime",
        timestamps: false,
        tableName: TABLES.DEVICE_EVENTS,
        schema : {
            deviceId                : Joi.string(),
            eventCreationTime       : Joi.string(),
            devicePublishTime       : Joi.string(),
            eventId                 : Joi.string(),
            eventSeverity           : Joi.number(),
            eventSource             : Joi.string(),
            eventType               : Joi.string(),
            eventData               : Joi.object()
        }
    });

    let CommandModel = dynogels.define('Command',{
        hashKey : "deviceId",
        rangeKey : "createdAt",
        timestamps: false,
        tableName: TABLES.DEVICE_COMMANDS,
        schema : {
            deviceId                : Joi.string(),
            createdAt               : Joi.string(),
            updatedAt               : Joi.string(),
            commandId               : Joi.string(),
            commandCode             : Joi.number(),
            params                  : Joi.object(),
            state                   : Joi.string(),
            stateMetadata           : Joi.object()
        }
    });

    let RegularDataModel = dynogels.define('RegularData',{
        hashKey : "deviceId",
        rangeKey : TABLE_RANGE_KEYS_MAP.DEVICE_REGULAR_DATA,
        timestamps: false,
        tableName: TABLES.DEVICE_REGULAR_DATA,
        schema : {
            deviceId                : Joi.string(),
            timestamp               : Joi.string(),
            AccCore                 : Joi.object(),
            AccPCB                  : Joi.object(),
            GPS                     : Joi.object(),
            PowerMeter              : Joi.object(),
            TamperSwitch            : Joi.object(),
            TempCPU                 : Joi.object(),
            TempPCB                 : Joi.object(),
            DataSwitch              : Joi.object()
        }
    });

    //endregion

    //region Private methods
    function dateStringToUnixTimestamp(dateStr) { return moment(dateStr).unix() }

    function unixTimestampToISOString(timestamp) { return moment.unix(timestamp).toISOString() }

    function paginate(result, queryParams, direction, tableRangeKey) {

        function getMetadata(data, lastEvalKey, rangeKey) {
            let max_id = null;
            let since_id = null;

            if (data.length > 0) {
                if (lastEvalKey !== null) {
                    if (direction === SCAN_DIRECTION.FORWARD) {
                        since_id = dateStringToUnixTimestamp(lastEvalKey[rangeKey]);
                        max_id = dateStringToUnixTimestamp(data[0][rangeKey]);
                    } else {
                        max_id = dateStringToUnixTimestamp(lastEvalKey[rangeKey]);
                        since_id = dateStringToUnixTimestamp(data[data.length - 1][rangeKey]);
                    }
                }
                else {
                    if (direction === SCAN_DIRECTION.FORWARD) {
                        since_id = null;
                        max_id = dateStringToUnixTimestamp(data[0][rangeKey]);
                    } else {
                        max_id = null;
                        since_id = dateStringToUnixTimestamp(data[0][rangeKey]);
                    }
                }
            }

            return {
                max_id: max_id,
                since_id: since_id
            }
        }

        function getData() {
            if (direction === SCAN_DIRECTION.FORWARD) {
                return result.Items.map((item) => item.attrs);
            } else {
                return result.Items.map((item) => item.attrs).reverse()
            }
        }

        Logger.debug("paginate()", {
            QueryParams: queryParams,
            QueryResult: result,
            LastEvaluatedKey: result.LastEvaluatedKey,
            ScanDirection: direction,
            RangeKey: tableRangeKey
        });

        const lastEvalKey = result.LastEvaluatedKey || null;
        const data = getData();

        return {
            search_metadata: getMetadata(data, lastEvalKey, tableRangeKey),
            data: data
        };
    }

    /**
     * Executes query to DynamoDB and loads data, the result set is limited to specific number of records.
     * @param params
     */
    function loadRegularData(params) {

        let itemsReadCount = 0;
        let maxItemsToRead = 3600; // FIXME Make parameter
        let resultCollection = [];

        const startingPromise = function() {
            return new Promise(function (resolve, reject) {
                RegularDataModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_REGULAR_DATA).between(params.dateFrom, params.dateTo)
                    .limit(50)
                    .exec((err, res) => { err ? reject(err) : resolve(res) });
            })
        };

        const loopingPromise = PromiseLoop(function (result) {
            return new Promise(function (resolve, reject) {

                const lastEvaluatedKey = result.LastEvaluatedKey || null;
                resultCollection = resultCollection.concat(result.Items.map((item) => item.attrs));
                itemsReadCount += result.Count;

                // loop until LastEvaluatedKey is null or maximum items have been read.
                if (itemsReadCount < maxItemsToRead && lastEvaluatedKey !== null) {
                    RegularDataModel
                        .query(params.deviceId)
                        .where(TABLE_RANGE_KEYS_MAP.DEVICE_REGULAR_DATA).between(params.dateFrom, params.dateTo)
                        .startKey(params.deviceId, lastEvaluatedKey.timestamp)
                        .limit(50)
                        .exec((err, res) => { err ? reject(err) : resolve(res)});
                } else {
                    reject(resultCollection)
                }
            })
        }, PromiseLoop.catchRejectPromise);

        return startingPromise()
            .then(loopingPromise)
    }

    //endregion

    //region Public methods
    this.saveCommand = function (command) {
        return new Promise(function (resolve, reject) {
            let params = {
                deviceId                : command.deviceId,
                commandId               : command.commandId,
                commandCode             : command.commandCode,
                params                  : command.params,
                state                   : command.state.runState,
                stateMetadata           : command.state.runStateMetadata,
                createdAt               : moment.unix(command.state.createdAt).toISOString(),
                updatedAt               : moment.unix(command.state.stateChangeTime).toISOString()
            };
            CommandModel.update(params, (err, model) => {
                err ? reject(err) : resolve(model)
            })
        })
    };

    this.getDeviceCommands = function (params) {
        Logger.debug("getDeviceCommands()", {QueryParams: params});

        return new Promise(function (resolve, reject) {
            if (params.max_id) {
                // Scan Backwards
                CommandModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .startKey(params.deviceId, params.max_id)
                    .descending()
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                            paginate(res, params, SCAN_DIRECTION.BACK, TABLE_RANGE_KEYS_MAP.DEVICE_COMMANDS))
                    })
            }
            else if (params.since_id) {
                // Scan forward
                CommandModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .startKey(params.deviceId, params.since_id)
                    .ascending()
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                            paginate(res, params, SCAN_DIRECTION.FORWARD, TABLE_RANGE_KEYS_MAP.DEVICE_COMMANDS))
                    })
            }
            else {
                // Scan forward
                CommandModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                            paginate(res,params, SCAN_DIRECTION.FORWARD, TABLE_RANGE_KEYS_MAP.DEVICE_COMMANDS))
                    })
            }
        })
    };

    this.getDeviceEvents = function (params) {
        Logger.debug("getDeviceEvents()", {QueryParams: params});

        return new Promise(function (resolve, reject) {
            if (params.max_id) {
                // Scan Backwards
                EventModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .startKey(params.deviceId, params.max_id)
                    .descending()
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                        paginate(res, params, SCAN_DIRECTION.BACK, TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS))
                    })
            }
            else if (params.since_id) {
                // Scan forward
                EventModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .startKey(params.deviceId, params.since_id)
                    .ascending()
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                        paginate(res, params, SCAN_DIRECTION.FORWARD, TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS))
                    })
            }
            else {
                // Scan forward
                EventModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .limit(params.limit)
                    .exec((err, res) => { err ? reject(err) : resolve(
                        paginate(res,params, SCAN_DIRECTION.FORWARD, TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS))
                    })
            }
        })
    };

    /**
     * Queries DynamoDB Events Table using Hash and Range index, loads all data then writes data
     * to csv file and stores locally
     * @param params
     */
    this.exportEventsToCSVFile = function (params) {
        Logger.debug("exportEventsToCSVFile()", {QueryParams: params});

        function loadEventsFromDB(params) {
            return new Promise(function (resolve, reject) {
                EventModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_EVENTS).between(params.dateFrom, params.dateTo)
                    .limit(params.limit || 50)
                    .loadAll()
                    .exec((err, res) => { err ? reject(err) : resolve(res.Items.map((item) => item.attrs))});
            })
        }

        function parseEventsToCsvString(events) {

            const json2csvOpts = {
                data: events,
                flatten: true,
                fields: (events && events.length) ? undefined : ['no-data']
            };

            return new Promise((resolve, reject) => {
                // Errors are thrown for bad options, or if the data is empty and no fields are provided.
                // Be sure to provide fields if it is possible that your data array will be empty.
                json2csv(json2csvOpts, (err, csv) => { err ? reject(err) : resolve(csv) });
            })
        }

        function writeCsvToFile(csv) {
            return new Promise((resolve, reject) => {
                const path = "/tmp/events-{ts}.csv".replace("{ts}", moment().unix());
                fs.writeFile(path, csv, (err) => { err ? reject(err) : resolve(path) });
            })
        }


        return loadEventsFromDB(params)
            .then(parseEventsToCsvString)
            .then(writeCsvToFile)
    };

    /**
     * Queries DynamoDB device-data table, loads data and writes to csv file
     * @param params
     */
    this.exportRegularDataToCSVFile = function (params) {
        Logger.debug("exportRegularDataToCSVFile()", {QueryParams: params});

        /*function loadDataFromDB(params) {
            return new Promise(function (resolve, reject) {
                RegularDataModel
                    .query(params.deviceId)
                    .where(TABLE_RANGE_KEYS_MAP.DEVICE_REGULAR_DATA).between(params.dateFrom, params.dateTo)
                    .limit(params.limit || 50)
                    .loadAll()
                    .exec((err, res) => { err ? reject(err) : resolve(res.Items.map((item) => item.attrs))});
            })
        }*/

        function parseRegularDataToCsvString(data) {

            const json2csvOpts = {
                data: data,
                flatten: true,
                fields: (data && data.length) ? undefined : ['no-data']
            };

            return new Promise((resolve, reject) => {
                // Errors are thrown for bad options, or if the data is empty and no fields are provided.
                // Be sure to provide fields if it is possible that your data array will be empty.
                json2csv(json2csvOpts, (err, csv) => { err ? reject(err) : resolve(csv) });
            })
        }

        function writeCsvToFile(csv) {
            return new Promise((resolve, reject) => {
                const path = "/tmp/regular-data-{ts}.csv".replace("{ts}", moment().unix());
                fs.writeFile(path, csv, (err) => { err ? reject(err) : resolve(path) });
            })
        }

        return loadRegularData(params)
            .then(parseRegularDataToCsvString)
            .then(writeCsvToFile)
    }
    //endregion
}

module.exports = new DynamoDbConnector({
    eventsTable: config.DynamoDB.DEVICE_EVENTS_TABLE,
    commandsTable: config.DynamoDB.DEVICE_COMMANDS_TABLE,
    regularDataTable: config.DynamoDB.DEVICE_REGULAR_DATA_TABLE,
    statusDataTable: config.DynamoDB.DEVICE_STATUS_TABLE
});