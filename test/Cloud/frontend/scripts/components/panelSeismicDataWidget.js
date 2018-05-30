'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelSeismicDataWidget', {
            templateUrl: 'scripts/components/panelSeismicDataWidget-tmpl.html',
            controller: PanelSeismicDataWidgetCtrl,
            bindings: {
                cardName: '@'
            }
        })
    ;

    function PanelSeismicDataWidgetCtrl($rootScope, $scope, SENSORS_CARDS) {

        // region Init
        const ctrl = this;
        ctrl.data = {};
        ctrl.card = SENSORS_CARDS.find(function(card){
            return card.name == ctrl.cardName;
        });

        $scope.selectedParam = {
            'sensor_name': '',
            'name': '',
            'title': ''
        };

        $scope.expanded = true;
        // endregion

        // region UI events
        $scope.toggle = function(){
            $scope.expanded = !$scope.expanded;
        };

        $scope.itemSelected = function(sensor_name, item){
            $scope.selectedParam.sensor_name = sensor_name;
            $scope.selectedParam.name = item.name;
            $scope.selectedParam.title = item.title;

            $rootScope.$broadcast('panelItemSelected', $scope.selectedParam);
        };

        $scope.getItemValue = function(item, item_scheme) {
            if(!item_scheme) {
                return '';
            }
            if(item_scheme.name == 'timestamp') {
                var s = new Date(angular.isObject(item.value[item_scheme.name]) ? item.value[item_scheme.name].current_value : item.value[item_scheme.name]);
                return s.toUTCString();
            }
            var val = angular.isObject(item.value[item_scheme.name]) ? item.value[item_scheme.name].current_value : item.value[item_scheme.name];
            if(!val) {
                return val;
            }
            var roundVal = Math.round(val);
            if(roundVal === val) {
                return val;
            }
            return val.toFixed(3);
        }

        // endregion

        // region Private methods
        function ApplyByScheme(sensors_data){
            var tempSensor = null;

            ctrl.data.sensor = ctrl.card.sensors.reduce(function(accum, current_card_sensor){
                tempSensor = sensors_data.find(function(payload_sensor){
                    return current_card_sensor.name == payload_sensor.name;
                });

                if(!!tempSensor){
                    var item = {
                        name: tempSensor.name,
                        title: ctrl.card.sensors.find(function(sensor){
                            return sensor.name == tempSensor.name;
                        }).title,
                        scheme: current_card_sensor.scheme,
                        value: {}
                    };
                    current_card_sensor.scheme.map(function(scheme_item){
                        item.value[scheme_item.name] = tempSensor[scheme_item.name];
                    });
                    accum.push(item);
                }

                return accum;
            }, []);
        }

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            ctrl.data = {};
        });

        $rootScope.$on('activeDeviceStatusDataReceived', function(event, status) {
            //ctrl.data.status = status.payload;
        });

        $rootScope.$on('activeDeviceSensorDataReceived', function(event, regular) {
            // filter payload sensors data by card sensor names
            ApplyByScheme(regular.payload.sensors.filter(function(payload_sensor){
                return !!ctrl.card.sensors.find(function(card_sensor){
                    return card_sensor.name == payload_sensor.name;
                });
            }));
        });
        // endregion
    }
})();