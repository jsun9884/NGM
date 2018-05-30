'use strict';

const promiseLoop = function(resolvePromise, rejectPromise) {
    rejectPromise = rejectPromise || promiseLoop.defaultRejectPromise;

    return function internalPromiseLoop(value) {
        return resolvePromise(value)
            .then(function(nextValue) {
                return internalPromiseLoop(nextValue);
            }, function(endValue) {
                return rejectPromise(endValue);
            });
    }
};

promiseLoop.defaultRejectPromise = (value) => value;

promiseLoop.catchRejectPromise = function(maybeError) {
    return new Promise(function(resolve, reject) {
        if (maybeError instanceof Error) {
            reject(maybeError);
        } else {
            resolve(maybeError);
        }
    });
};

module.exports = promiseLoop;