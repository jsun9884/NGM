'use strict';
/*global require, module */

const AWS       = require('aws-sdk');
const Q         = require('q');
const Logger    = require('../../util/logger')(module);
const config    = require('config');

function LambdaConnector(opts) {

    //region Init
    let self = this;

    const PREPARE_HR_DATA_LAMBDA_FN_NAME    = 'iotPrepareHRDataHandler';
    const API_VERSION                       = '2015-03-31';
    const REGION                            = 'us-west-2';
    const INVOCATION_TYPE                   = 'Event';
    const PREPARE_HR_DATA_LAMBDA_FN_QUALIFIER = opts.PREPARE_HR_DATA_FN_QUALIFIER;

    self.apiVersion = opts.apiVersion   || API_VERSION;
    self.region     = opts.region       || REGION;
    //endregion Init

    //region Private
    function invokeLambdaFunctions(params) {
        Logger.debug("invokeLambdaFunctions()", {Params: params});
        return Q.Promise(function(resolve, reject){
            let lambda = new AWS.Lambda({apiVersion: self.apiVersion});
            lambda.invoke(params, function (err, data) {
                err ? reject(err) : resolve(data)
            })
        })
    }
    //endregion

    //region Public
    this.invokePrepareHRData = function (command) {
        let params = {
            FunctionName: PREPARE_HR_DATA_LAMBDA_FN_NAME,
            InvocationType: INVOCATION_TYPE,
            Qualifier: PREPARE_HR_DATA_LAMBDA_FN_QUALIFIER,
            Payload : JSON.stringify({
                deviceId:       command.deviceId,
                commandId:      command.commandId,
                commandCode:    command.commandCode,
                fileName:       command.params.filename
            })
        };

        return invokeLambdaFunctions(params)
    };
    //endregion
}

module.exports = new LambdaConnector({
    PREPARE_HR_DATA_FN_QUALIFIER: config.Lambda.PREPARE_HR_DATA_FN_QUALIFIER
});