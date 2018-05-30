'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DeviceSeismicController',[
            '$scope',
            '$rootScope',
            '$log',
            '$interval',
            'DeviceFactory',
            DeviceSeismicController
        ]);

    function DeviceSeismicController($scope, $rootScope, $log, $interval, DeviceFactory) {
        $log.info("DeviceSeismicController Loaded");

        var statusCtrl = this;


        statusCtrl.schemeStatus  =
            {
                title: 'Device Status',
                name: 'status',
                params: [
                    {
                        name: 'cpuTemperature',
                        title: 'CPU Temperature',
                    },
                    {
                        name: 'altitude',
                        title: 'Altitude'
                    },
                    {
                        name: 'latitude',
                        title: 'Latitude'
                    },
                    {
                        name: 'longtitude',
                        title: 'Longitude'
                    }
                ]
            }
        ;
        statusCtrl.schemeSensor  =
            {
                title: 'Sensor Data',
                name: 'sensor',
                params: [
                    {
                        name: 'state',
                        title: 'Sensor State',
                    },
                    {
                        name: 'voltage',
                        title: 'Voltage'
                    },
                    {
                        name: 'current',
                        title: 'Current',
                    },
                    {
                        name: 'active_power',
                        title: 'Active Power'
                    },
                    {
                        name: 'reactive_power',
                        title: 'Reactive Power',
                    },
                    {
                        name: 'apparent_power',
                        title: 'Apparent Power'
                    },
                    {
                        name: 'power_factor',
                        title: 'Power Factor',
                    },
                    {
                        name: 'frequency',
                        title: 'Frequency'
                    },
                ]
            }
        ;

        //region Event Handlers
        $scope.$on("$destroy",function(){
             $log.warn("DeviceStatusController destroy event");
        });
        //endregion
    }
})();