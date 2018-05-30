"use strict";

const errorFactory = require('error-factory');

function _requestSignedURLValidator(req, res, next) {
    if (req.user.role === 'TECH') {
        let InsufficientRightsException = errorFactory('InsufficientRightsException');
        next(InsufficientRightsException('InsufficientRightsException.'))
    } else {
        next()
    }
}

function _updateDeviceShadowValidator(req, res, next) {

    if (req.user.role === 'TECH') {
        let InsufficientRightsException = errorFactory('InsufficientRightsException');
        next(InsufficientRightsException('InsufficientRightsException.'))
    } else {
        if (req.params.deviceId && req.body.payload) {
            next()
        } else {
            let ArgumentException = errorFactory('ArgumentException');
            next(ArgumentException('Invalid Arguments.'))
        }
    }
}

function _sendCommandValidator(req, res, next) {
    if (req.user.role === 'TECH') {
        let InsufficientRightsException = errorFactory('InsufficientRightsException');
        next(InsufficientRightsException('InsufficientRightsException.'))
    } else {
        next()
    }
}

module.exports = {
    sendCommandValidator: _sendCommandValidator,
    requestSignedURLValidator: _requestSignedURLValidator,
    updateDeviceShadowValidator: _updateDeviceShadowValidator
};