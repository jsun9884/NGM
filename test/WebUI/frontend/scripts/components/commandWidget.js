'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('commandWidget', {
            templateUrl: 'scripts/components/commandWidget-tmpl.html',
            controller: CmdWidgetController,
            bindings: {
                command: '<'
            }
        });

    function CmdWidgetController($log, $interval, $rootScope, DeviceFactory, DeviceCommandsFactory, $q, $mdDialog) {

        //region Init
        const ctrl = this;

        ctrl.CMD_STATES = {
            IDLE        : "IDLE",
            POSTED      : "POSTED",
            PENDING     : "PENDING",
            DONE        : "DONE",
            REJECTED    : "REJECTED",
            TIMEOUT     : "TIMEOUT",
            UPLOADING   : "UPLOADING"
        };

        var periodicGetState;

        var defaultState = {
            execCmdState : ctrl.CMD_STATES.IDLE,
            commandId : "",
            metaData: {
                error: '',
                message: ''
            }
        };

        ctrl.state = defaultState;

        ctrl.resultMode = false;
        ctrl.progressMode = false;

        ctrl.paramsExpanded = false;
        ctrl.paramsError = false;
        ctrl.paramsErrorMessage = '';
        ctrl.deviceIsOnline = DeviceFactory.isDeviceOnline();

        ctrl.cmdParams = angular.copy(ctrl.command.params);

        ctrl.fileControl = {};
        //endregion

        //region Private Methods
        function _checkRequiredParams(){
            return $q(function(resolve, reject){
                ctrl.cmdParams.reduce(function(result, param){
                    return result && (!param.required || angular.isDefined(param.value) || param.attributes.type == 'file');
                }, true) ? resolve() : reject('Required params are not defined')
            });
        }

        function _isRequiredFileDefined(fileParams){
            return (!fileParams.required || ctrl.fileControl.isDefined());
        }

        function _confirmation() {
            return $q(function (resolve, reject) {
                if (ctrl.command.confirmationText) {
                    var confirm = $mdDialog.confirm({skipHide: true})
                        .title('Are you sure?')
                        .textContent(ctrl.command.confirmationText)
                        .ok('Ok')
                        .cancel('Cancel');

                    return $mdDialog.show(confirm).then(function () {
                        resolve();
                    });
                }
                resolve();
            })
        }

        function _filesProcessing() {
            return $q(function (resolve, reject) {
                var filesParams = ctrl.command.params.filter(function(param){
                    return typeof param.attributes !== 'undefined' && param.attributes.type == 'file'
                });

                if (filesParams.length) {
                    if(_isRequiredFileDefined(filesParams[0])){
                        ctrl.progressMode = true;
                        _startWaitingState(ctrl.CMD_STATES.UPLOADING);

                        return ctrl.fileControl
                            .uploadFile()
                            .then(function(resonse){
                                _stopWaitingState();
                                resolve();
                            })
                            .catch(function(){
                                ctrl.progressMode = false;
                                reject();
                            })
                        ;
                    }
                    else{
                        //$log.error('You have to specify required parameters');
                        return $mdDialog.show(
                            $mdDialog.confirm({skipHide: true})
                                .title('Error')
                                .textContent('You have to specify required parameters')
                                .ok('Ok')
                            )
                            .then(function () {
                                reject();
                            })
                        ;
                    }
                }
                else{
                    resolve();
                }
            })
        }

        function _showError(message){
            ctrl.paramsError = false;
            ctrl.paramsErrorMessage = message;
            setTimeout(function(){
                ctrl.paramsError = true;
                setTimeout(function(){
                    ctrl.paramsError = false;
                }, 2000);
            }, 10);
        }

        function _stopPeriodicCall(fn) {
            if (angular.isDefined(fn)) {
                $interval.cancel(fn);
            }
        }

        function _startPeriodicCall(fn, period) {
            ctrl.paramsExpanded = false;
            ctrl.resultMode = true;
            return angular.isDefined(fn) ? $interval(fn, period) : undefined
        }

        function _resetState(){
            _stopWaitingState();
            ctrl.paramsExpanded = false;
            ctrl.resultMode = false;
            ctrl.progressMode = false;
        }

        function _startWaitingState(currentCmdState){
            ctrl.state.execCmdState = currentCmdState;
            periodicGetState = _startPeriodicCall(ctrl.refreshState, 1 * 1000);
        }

        function _stopWaitingState(currentCmdState){
            ctrl.state.execCmdState = currentCmdState || ctrl.state.execCmdState;
            _stopPeriodicCall(periodicGetState);
        }

        function _sendCommand() {
            var commandToSend = {
                commandCode : ctrl.command.commandCode,
                timestamp : moment().utc()
            };

            return _checkRequiredParams()
                .then(function(){
                    commandToSend.params = angular.copy(ctrl.cmdParams);

                    // if command id == 100 condition is needed
                    commandToSend.params[0]['filename'] = ctrl.fileControl.getFileData().name;

                    ctrl.progressMode = true;
                    _startWaitingState(ctrl.CMD_STATES.POSTED);
                    
                    DeviceCommandsFactory
                        .sendCommandToDevice(commandToSend)
                        .then(function(response) {
                            $log.info("Command Response:", response);
                            ctrl.state.commandId = response.commandId;
                            //_startWaitingState(ctrl.CMD_STATES.POSTED);
                        })
                        .catch(function(error) {
                            $log.error('sendCommand failed:', error);
                            ctrl.progressMode = false;
                            ctrl.state.execCmdState = ctrl.CMD_STATES.ERROR;
                        });
                })
                .catch(function(error){
                    _showError(error);
                    $log.error('sendCommand failed:', error);
                });
        }
        //endregion

        //region UI Actions
        ctrl.refreshState = function() {
            if(ctrl.state.execCmdState === ctrl.CMD_STATES.UPLOADING){
                return;
            }
            if (ctrl.state.execCmdState !== ctrl.CMD_STATES.POSTED && ctrl.state.execCmdState !== ctrl.CMD_STATES.PENDING) {
                _stopWaitingState();
                return
            }

            DeviceCommandsFactory
                .getCommandExecState(ctrl.state.commandId)
                .then(function(response) {
                    ctrl.state.execCmdState = response ? response.runState : ctrl.CMD_STATES.IDLE;
                    ctrl.state.metaData = response.runStateMetadata || {};
                })
                .catch(function(error){
                    $log.error("refreshState failed: ",error)
                })
        };

        ctrl.send = function(){
            _confirmation()
                .then(function(){
                    return _filesProcessing();
                })
                .then(function(){
                    return _sendCommand();
                })
            ;
        };

        ctrl.resetState = function(){
            ctrl.state.execCmdState = ctrl.CMD_STATES.IDLE;
            _resetState()
        };
        //endregion

        $rootScope.$on('deviceStatusDataReceived', function(event) {
            ctrl.deviceIsOnline = DeviceFactory.isDeviceOnline();
        });

        ctrl.$onDestroy = function () {
            _resetState();
        };
        //endregion
  }
})();