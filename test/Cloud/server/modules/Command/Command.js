'use strict';

const moment  = require('moment');
const Logger  = require('../../util/logger')(module);

function Command(opts) {

    //region Init
    let self = this;

    let CMD_STATES      = {
        UNDEFINED   : "UNDEFINED",
        POSTED      : "POSTED",
        PENDING     : "PENDING",
        PROGRESS    : "PROGRESS",
        ACCEPTED    : "ACCEPTED",
        REJECTED    : "REJECTED",
        DONE        : "DONE",
        TIMEOUT     : "TIMEOUT"
    };

    let STATE_TIMEOUT_SEC = {
        POSTED      : 10,
        ACCEPTED    : 300,
        PENDING     : 300,
        PROGRESS    : 300
    };

    let STATE_CACHE_KEY_EXPIRE_TIME_SEC = {
        UNDEFINED   : 30,
        POSTED      : 5 * 60,
        PENDING     : 5 * 60,
        REJECTED    : 5 * 60,
        DONE        : 5 * 60,
        TIMEOUT     : 30
    };

    self.deviceId       = opts.deviceId         || 0;
    self.commandId      = opts.commandId        || '';
    self.commandCode    = opts.commandCode      || 0;
    self.cacheDbClient  = opts.cacheDbClient    || null;
    self.params         = opts.params           || {};

    self.state          = {
        commandCode         : opts.commandCode || 0,
        createdAt           : moment().unix(),
        stateChangeTime     : moment().unix(),
        runState            : opts.runState || CMD_STATES.UNDEFINED,
        runStateMetadata    : opts.runStateMetadata || {}
    };

    let key_pattern = "devices:{deviceId}:commands:{commandId}"
        .replace("{deviceId}",self.deviceId)
        .replace("{commandId}",self.commandId);
    
    let stateHandlers = {

        posted : function(stateMetadata){
            self.state.runState = CMD_STATES.POSTED;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        pending : function (stateMetadata) {
            self.state.runState = CMD_STATES.PENDING;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        progress : function (stateMetadata) {
            self.state.runState = CMD_STATES.PROGRESS;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        timeout : function (stateMetadata) {
            self.state.runState = CMD_STATES.TIMEOUT;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        accepted : function (stateMetadata) {
            self.state.runState = CMD_STATES.ACCEPTED;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        rejected : function (stateMetadata) {
            self.state.runState = CMD_STATES.REJECTED;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        },

        done    : function (stateMetadata) {
            self.state.runState = CMD_STATES.DONE;
            self.state.runStateMetadata = stateMetadata || {};
            self.state.stateChangeTime = moment().unix();
        }
    };
    //endregion Init

    //region Private Methods
    function getStateFromCache() {
        Logger.debug("getStateFromCache()");
        return self.cacheDbClient.get(key_pattern);
    }

    function writeStateToCache(expire) {
        Logger.debug("writeStateToCache()");
        if (expire) {
            return self.cacheDbClient.setex(key_pattern, expire, self.state);
        }
        else return self.cacheDbClient.set(key_pattern, self.state)
    }

    function isCurrentStateTimedOut() {
        Logger.debug("isCurrentStateTimedOut()", { State: self.state });
        switch (self.state.runState) {
            case CMD_STATES.POSTED:
                return (moment().unix() - self.state.stateChangeTime) > STATE_TIMEOUT_SEC.POSTED;
            case CMD_STATES.PENDING:
                return (moment().unix() - self.state.stateChangeTime) > STATE_TIMEOUT_SEC.PENDING;
            case CMD_STATES.PROGRESS:
                return (moment().unix() - self.state.stateChangeTime) > STATE_TIMEOUT_SEC.PROGRESS;
            case CMD_STATES.ACCEPTED:
                return (moment().unix() - self.state.stateChangeTime) > STATE_TIMEOUT_SEC.ACCEPTED;
            case CMD_STATES.TIMEOUT:
                return true;
            default:
                return false;
        }
    }
    //endregion

    //region Public Methods
    this.saveToCache = function () {
        Logger.debug("saveToCache()", {
            CommandId: self.commandId,
            State:  self.state,
            Params: self.params
        });
        return writeStateToCache();

        /*
        Logger.debug("saveToCache", { runState: self.state.runState});
        switch (self.state.runState) {
            case CMD_STATES.POSTED:
                return writeStateToCache(STATE_CACHE_KEY_EXPIRE_TIME_SEC.POSTED);
            case CMD_STATES.PENDING:
                return writeStateToCache(STATE_CACHE_KEY_EXPIRE_TIME_SEC.PENDING);
            case CMD_STATES.REJECTED:
                return writeStateToCache(STATE_CACHE_KEY_EXPIRE_TIME_SEC.REJECTED);
            case CMD_STATES.TIMEOUT:
                return writeStateToCache(STATE_CACHE_KEY_EXPIRE_TIME_SEC.TIMEOUT);
            default:
                return writeStateToCache();
        }
         */
    };

    this.restoreFromCache = function() {
        Logger.debug("restoreFromCache()" , self.commandId);

        return getStateFromCache()
            .then(function(state){
                if (state) {
                    self.state = state;
                } else {
                    self.state.runState = CMD_STATES.UNDEFINED;
                    self.state.stateChangeTime = moment().unix()
                }
            })
    };

    this.changeRunState = function (runState, runStateMetadata) {

        Logger.debug("changeRunState()", {
            runState: runState,
            runStateMetadata: runStateMetadata
        });

        switch (runState) {
            case CMD_STATES.POSTED:
                stateHandlers.posted(runStateMetadata);
                break;
            case CMD_STATES.PENDING:
                stateHandlers.pending(runStateMetadata);
                break;
            case CMD_STATES.ACCEPTED:
                stateHandlers.accepted(runStateMetadata);
                break;
            case CMD_STATES.PROGRESS:
                stateHandlers.progress(runStateMetadata);
                break;
            case CMD_STATES.REJECTED:
                stateHandlers.rejected(runStateMetadata);
                break;
            case CMD_STATES.TIMEOUT:
                stateHandlers.timeout(runStateMetadata);
                break;
            case CMD_STATES.DONE:
                stateHandlers.done(runStateMetadata);
                break;
            default:
                throw new Error('State change failed. Invalid state.');
        }
    };

    this.updateState = function() {
        Logger.debug("updateState()", self.commandId);

        return self.restoreFromCache()
            .then(isCurrentStateTimedOut)
            .then(function(isTimeout){
                if (isTimeout) {
                    self.changeRunState(CMD_STATES.TIMEOUT, {});
                    return self.saveToCache()
                }
            })
    };

    this.isRunning = function() {
        Logger.debug("isRunning()", {commandId: self.commandId});

        switch (self.state.runState) {
            case CMD_STATES.POSTED:
            case CMD_STATES.PENDING:
            case CMD_STATES.PROGRESS:
            case CMD_STATES.ACCEPTED:
                return true;
            default:
                return false;
        }
    };
    //endregion
}

module.exports = Command;