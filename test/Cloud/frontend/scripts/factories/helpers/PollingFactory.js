'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('PollingFactory',[
            '$q',
            '$rootScope',
            '$timeout',
            PollingFactory
        ]);

    function PollingFactory($q, $rootScope, $timeout){

        function _getPoller(){
            // region Init
            var pollingFuncResult = undefined;

            var defer = null;
            var timeout;
            var currentCycle = 0;
            var timerOn = false;
            // endregion

            // region Public functions
            function _go(fn, period, cyclesCount) {
                if(timerOn){
                    _stop(true);
                }
                defer = $q.defer();

                period = !!period ? period : 1000;
                cyclesCount = typeof cyclesCount !== 'undefined' ? cyclesCount : 0;

                if (angular.isDefined(fn)) {
                    currentCycle = cyclesCount;
                    timerOn = true;

                    var loop = function(){
                        timeout = $timeout(function(){
                            _tick(fn);

                            // if (infinite (!cyclesCount) or not all cycles passed) and timer is "on"
                            if((!cyclesCount || (--currentCycle)) && timerOn){
                                loop();
                            }
                            else{
                                if(!currentCycle && timerOn){
                                    defer.reject('timeout');
                                }
                            }
                        }, period);
                    };
                    loop();

                    return defer.promise;
                }
                else{
                    return $q.reject();
                }
            }

            function _stop(do_not_resolve){
                do_not_resolve = !!do_not_resolve;
                timerOn = false;
                $timeout.cancel(timeout);

                if(!do_not_resolve && !!defer){
                    defer.resolve('stopped');
                }
            }
            // endregion

            // region Private functions
            function _tick(fn){
                if (typeof fn === "function"){
                    pollingFuncResult = fn();//.apply(this);

                    if(angular.isDefined(pollingFuncResult)){
                        timerOn = false;

                        if(pollingFuncResult === true){
                            defer.resolve(pollingFuncResult);
                        }
                        if(pollingFuncResult === false){
                            defer.reject(pollingFuncResult);
                        }
                    }
                }
            }
            // endregion

            // region Events handlers
            $rootScope.$on('logout', function(){
                _stop();
            });
            // endregion

            return {
                go: _go,
                stop: _stop
            }
        }

        return {
            getPoller           :  _getPoller
        }
    }
})(angular);