'use strict';

const Logger = require('../util/logger')(module);
const spex = require('spex')(Promise);

exports.batchSendCommand = function (req, res, next) {
    res.json([{
        CommandId: "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx"
    }, {
        CommandId: "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx"
    }])
};