'use strict';

const Redis           = require('redis');
const Q               = require('q');
const config          = require('config');
const Logger          = require('../../util/logger')(module);

function RedisConnector(opts) {

    //region Init
    let client  = Redis.createClient(opts.port, opts.host);
    //endregion Init

    //region Event Handlers
    client.on('error', function (error) {
        Logger.error("RedisConnector 'error' occurred.", error);
        throw error;
    });

    client.on('ready', function () {
        Logger.info("RedisConnector 'ready'. Connection options: ", {Options: opts})
    });

    client.on('end', function () {
        // client will emit end when an established Redis server connection has closed.
        Logger.info("RedisConnector 'end' event.")
    });
    //endregion Event Handlers

    //region Public Methods
    this.set = function _set(key, value) {
        return Q.Promise(function (resolve, reject) {
            client.set(key, JSON.stringify(value), function (err, reply) {
                err ? reject(err) : resolve(reply)
            });
        });
    };

    this.get = function _get(key) {
         return Q.npost(client, "get", [key])
            .then(function(reply){
                return JSON.parse(reply);
            })
    };

    this.setex = function _setex(key, expire, value) {
        return Q.npost(client, "setex", [key, expire, JSON.stringify(value)])
    };

    this.zrevrange = function(key, start, stop){
        return Q.npost(client, "zrevrange", [key, start, stop])
    };

    this.mget = function _mget(keys) {
        return Q.npost(client, "mget", [keys])
    };

    this.mset = function _mset(keyValueArray) {
        // client.mset(["key1", "val1", "key2", "val2"], function (err, res) {});
        return Q.npost(client, "mset", keyValueArray)
    };

    this.quit =  function _quit() {
        return Q.nbind(client.quit, client)();
    };

    this.keys = function _keys(keyPattern) {
        return Q.npost(client, "keys", [keyPattern])
    };

    this.terminate = function _terminate() {
        return Q.nbind(client.end, client)();
    };
    //endregion Public Methods
}

//noinspection JSUnresolvedVariable
module.exports = new RedisConnector({
    port : config.AwsElastiCache.port,
    host : config.AwsElastiCache.endpoint,
    db   : config.AwsElastiCache.sensorDB
});