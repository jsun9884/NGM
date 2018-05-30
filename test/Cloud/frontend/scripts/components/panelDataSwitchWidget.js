'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelDataSwitchWidget', {
            templateUrl: 'scripts/components/panelDataSwitchWidget-tmpl.html',
            controller: PanelDataSwitchWidgetCtrl,
            bindings: {
                cardName: '@'
            }
        })
    ;

    function PanelDataSwitchWidgetCtrl($rootScope, $scope, SENSORS_CARDS, ActiveDeviceFactory) {

        // region Init
        const ctrl = this;
        ctrl.data = {
            sensor: [],
            status: []
        };
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
        ctrl.toggleExpansion = function (item) {
            item.expanded = !item.expanded;
        };

        $scope.itemSelected = function(sensor_name, item){
            $scope.selectedParam.sensor_name = sensor_name;
            $scope.selectedParam.name = item.name;
            $scope.selectedParam.title = item.title;

            $rootScope.$broadcast('panelItemSelected', $scope.selectedParam);
        };
        // endregion

        // region Private methods
        function ApplyByScheme(sensors_data){
            var sensorData = sensors_data.find(function(payload_sensor){
                return !!ctrl.card.sensors.find(function(card_sensor){
                    return card_sensor.name == payload_sensor.name;
                });
            });

            if(!!sensorData){
                ctrl.data.sensor = ctrl.card.sensors.find(function(card_sensor){
                    return card_sensor.name == sensorData.name;
                }).scheme.reduce(function(accum, current_scheme_item){

                    if(current_scheme_item.name == 'port'){
                        current_scheme_item.data.map(function(sub_item){
                            var item_data = {
                                name: sub_item.name,
                                title: sub_item.title,
                                value: {
                                    portA: sensorData['PortA_' + sub_item.name],
                                    portB: sensorData['PortB_' + sub_item.name],
                                }
                            };
                            if(!accum.find(function(item){
                                if(item.name == sub_item.name){
                                    item.value = item_data.value;
                                    return true;
                                }
                                return false;
                            })){
                                accum.push(item_data);
                            }
                        });
                    }
                    else{
                        var item_data = {
                            name: current_scheme_item.name,
                            title: current_scheme_item.title,
                            value: sensorData[current_scheme_item.name]
                        };

                        if(!accum.find(function(item){
                            if(item.name == current_scheme_item.name){
                                item.value = item_data.value;
                                return true;
                            }
                            return false;
                        })){
                            accum.push(item_data);
                        }
                    }
                    return accum;

                }, ctrl.data.sensor);
            }
        }
        // endregion

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            if(!activeDevice.online){
                ctrl.data = {
                    sensor: [],
                    status: []
                };
            }
        });

        $rootScope.$on('activeDeviceSensorDataReceived', function(event, regular) {
            if(ActiveDeviceFactory.isActiveDeviceOnline()){
                ApplyByScheme(regular.payload.sensors.filter(function(payload_sensor){
                    return !!ctrl.card.sensors.find(function(card_sensor){
                        return card_sensor.name == payload_sensor.name;
                    });
                }));
            }
        });

        $scope.$on("devicesPresenceRefreshed", function(){
            if(!ActiveDeviceFactory.isActiveDeviceOnline()){
                ctrl.data = {
                    sensor: [],
                    status: []
                };
            }
        });
        // endregion
    }
})();