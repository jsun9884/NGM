'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('DeviceCommandsFactory',[
            '$rootScope',
            'ApiFactory',
            DeviceCommandsFactory
        ]);

    function DeviceCommandsFactory($rootScope, ApiFactory) {

        var commands = [
            {
                displayName : 'Upgrade',
                commandCode   : 100,
                description : 'Perform firmware upgrade.',
                haveParams : true,
                roles: ['ADMIN'],
                params : [
                    {
                        name: 'firmware_file',
                        caption: 'Firmware file',
                        required: true,
                        filename: '',
                        attributes: {
                            'type' : 'file',
                            'repository': 'ngm-firmware',
                            'file-input' : "files",
                            'control': "$ctrl.fileControl"
                        },
                    },
                ]
            }
        ];

        function _getCommandsList () {
            return commands.filter(function(command){
                    return !command.roles || command.roles.indexOf($rootScope.userProfile.role) !== -1;
                }) || []
        }

        function _postCommandToDevice(command) {
            //var commandToSend = angular.copy(command);
            //delete commandToSend.params;
            return ApiFactory
                    .postDeviceCommand(command)
                    .then(function(response){
                        if(response.status !== 200){
                            throw response.data.message;
                        }
                        return response.data
                    })
        }

        function _getCommandExecutionState(deviceId, commandUuid) {
            return ApiFactory
                .getCommandExecState(deviceId, commandUuid)
                .then(function(response){
                   return response.data
                });
        }

        return {
            listCommands                    : _getCommandsList,
            sendCommandToDevice             : _postCommandToDevice,
            getCommandExecState             : _getCommandExecutionState
        }
    }
})(angular);