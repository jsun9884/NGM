'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DevicePowerController',[
            '$scope',
            'ActiveDeviceFactory',
            'SENSORS_CARDS',
            DevicePowerController
        ]);

    function DevicePowerController($scope, ActiveDeviceFactory, SENSORS_CARDS) {
        // region Init
        var devPowerCtrl = this;

        devPowerCtrl.sensorsCards = SENSORS_CARDS.filter(function(card){
            return card.type == 'power'
        });
        devPowerCtrl.sensors = [];
        // endregion

        // region Private methods
        function init() {
            devPowerCtrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
        }
        init();
        // endregion

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice){
            devPowerCtrl.activeDevice = activeDevice;
            devPowerCtrl.widgets = [];
        });
        //endregion
    }
})();