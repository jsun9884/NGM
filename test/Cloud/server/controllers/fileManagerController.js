'use strict';

const S3Connector = require('../modules/S3Connector');

exports.requestSignedURL = function(req, res, next){

    let opts = {
        operation   : req.body.operation,
        filename    : req.body.filename,
        repository  : req.body.repository,
        saveas      : req.body.saveas
    };

    function sendResponse(url) {
        res.json({url: url})
    }

    S3Connector
        .getSignedURL(opts)
        .then(sendResponse)
        .catch(next)
};