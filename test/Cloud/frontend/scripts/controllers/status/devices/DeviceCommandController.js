'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DeviceCommandController',[
            'COMMANDS_CARDS',
            DeviceCommandController
        ]);

    function DeviceCommandController(COMMANDS_CARDS) {
        //region Init
        var commandsCtrl = this;
        commandsCtrl.commandsCards = COMMANDS_CARDS;
        // endregion
    }
})();