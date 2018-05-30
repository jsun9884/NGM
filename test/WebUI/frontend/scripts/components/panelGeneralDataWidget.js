'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelGeneralDataWidget', {
            templateUrl: 'scripts/components/panelGeneralDataWidget-tmpl.html',
            controller: panelGeneralDataWidgetCtrl
        })
    ;

    function panelGeneralDataWidgetCtrl($log, $rootScope, $scope) {

        $log.info("panelGeneralDataWidgetCtrl Loaded");

        const ctrl = this;
        ctrl.data = {};

        $scope.expanded = false;
        $scope.toggle = function(){
            $scope.expanded = !$scope.expanded;
        };

        //region Event Handlers
        $rootScope.$on('deviceSensorDataReceived', function(event, regular) {
            ctrl.data = regular.payload.sensors.filter(function(sensor){
                return (sensor.name == 'TempPCB' || sensor.name == 'TempCPU' || sensor.name == 'TamperSwitch' || sensor.name == 'GPS');
            });
        });

    }
})();