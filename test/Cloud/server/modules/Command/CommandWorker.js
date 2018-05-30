"use strict";

const NR                = require("node-resque");
const config            = require('config');
const Command           = require('../../modules/Command/Command');
const RedisConnector    = require('../RedisConnector');
const DynamoDBConnector = require('../DynamoDbConnector');
const Logger            = require('../../util/logger')(module);

const SIMPLE_PROCESSOR_INTERVAL   = 250;

const connectionDetails = {
    pkg:       'ioredis',
    host:      config.AwsElastiCache.endpoint,
    password:  null,
    port:      config.AwsElastiCache.port,
    database:  config.AwsElastiCache.queueDB,
    namespace: 'resque'
};

let jobs = {
    "monitor": {
        plugins: [],
        pluginOptions: {},
        perform: defaultProcessor
    }
};

let multiWorker = new NR.multiWorker({
    connection: connectionDetails,
    queues: ['CommandQueue'],
    minTaskProcessors:   1,
    maxTaskProcessors:   50,
    checkTimeout:        500,
    maxEventLoopDelay:   10,
    toDisconnectProcessors: true,
    timeout: 500 //pooling interval;
}, jobs);

//region Normal worker emitters
multiWorker.on('start', function(workerId){
    // console.log("worker["+workerId+"] started");
});

multiWorker.on('end', function(workerId){
    // console.log("worker["+workerId+"] ended");
});

multiWorker.on('cleaning_worker', function(workerId, worker, pid){
    console.log("cleaning old worker " + worker);
});

multiWorker.on('poll', function(workerId, queue){
    //console.log("worker["+workerId+"] polling " + queue);
});

multiWorker.on('job', function(workerId, queue, job){
    console.log("worker["+workerId+"] working job " + queue + " " + JSON.stringify(job));
});

multiWorker.on('reEnqueue', function(workerId, queue, job, plugin){
    console.log("worker["+workerId+"] reEnqueue job (" + plugin + ") " + queue + " " + JSON.stringify(job));
});

multiWorker.on('success', function(workerId, queue, job, result){
    console.log("worker["+workerId+"] job success " + queue + " " + JSON.stringify(job) + " >> " + result);
});

multiWorker.on('failure', function(workerId, queue, job, failure){
    console.log("worker["+workerId+"] job failure " + queue + " " + JSON.stringify(job) + " >> " + failure);
});

multiWorker.on('error', function(workerId, queue, job, error){
    console.log("worker["+workerId+"] error " + queue + " " + JSON.stringify(job) + " >> " + error);
});

multiWorker.on('pause', function(workerId){
    //console.log("worker["+workerId+"] paused");
});
//endregion

//region MultiWorker emitters
multiWorker.on('internalError', function(error){
    console.log(error);
});

multiWorker.on('multiWorkerAction', function(verb, delay){
    //console.log("*** checked for worker status: " + verb + " (event loop delay: " + delay + "ms)");
});
//endregion

multiWorker.start();

process.on('SIGINT', function () {
    multiWorker.stop(function () {
        console.log('*** ALL WORDKERS STOPPED ***');
        process.exit()
    })
});

function defaultProcessor(job, done) {
    Logger.debug('defaultProcessor(). Default Processor Started.', { Job: job});

    let command = new Command({
        deviceId        : job.deviceId,
        commandId       : job.commandId,
        commandCode     : job.commandCode,
        cacheDbClient   : RedisConnector
    });

    let interval = setInterval(_periodicHandler, SIMPLE_PROCESSOR_INTERVAL);

    _periodicHandler();

    function _errorHandler(error) {
        Logger.error(error);
        clearInterval(interval);
        command = null;
        done(error);
    }

    function _stopProcessingCheck(isRunning) {
        Logger.debug("_stopProcessingCheck: ", {
            CommandId: command.commandId,
            IsRunning: isRunning
        });

        if (!isRunning) {
            Logger.debug('_stopProcessingCheck: Finished Processing Command.', {
                CommandId: command.commandId,
                State: command.state
            });
            DynamoDBConnector.saveCommand(command); // TODO refactor
            clearInterval(interval);
            command = null;
            done(null, true);
        }
    }

    function _periodicHandler() {
        Logger.debug('_periodicHandler()');

        command
            .updateState()
            .then(command.isRunning.bind(command))
            .then(_stopProcessingCheck)
            .catch(_errorHandler)
    }
}