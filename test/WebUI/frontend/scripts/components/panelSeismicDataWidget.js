'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelSeismicDataWidget', {
            templateUrl: 'scripts/components/panelSeismicDataWidget-tmpl.html',
            controller: PanelSeismicDataWidgetCtrl
        })
    ;

    function PanelSeismicDataWidgetCtrl($log, $rootScope, $scope) {

        $log.info("PanelSeismicDataWidgetCtrl Loaded");

        const ctrl = this;
        ctrl.scheme = [
            {
                name: 'AccCore',
                title: 'Accelerometer (on board)'
            },
            {
                name: 'AccPCB',
                title: 'Accelerometer (external)'
            },
        ];

        ctrl.data = {

        };

        $scope.expanded = false;
        $scope.toggle = function(){
            $scope.expanded = !$scope.expanded;
        };

        ctrl.toggleExpansion = function (item) {
            item.expanded = !item.expanded;
        };


        //region Event Handlers
        $rootScope.$on('deviceSensorDataReceived', function(event, regular) {
            ctrl.data.AccCore = regular.payload.sensors.find(function(sensor){
                return sensor.name == 'AccCore';
            });
            ctrl.data.AccPCB = regular.payload.sensors.find(function(sensor){
                return sensor.name == 'AccPCB';
            });
        });

    }
})();