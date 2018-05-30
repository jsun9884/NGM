"use strict";

const AWS = require("aws-sdk");
const Config = require("config");
const Q = require("q");
const Logger = require('../../util/logger')(module);


function S3Connector() {

    //region Init
    const S3 = new AWS.S3({
        signatureVersion: 'v4',
        region: 'us-west-2'
    });
    const DEFAULT_EXPIRE_TIME_SEC = 60 * 15;
    const CONTENT_TYPE_BINARY = "binary/octet-stream";
    const S3_OPERATIONS_MAP = {
        UPLOAD: "putObject",
        DOWNLOAD: "getObject"
    };
    const S3_BUCKETS_MAP = {
        FIRMWARE: Config.S3.firmwareBucket,
        CONFIG_FILES: Config.S3.cfgFilesBucket,
        HR_DATA: Config.S3.hrDataBucket
    };
    //endregion Init

    //region Private Methods
    function getS3Operation(operation) {
        return S3_OPERATIONS_MAP[operation] || false;
    }

    function getS3RequestParams(opts) {
        Logger.debug("getS3RequestParams", {Opts: opts});
        let params = {
            Bucket: S3_BUCKETS_MAP[opts.repository] || "",
            Key: opts.filename || "",
            Expires: DEFAULT_EXPIRE_TIME_SEC,

        };

        //TODO Refactor this...
        if (opts.operation === 'UPLOAD') {
            params['ContentType'] = CONTENT_TYPE_BINARY;
        }

        if (opts.operation === 'DOWNLOAD') {
            if (opts.saveas) {
                params['ResponseContentDisposition'] = "attachment; filename=\"{saveas}\"".replace("{saveas}", opts.saveas)
            }
        }

        return params;
    }

    function s3GetSignURL(s3Operation, s3ReqParams) {
        Logger.debug("s3GetSignURL", {
            s3Operation: s3Operation,
            s3ReqParams: s3ReqParams
        });
        return Q.Promise(function (resolve, reject) {
            if (s3Operation === S3_OPERATIONS_MAP.DOWNLOAD) {
                let params = {
                    Bucket: s3ReqParams.Bucket,
                    Key: s3ReqParams.Key
                };

                S3.getObject(params, function (err, data) {
                    if (err) {
                        reject(err)
                    } else {


                        S3.getSignedUrl(s3Operation, s3ReqParams, function (err, url) {
                            err ? reject(err) : resolve(url)
                        });
                    }
                });
            } else {
                S3.getSignedUrl(s3Operation, s3ReqParams, function (err, url) {
                    err ? reject(err) : resolve(url)
                });
            }
        });
    }

    //endregion Private Methods

    //region Public
    this.getSignedURL = function (opts) {
        return s3GetSignURL(getS3Operation(opts.operation), getS3RequestParams(opts))
    };
    //endregion Public
}

module.exports = new S3Connector();