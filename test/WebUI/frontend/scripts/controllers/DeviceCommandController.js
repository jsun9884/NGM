'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DeviceCommandController',[
            '$scope',
            'DeviceCommandsFactory',
            DeviceCommandController
        ]);

    function DeviceCommandController($scope, DeviceCommandsFactory) {
        //region Init
        var commandsCtrl = this;
        //endregion

        //region UI Actions
        commandsCtrl.getCommandsList = function() {
            return DeviceCommandsFactory.listCommands()
        };
        //endregion

    }
})();