'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelPowerDataWidget', {
            templateUrl: 'scripts/components/panelPowerDataWidget-tmpl.html',
            controller: PanelDataWidgetCtrl
        })
    ;

    function PanelDataWidgetCtrl($log, $rootScope, $scope) {

        $log.info("PanelDataWidgetCtrl Loaded");

        const ctrl = this;
        ctrl.data = {};
        ctrl.scheme = [
            {
                'name': 'active_power',
                'title': 'Active power'
            },
            {
                'name': 'apparent_power',
                'title': 'Apparent power'
            },
            {
                'name': 'current',
                'title': 'Current'
            },
            {
                'name': 'frequency',
                'title': 'Frequency'
            },
            {
                'name': 'power_factor',
                'title': 'Power factor'
            },
            {
                'name': 'reactive_power',
                'title': 'Reactive power'
            },
            {
                'name': 'voltage',
                'title': 'Voltage'
            }
        ];

        $scope.expanded = false;
        $scope.toggle = function(){
            $scope.expanded = !$scope.expanded;
        };

        ctrl.toggleExpansion = function (item) {
            item.expanded = !item.expanded;
        };


        //region Event Handlers
        $rootScope.$on('deviceSensorDataReceived', function(event, regular) {
            ctrl.data = regular.payload.sensors.find(function(sensor){
                return sensor.name == 'PowerMeter';
            });
        });

    }
})();