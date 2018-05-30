'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DeviceConfigController',[
            '$scope',
            '$rootScope',
            '$log',
            '$mdDialog',
            'DeviceFactory',
            DeviceConfigController
        ]);

    function DeviceConfigController($scope, $rootScope, $log, $mdDialog, DeviceFactory) {
        $log.info("DeviceConfigController Loaded");

        var configCtrl = this;

    }
})();