'use strict';

const moment            = require('moment');
const RedisConnector    = require('../modules/RedisConnector');
const DynamoDBConnector = require('../modules/DynamoDbConnector');

exports.getEvents = function (req, res, next) {

    function unixTimestampToISOString(unixtime) {
        return moment.unix(unixtime).toISOString()
    }

    let params = {
        deviceId: req.params.deviceId,
        dateFrom: unixTimestampToISOString(req.query.dateFrom),
        dateTo: unixTimestampToISOString(req.query.dateTo),
        limit: req.query.limit || 25,
        max_id: req.query.max_id ? unixTimestampToISOString(req.query.max_id) : null,
        since_id: req.query.since_id ? unixTimestampToISOString(req.query.since_id) : null
    };

    return DynamoDBConnector
        .getDeviceEvents(params)
        .then(res.json.bind(res))
        .catch(next)
};

exports.getCommands = function (req, res, next) {

    function unixTimestampToISOString(unixtime) {
        return moment(unixtime).toISOString()
    }

    let params = {
        deviceId: req.params.deviceId,
        dateFrom: unixTimestampToISOString(req.query.dateFrom),
        dateTo: unixTimestampToISOString(req.query.dateTo),
        limit: req.query.limit || 25,
        max_id: req.query.max_id ? unixTimestampToISOString(req.query.max_id) : null,
        since_id: req.query.since_id ? unixTimestampToISOString(req.query.since_id) : null
    };

    return DynamoDBConnector
        .getDeviceCommands(params)
        .then(res.json.bind(res))
        .catch(next)
};

exports.getRecentEvents = function (req, res, next) {

    function fetchRecentEvents(req) {

        let key = "devices:events";
        let start = 0;
        let stop = req.query.count || 50;

        return RedisConnector
            .zrevrange(key, start, stop)
            .then(function (eventsList) {
                return eventsList.map(function (event) {
                    return JSON.parse(event)
                })
            })
    }

    fetchRecentEvents(req)
        .then(res.json.bind(res))
        .catch(next)
};

exports.fileAccepted = function (req, res, next) {
    res.send('ok');
};

exports.exportEvents = function (req, res, next) {

    function unixTimestampToISOString(unixtime) {
        return moment.unix(unixtime).toISOString()
    }

    function sendResponse(filePath) {

        const name = "{deviceId}-{ts}-events.csv"
            .replace("{deviceId}",req.params.deviceId)
            .replace("{ts}", moment().format("YYYY-MM-DD_HH-mm-ss"));

        res.download(filePath, name, function(err){
            if (err) {
                throw err;
                // Handle error, but keep in mind the response may be partially-sent so check res.headersSent
            } else {
                // decrement a download credit, etc.
            }
        });
    }

    let params = {
        deviceId: req.params.deviceId,
        dateFrom: unixTimestampToISOString(req.query.dateFrom),
        dateTo: unixTimestampToISOString(req.query.dateTo),
    };

    DynamoDBConnector
        .exportEventsToCSVFile(params)
        .then(sendResponse)
        .catch(next)
};

exports.exportRegularData = function (req, res, next) {

    function unixTimestampToISOString(unixtime) {
        return moment.unix(unixtime).toISOString()
    }

    function sendResponse(filePath) {

        const name = "{deviceId}-{ts}-data.csv"
            .replace("{deviceId}",req.params.deviceId)
            .replace("{ts}", moment().format("YYYY-MM-DD_HH-mm-ss"));

        res.download(filePath, name, function(err){
            if (err) {
                throw err;
                // Handle error, but keep in mind the response may be partially-sent so check res.headersSent
            } else {
                // decrement a download credit, etc.
            }
        });
    }

    let params = {
        deviceId: req.params.deviceId,
        dateFrom: unixTimestampToISOString(req.query.dateFrom),
        dateTo: unixTimestampToISOString(req.query.dateTo),
    };

    DynamoDBConnector
        .exportRegularDataToCSVFile(params)
        .then(sendResponse)
        .catch(next)
};