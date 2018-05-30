'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelPowerLagLeadWidget', {
            templateUrl: 'scripts/components/panelPowerLagLeadWidget-tmpl.html',
            controller: PanelPowerLagLeadWidgetCtrl,
            bindings: {
                cardName: '@',
                sourcePrefix: '@',
            }
        })
    ;

    function PanelPowerLagLeadWidgetCtrl($rootScope, $scope, SENSORS_CARDS, ActiveDeviceFactory) {

        // region Init
        const ctrl = this;
        ctrl.data = {
            sensor: [],
            status: []
        };
        ctrl.card = SENSORS_CARDS.find(function(card){
            return card.name == ctrl.cardName;
        });

        // endregion

        // region UI events
        // endregion

        // region Private methods
        function ApplyByScheme(sensors_data){
            var sensorData = sensors_data.find(function(payload_sensor){
                return !!ctrl.card.sensors.find(function(card_sensor){
                    return card_sensor.name == payload_sensor.name;
                });
            });


            if(!!sensorData){
                var power1 = {
                    active: angular.isObject(sensorData[ctrl.sourcePrefix +'active_power1']) ? sensorData[ctrl.sourcePrefix +'active_power1'].current_value : sensorData[ctrl.sourcePrefix +'active_power1'],
                    reactive: angular.isObject(sensorData[ctrl.sourcePrefix +'reactive_power1']) ? sensorData[ctrl.sourcePrefix +'reactive_power1'].current_value : sensorData[ctrl.sourcePrefix +'reactive_power1']
                };
                var power2 = {
                    active: angular.isObject(sensorData[ctrl.sourcePrefix +'active_power2']) ? sensorData[ctrl.sourcePrefix +'active_power2'].current_value : sensorData[ctrl.sourcePrefix +'active_power2'],
                    reactive: angular.isObject(sensorData[ctrl.sourcePrefix +'reactive_power2']) ? sensorData[ctrl.sourcePrefix +'reactive_power2'].current_value : sensorData[ctrl.sourcePrefix +'reactive_power2']
                };
                var power3 = {
                    active: angular.isObject(sensorData[ctrl.sourcePrefix +'active_power3']) ? sensorData[ctrl.sourcePrefix +'active_power3'].current_value : sensorData[ctrl.sourcePrefix +'active_power3'],
                    reactive: angular.isObject(sensorData[ctrl.sourcePrefix +'reactive_power3']) ? sensorData[ctrl.sourcePrefix +'reactive_power3'].current_value : sensorData[ctrl.sourcePrefix +'reactive_power3']
                };

                ctrl.data.sensor = [
                    {
                        quadrantTitle: 'Quadrant&nbsp;1 Lag/Inductive',
                        signTitle: 'Delivered',

                        enabledPhase1: (power1.active >= 0 && power1.reactive >= 0),
                        enabledPhase2: (power2.active >= 0 && power2.reactive >= 0),
                        enabledPhase3: (power3.active >= 0 && power3.reactive >= 0)
                    },
                    {
                        quadrantTitle: 'Quadrant&nbsp;2 Lead/Inductive',
                        signTitle: 'Received',

                        enabledPhase1: (power1.active < 0 && power1.reactive >= 0),
                        enabledPhase2: (power2.active < 0 && power2.reactive >= 0),
                        enabledPhase3: (power3.active < 0 && power3.reactive >= 0)
                    },
                    {
                        quadrantTitle: 'Quadrant&nbsp;3 Lag/Capacitive',
                        signTitle: 'Received',

                        enabledPhase1: (power1.active < 0 && power1.reactive < 0),
                        enabledPhase2: (power2.active < 0 && power2.reactive < 0),
                        enabledPhase3: (power3.active < 0 && power3.reactive < 0)
                    },
                    {
                        quadrantTitle: 'Quadrant&nbsp;4 Lead/Capacitive',
                        signTitle: 'Delivered',

                        enabledPhase1: (power1.active >= 0 && power1.reactive < 0),
                        enabledPhase2: (power2.active >= 0 && power2.reactive < 0),
                        enabledPhase3: (power3.active >= 0 && power3.reactive < 0)
                    },
                ];
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