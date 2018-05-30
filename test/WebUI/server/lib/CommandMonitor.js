'use strict';

var config          = require('config');
var moment          = require('moment');
var util            = require('util');
var EventEmitter    = require('events').EventEmitter;

var device          = require('../lib/Device');
var logger          = require('../util/logger');
var Command         = require('../lib/Command');


function CommandMonitor() {

    //region Variable declaration and Initialization
    var self    = this,
        runningTasks   = [], // Running commands list
        finishedTasks = []; // finished processing commands list

    const DEFAULT_PROCESSOR_INTERVAL_MS = 1000;
    const CLEAR_FINISHED_TASKS_INTERVAL_MS = 1000 * 60 * 2;

    EventEmitter.call(this);

    //var commandProcessorInterval = setInterval(defaultProcessor, DEFAULT_PROCESSOR_INTERVAL_MS);
    //var clearFinsihedTasksInterval = setInterval(clearFinsihedTasks, CLEAR_FINISHED_TASKS_INTERVAL_MS);

    //endregion Variable declaration and Initialization

    //region Device event handlers

    device.on(device.COMMAND_EVENTS.MSG_CMD_ACCEPTED, function (message) {

        var command = runningTasks.find(function(task){
            return task.command.commandId == message.commandId
        });

        if (command) {
            command.changeRunState(message.state, message.stateMetadata)
        } else {
            logger.debug("CommandMonitor - COMMAND_EVENTS.MSG_CMD_ACCEPTED. Command not found.")
        }
    });

    device.on(device.COMMAND_EVENTS.MSG_CMD_REJECTED, function (message) {
        var command = runningTasks.find(function(task){
            return task.command.commandId == message.commandId
        });

        if (command) {
            command.changeRunState(message.state, message.stateMetadata)
        } else {
            logger.debug("CommandMonitor - COMMAND_EVENTS.MSG_CMD_REJECTED. Command not found.")
        }
    });
    //endregion Device event handlers

    //region Private Methods

    function clearFinsihedTasks() {
        // TODO Implement
        logger.debug("CommandMonitor - clearFinsihedTasks.")
    }

    function defaultProcessor() {

        function removeFromRunning(task){

            var taskIndex = runningTasks.findIndex(function(runningTask){
                return runningTask.command.commandId = task.command.commandId
            });

            if(taskIndex  != -1) {
                logger.debug("CommandMonitor - defaultProcessor - removeFromRunning. Finished task: ", {Task : task});
                runningTasks.splice(taskIndex, 1);
                finishedTasks.push(task);
            } else {
                logger.debug("CommandMonitor - defaultProcessor - removeFromRunning. Task not found.", {Task : task})
            }
        }

        function taskHandler(task){
            var isRunning = task.command.isRunning();
            var isTimeout = task.command.isCurrentStateTimedOut();

            if (isTimeout) {
                // TODO Change string to variable
                task.command.changeRunState("TIMEOUT");
                removeFromRunning(task);
            } else if (!isRunning){
                removeFromRunning(task);
            }
        }

        if (runningTasks.length) {
            logger.debug('defaultProcessor processing tasks...');
            runningTasks.map(taskHandler);
        } else {
            logger.debug("defaultProcessor. Tasks list Empty.");
        }
    }
    //endregion

    //region Public Methods
    this.createMonitorTask = function createTask(command) {
        logger.debug('CommandMonitor. createMonitorTask: ', {command : command});
        runningTasks.push({command : command})
    };

    this.getCommandStateById = function(commandId) {
        var task = runningTasks.concat(finishedTasks).find(function(task){
            return task.command.commandId = commandId
        });
        logger.debug('CommandMonitor. getCommandStateById: ', {Task : task});
        return task ? task.command : null
    };

    //endregion
}

util.inherits(CommandMonitor, EventEmitter);

module.exports = new CommandMonitor();