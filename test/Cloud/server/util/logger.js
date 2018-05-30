"use strict";

const winston = require('winston');
const fs = require('fs');
const path = require('path');


winston.emitErrs = true;

function createLogger(module) {

    const mkdirSync = function (dirPath) {
        try {
            fs.mkdirSync(dirPath)
        } catch (err) {
            if (err.code !== 'EEXIST') throw err
        }
    };

    let LOG_FILE_PATH = null;

    if (process.env.NODE_ENV !== 'development') {
        mkdirSync("/home/ubuntu/site/ngm-dashboard.tk/log");
        LOG_FILE_PATH = '/home/ubuntu/site/ngm-dashboard.tk/log/ngm-server-app.log';
    } else {
        mkdirSync("/home/ubuntu/site/ngm-dashboard.com/log");
        LOG_FILE_PATH = '/home/ubuntu/site/ngm-dashboard.com/log/ngm-server-app.log';
    }

    // Return the last folder name in the path and the calling module's filename.
    let getLabel = function(module) {
        let parts = module.filename.split('/');
        return parts[parts.length - 2] + '/' + parts.pop();
    };

    // TODO Stream logs to AWS ES (For Kibana analysis)
    // TODO Stream logs to AWS ElasticCache (Redis)
    // TODO Stream logs to file with rotation
    // TODO Fix issue with maximum event listeners
    return new winston.Logger({
        transports: [

             new winston.transports.File({
             level: 'debug',
             filename: LOG_FILE_PATH,
             handleExceptions: true,
             json: true,
             maxsize: 5242880 * 2, // 10MB
             maxFiles: 5,
             colorize: false
             }),

            new winston.transports.Console({
                level: 'debug',
                handleExceptions: true,
                timestamp: true,
                json: false,
                prettyPrint: true,
                colorize: true,
                label : getLabel(module)
            })
        ],
        exitOnError: false
    });
}

module.exports = createLogger;