'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .directive('hrDownload', [
            '$q',
            '$interval',
            '$log',
            'CommandsFactory',
            'ApiFactory',
            'DlgFactory',
            'CMD_STATES',
            hrDownload
        ]);

    function hrDownload($q, $interval, $log, CommandsFactory, ApiFactory, DlgFactory, CMD_STATES){
        return {
            restrict: 'E',
            bindToController: {
                deviceId: '@',
                eventId: '@'
            },
            controllerAs: 'ctrl',
            controller: function(){

                // region Init
                const ctrl = this;

                var periodicGetState;
                var waitingCount = 30;
                ctrl.isWaiting = false;
                ctrl.state = {
                    execCmdState : CMD_STATES.IDLE,
                    commandId : "",
                    metaData: {
                        error: '',
                        message: ''
                    }
                };

                // region Private methods
                function _stopPeriodicCall(fn) {
                    if (angular.isDefined(fn)) {
                        $interval.cancel(fn);
                    }
                    ctrl.isWaiting = false;
                }

                function _startPeriodicCall(fn, period) {
                    ctrl.isWaiting = true;
                    waitingCount = 30;
                    return angular.isDefined(fn) ? $interval(fn, period) : undefined
                }

                function _refreshState() {
                    if(!waitingCount--){
                        DlgFactory.showMessage('Error', 'Preparing HR data timeout');
                        ctrl.state.execCmdState = CMD_STATES.IDLE;
                        _stopPeriodicCall(periodicGetState);
                    }

                    if (ctrl.state.execCmdState !== CMD_STATES.POSTED
                        && ctrl.state.execCmdState !== CMD_STATES.PENDING
                    ) {
                        _stopPeriodicCall(periodicGetState);

                        if(ctrl.state.execCmdState == CMD_STATES.DONE){
                            _getDownloadUrl(ctrl.eventId, 'HR_DATA');
                        }

                        return;
                    }

                    CommandsFactory
                        .getCommandExecState(ctrl.deviceId, ctrl.state.commandId)
                        .then(function(response) {
                            ctrl.state.execCmdState = response ? response.runState : CMD_STATES.IDLE;
                            ctrl.state.metaData = response.runStateMetadata || {};
                        })
                        .catch(function(error){
                            $log.error("refreshState failed: ", error);
                        })
                }

                function _sendCommandPrepareHRFile() {
                    var commandToSend = {
                        commandCode : 330,      // prepare HR data
                        timestamp : moment().utc(),
                        params: {
                            filename: ctrl.eventId
                        }
                    };


                    return CommandsFactory
                        .sendCommandToDevice(ctrl.deviceId, commandToSend)
                        .then(function(response) {
                            $log.info("Command Response:", response);
                            ctrl.state.execCmdState = CMD_STATES.POSTED;
                            ctrl.state.commandId = response.commandId;
                        })
                        .then(function(){
                            periodicGetState = _startPeriodicCall(_refreshState, 1 * 1000)
                        })
                        .catch(function(error) {
                            $log.error('sendCommand failed:', error);
                            ctrl.state.execCmdState = CMD_STATES.IDLE;
                        })
                    ;

                }

                function _getDownloadUrl(filename, repository){
                    return ApiFactory
                        .requestDownloadLink({
                            'filename': filename + '.csv',
                            'repository': repository,
                            'saveas': ctrl.deviceId + '-' + moment.utc().format("HH:mm:ss-MM.DD.YYYY") + '.csv',
                            'operation': 'DOWNLOAD'
                        })
                        .then(function(result){
                            if(result.status == 404){
                                return _sendCommandPrepareHRFile();
                            }
                            return _openHRFile(result.data.url);
                        })
                        .catch(function(error){
                            console.error(error);
                        })
                    ;
                }

                function _openHRFile(url){
                    return $q(function(resolve, reject) {
                            var pom = document.createElement('a');
                            pom.setAttribute('href', url);
                            //pom.setAttribute('download', 'test.txt');

                            if (document.createEvent) {
                                var event = document.createEvent('MouseEvents');
                                event.initEvent('click', true, true);
                                pom.dispatchEvent(event);
                            }
                            else {
                                pom.click();
                            }
                            resolve();
                        })

                        .catch(function(){
                            ctrl.state.execCmdState = CMD_STATES.IDLE;
                            alert('Please allow popups for this website');
                        })
                    ;
                }
                // endregion

                // region UI events
                ctrl.getHRFile = function(){
                    _getDownloadUrl(ctrl.eventId, 'HR_DATA')
                        .then(function(url){
                        })
                        .catch(function(){
                        })
                    ;
                };
                // endregion
            },
            template:
                '<md-button class="md-raised md-accent" ng-class="{\'waiting\': ctrl.isWaiting}" ng-click="ctrl.getHRFile()" ng-disabled="ctrl.isWaiting">' +
                '    <span class="btn-title">Get HR Data</span>' +
                '    <span class="btn-spinner">' +
                '       <md-progress-circular md-mode="indeterminate" md-diameter="20" class="md-accent md-hue-1 btn-waiting-spinner"></md-progress-circular>' +
                '   </span>' +
                '</md-button>',
            link: function(scope, element, attrs) {
            }
        };
    }
})(angular);