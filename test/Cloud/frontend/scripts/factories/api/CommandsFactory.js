'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('CommandsFactory',[
            '$rootScope',
            'ApiFactory',
            'COMMANDS_LIST',
            CommandsFactory
        ]);

    function CommandsFactory($rootScope, ApiFactory, COMMANDS_LIST) {
        var commands = COMMANDS_LIST;
        
        function _getCommandsList () {
            return commands.filter(function(command){
                    return !command.roles || command.roles.indexOf($rootScope.userProfile.role) !== -1;
                }) || [];
        }

        function _postCommandToDevice(deviceId, command) {
            return ApiFactory
                .postDeviceCommand(deviceId, command)
                .then(function(response){
                    return response.data
                })
            ;
        }

        function _getCommandExecutionState(deviceId, commandUuid) {
            return ApiFactory
                .getCommandExecState(deviceId, commandUuid)
                .then(function(response){
                   return response.data
                })
            ;
        }

        return {
            listCommands                    : _getCommandsList,
            sendCommandToDevice             : _postCommandToDevice,
            getCommandExecState             : _getCommandExecutionState
        }
    }
})(angular);