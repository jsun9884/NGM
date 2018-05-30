'use strict';

var moment  = require('moment');
var Logger  = require('../util/logger');

function Command(opts) {

    var self = this;

    var CMD_STATES      = {
        UNDEFINED   : "UNDEFINED",
        POSTED      : "POSTED",
        PENDING     : "PENDING",
        PROGRESS    : "PROGRESS",
        REJECTED    : "REJECTED",
        DONE        : "DONE",
        TIMEOUT     : "TIMEOUT"
    };

    var STATE_TIMEOUT_SEC = {
        POSTED      : 10,
        PENDING     : 60
    };

    this.deviceId       = opts.deviceId         || 0;
    this.commandId      = opts.commandId        || '';
    this.commandCode    = opts.commandCode      || 0;
    this.params         = opts.params           || {};
    this.state          = {
        commandCode         : opts.commandCode || 0,
        createdAt           : moment().unix(),
        stateChangeTime     : moment().unix(),
        runState            : opts.runState || CMD_STATES.UNDEFINED,
        runStateMetadata    : opts.runStateMetadata || {}
    };

    var stateHandlers = {

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

        timeout : function (stateMetadata) {
            self.state.runState = CMD_STATES.TIMEOUT;
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

    /* Private Methods */


    /* Public Methods */

    this.changeRunState = function (runState, stateMetadata) {

        Logger.debug("### Command: changeRunState. ###",
            {   runState: runState,
                stateMetadata: stateMetadata
            });

        switch (runState) {
            case CMD_STATES.POSTED:
                stateHandlers.posted(stateMetadata);
                break;
            case CMD_STATES.PENDING:
                stateHandlers.pending(stateMetadata);
                break;
            case CMD_STATES.REJECTED:
                stateHandlers.rejected(stateMetadata);
                break;
            case CMD_STATES.TIMEOUT:
                stateHandlers.timeout(stateMetadata);
                break;
            case CMD_STATES.DONE:
                stateHandlers.done(stateMetadata);
                break;
            default:
                throw new Error('State change failed. Invalid state.');
        }
    };

    this.isCurrentStateTimedOut = function () {
        switch (self.state.runState) {
            case CMD_STATES.POSTED:
                return (moment().unix() - self.state.stateChangeTime > STATE_TIMEOUT_SEC.POSTED);
            case CMD_STATES.PENDING:
                return (moment().unix() - self.state.stateChangeTime > STATE_TIMEOUT_SEC.PENDING);
            default:
                return false;
        }
    };

    this.isRunning = function(){
        Logger.debug("### Command: Is Running. ###", self.commandId);

        switch (self.state.runState) {
            case CMD_STATES.POSTED:
            case CMD_STATES.PENDING:
            case CMD_STATES.PROGRESS:
                return true;
            default:
                return false;
        }
    };
}

module.exports = Command;