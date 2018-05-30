'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('CommandFlowFactory',[
            '$q',
            '$log',
            'DlgFactory',
            'ApiFactory',
            'PollingFactory',
            'CommandsFactory',
            'ActiveDeviceFactory',
            'CMD_STATES',
            'COMMANDS_LIST',
            CommandFlowFactory
        ]);

    function CommandFlowFactory($q, $log, DlgFactory, ApiFactory, PollingFactory, CommandsFactory, ActiveDeviceFactory, CMD_STATES, COMMANDS_LIST) {
        return {
            init: function(card, fileControl){

                // region Init
                var commandFlow = {
                    cmd: null,/*COMMANDS_LIST.find(function (COMMAND) {
                        return COMMAND.commandCode == card.cmdChains[0];
                    }),*/
                    cmdChains: card.cmdChains,
                    uid: moment(),
                    activeDeviceIsOnline: false,
                    periodicGetStateInterval: null,
                    progressMode: false,
                    resultMode: false,
                    //currentCommandNumber: 0,

                    // default state
                    state: {
                        execCmdState: CMD_STATES.IDLE,
                        commandId: "",
                        metaData: {
                            error: '',
                            message: ''
                        }
                    },

                    poller: PollingFactory.getPoller()
                };
                Reset();
                // endregion

                // region Private functions
                function Reset(){
                    commandFlow.cmd = COMMANDS_LIST.find(function (COMMAND) {
                        return COMMAND.commandCode == card.cmdChains[0];
                    });
                }

                function checkParams(params){
                    return params.reduce(function(result, param){
                        return result && (
                                !param.required || angular.isDefined(param.value) ||
                                (!!param.attributes && !!param.attributes.type && param.attributes.type == 'file')
                            );
                    }, true);
                }

                function isRequiredFileDefined(fileParams){
                    return (!fileParams.required || fileControl.isDefined());
                }

                function uploadFile(){
                    // TODO: params as object
                    return $q(function(resolve, reject) {
                        var filesParams = commandFlow.cmd.params ? commandFlow.cmd.params.filter(function(param){
                            return !!param.attributes && !!param.attributes.type && param.attributes.type == 'file'
                        }) : [];

                        // region Upload files (if they are in params) (only for one file is implemented)
                        if (filesParams.length) {
                            if(isRequiredFileDefined(filesParams[0])){
                                commandFlow.state.execCmdState = CMD_STATES.UPLOADING;
                                commandFlow.progressMode = true;

                                var fileName = null;
                                if(commandFlow.cmd.commandCode == 310)
                                {
                                    fileName = ActiveDeviceFactory.getActiveDevice().thingName + '.xml';
                                }
                                return fileControl
                                    .prepareFile(fileName)
                                    .then(fileControl.uploadFile)
                                    .then(function(){
                                        commandFlow.cmd.params[0]['filename'] = fileControl.getFileData().name;
                                        resolve();
                                    })
                                    .catch(function(){
                                        commandFlow.progressMode = false;
                                        //self.resultMode = false;
                                        alert('Bucket CORS problems?');
                                        reject();
                                    })
                                    ;
                            }
                            else{
                                DlgFactory.showMessage('Error', 'You have to specify required parameters');
                                reject();
                            }
                        }
                        // endregion
                        else{
                            resolve();
                        }
                    });
                }

                function openHRFile(url){
                    var pom = document.createElement('a');
                    pom.setAttribute('href', url);

                    if (document.createEvent) {
                        var event = document.createEvent('MouseEvents');
                        event.initEvent('click', true, true);
                        pom.dispatchEvent(event);
                    }
                    else {
                        pom.click();
                    }

                    //pom.remove();
                }

                function downloadFile(fileNameRemote, saveAs, repository){
                    var paramsToSend = {
                        'filename': fileNameRemote,
                        'repository': repository,
                        'operation': 'DOWNLOAD'
                    };

                    if(!!saveAs){
                        paramsToSend.saveas = saveAs;
                    }
                    return ApiFactory
                        .requestDownloadLink(paramsToSend)
                        .then(function(result){
                            if(result.status == 404){
                                throw new Error('File was not found');
                            }
                            return openHRFile(result.data.url);
                        })
                    ;
                }


                function sendCommand (commandCode, result) {
                    var commandToSend = {
                        commandCode : commandCode,
                        timestamp : moment().utc()
                    };

                    commandFlow.cmd = COMMANDS_LIST.find(function (COMMAND) {
                        return COMMAND.commandCode == commandCode;
                    });

                    if(!commandFlow.cmd.params || checkParams(commandFlow.cmd.params)){

                        // 330 || 320 command gets params from previous command result /       PREPARE_HR_DATA || GET_CFG_FILE
                        if(commandToSend.commandCode == 330) {
                            commandToSend.params = {
                                filename: commandFlow.state.commandId
                            };
                        }
                        else if(commandToSend.commandCode == 320 ) {
                            commandToSend.params = {
                                filename: ActiveDeviceFactory.getActiveDevice().thingName + '.xml'
                            };
                        }
                        else if(commandToSend.commandCode == 301) {
                            commandToSend.params = {
                                filename: ActiveDeviceFactory.getActiveDevice().thingName + '-AccRegDump.txt',
                                logicName: ActiveDeviceFactory.getActiveDevice().attributes.LogicName
                            };
                        }
                        else{
                            // params - as object
                            commandToSend.params = commandFlow.cmd.params.reduce(function(accum, current_param){
                                accum[current_param.name] = !!current_param.value ? current_param.value : current_param[current_param.name];
                                return accum;
                            }, {});
                        }

                        // send and wait
                        var deviceId = ActiveDeviceFactory.getActiveDevice().thingName;
                        return CommandsFactory
                            .sendCommandToDevice(deviceId, commandToSend)
                            .then(function(response) {
                                commandFlow.state.execCmdState = CMD_STATES.POSTED;
                                commandFlow.state.commandId = response.commandId;

                                commandFlow.progressMode = true;

                                return commandFlow.poller.go(function(){
                                    return refreshState()
                                }.bind(this), 1000, 600);

                            })
                            .then(function(result){
                                // region Download file /       PREPARE_HR_DATA || GET_CFG_FILE
                                if(commandToSend.commandCode == 330){
                                    return downloadFile(commandToSend.params.filename +'.csv', deviceId + '-' + moment.utc().format("HH:mm:ss-MM.DD.YYYY") + '.csv', 'HR_DATA') ;
                                }
                                else{
                                    if(commandToSend.commandCode == 320){
                                        return downloadFile(commandToSend.params.filename, null, 'CONFIG_FILES') ;
                                    }
                                    else if(commandToSend.commandCode == 301){
                                        return downloadFile(commandToSend.params.filename, null, 'CONFIG_FILES') ;
                                    }
                                }

                                return $q.resolve();
                                // endregion
                            })
                            .catch(function(error) {
                                commandFlow.resultMode = true;
                                commandFlow.progressMode = false;
                                $log.error('sendCommand failed: ', error);
                                return $q.reject();
                            })
                        ;
                    }
                    else{
                        DlgFactory.showMessage('Error', 'You have to specify required parameters');
                        return $q.reject();
                    }
                }

                function refreshState(){
                    // return undefined - waiting proceed
                    if(commandFlow.state.execCmdState === CMD_STATES.UPLOADING){
                        return;
                    }

                    // return true/false - result of waiting (waiting stop)
                    if (commandFlow.state.execCmdState !== CMD_STATES.POSTED
                        && commandFlow.state.execCmdState !== CMD_STATES.PENDING
                        && commandFlow.state.execCmdState !== CMD_STATES.ACCEPTED
                        && commandFlow.state.execCmdState !== CMD_STATES.PROGRESS
                    ) {

                        return commandFlow.state.execCmdState === CMD_STATES.DONE;
                    }

                    // return undefined - waiting proceed
                    CommandsFactory
                        .getCommandExecState(ActiveDeviceFactory.getActiveDevice().thingName, commandFlow.state.commandId)
                        .then(function(response) {
                            commandFlow.state.execCmdState = response ? response.runState : CMD_STATES.IDLE;
                            commandFlow.state.metaData = response.runStateMetadata || {};
                        })
                        .catch(function(error){
                            $log.error("refreshState failed: ", error);
                        })
                    ;
                }
                // endregion

                // region Public functions
                commandFlow.start = function(){
                    const self = this;

                    uploadFile().then(function(){

                        // region Command chain execution (promises chain implementation)
                        var results = [];
                        return self.cmdChains
                            .reduce(function(p, item) {
                                return p.then(function(result) {
                                    return sendCommand(item, result)
                                        .then(function(data) {
                                            results.push(data);
                                            return results;
                                        })
                                    ;
                                });
                            }, $q.resolve())
                            .then(function(){

                                // set back card
                                commandFlow.resultMode = true;
                                commandFlow.progressMode = false;
                                Reset();

                                console.log('>>', results);
                            })
                        ;
                        // endregion

                    })
                    .catch(function(){
                        //self.state.execCmdState = CMD_STATES.IDLE;
                    });
                };

                commandFlow.stop = function(){
                    commandFlow.poller.stop();
                };
                // endregion


                return commandFlow;
            }
        };
    }
})(angular);