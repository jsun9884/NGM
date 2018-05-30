'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('pieChartWidget', {
            templateUrl: 'scripts/components/pieChartWidget-tmpl.html',
            controller: PieChartWidgetController,
            bindings: {
                command: '@'
            }
        });

    function PieChartWidgetController($scope, DeviceListFactory) {

        //region Init
        const ctrl = this;

        ctrl.options = {
            chart: {
                type: 'pieChart',
                margin : {
                    top: 20,
                    right: 10,
                    bottom: 10,
                    left: 10
                },
                x: function(d){return d.key;},
                y: function(d){return d.y;},
                showLabels: false,
                valueFormat : d3.format('d'),
                duration: 500,
                labelThreshold: 0.01,
                labelType : "value",
                labelSunbeamLayout: true,
                legend: {
                    margin: {
                        top: 5,
                        right: 5,
                        bottom: 5,
                        left: 0
                    }
                }
            }
        };
        // endregion

        // region Private methods
        function ReduceToStatuses(devices){
            return devices.reduce(function(prevValue, curValue){
                curValue.online ? prevValue.online++ : prevValue.offline++;
                return prevValue;
            },{
                online : 0,
                offline : 0
            });
        }
        // endregion

        // region Private functions
        // function RefreshOnlineStatus(devices){
        function RefreshOnlineStatus(){
            var devices = DeviceListFactory.getDevicesForce();
            var devicesStatuses = ReduceToStatuses(devices);
            ctrl.chartData = [
                {
                    key: "Online ("+ devicesStatuses.online +")",
                    y: devicesStatuses.online,
                    color: '#2196f3'
                },
                {
                    key: "Offline ("+ devicesStatuses.offline +")",
                    y: devicesStatuses.offline,
                    color: '#e53935'
                }
            ];
        }
        // endregion

        //region Event Handlers
        $scope.$on('devicesPresenceRefreshed', function() {
            RefreshOnlineStatus();
        });

//        $rootScope.$on('$windowResized', function(event) {
////            RefreshChartContainer();
//        });
        // endregion
    }
})();