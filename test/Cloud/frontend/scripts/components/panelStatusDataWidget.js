'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('panelStatusDataWidget', {
            templateUrl: 'scripts/components/panelStatusDataWidget-tmpl.html',
            controller: PanelStatusDataWidgetCtrl,
        })
    ;

    function PanelStatusDataWidgetCtrl($scope, $rootScope, ActiveDeviceFactory) {

        // region Init
        const ctrl = this;
        ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();
        // endregion

        // region Private functions
        function Reset(){
            ctrl.data = {
                'TempCPU':              'N/A',
                'TempPCB':              'N/A',
                'TamperSwitch':         'N/A',
                'FWVersion':            'N/A',
                'GsmRSSI':              'N/A',
                'GPS': {
                    latitude:           'N/A',
                    longitude:          'N/A',
                    satelliteNumber:    'N/A',
                    timestamp:          'N/A'
                },
                'LTE': {
                    rx:                 'N/A',
                    tx:                 'N/A',
                    timestamp:          'N/A'
                },
                'status': {
                    'TamperSwitch':     'N/A',
                    'network':          'N/A',
                    'PowerMeter':       'N/A',
                    'GPS':              'N/A',
                    'AccCore':          'N/A',
                    'AccPCB':           'N/A',
                    'LTE':              'N/A',

                    'bt':               'N/A',
                    'zb':               'N/A',
                    'data_switch':      'N/A'
                }
            };
        }
        // endregion

        Reset();

        // region Event handlers
        $scope.$on('devicesStatusRefreshed', function() {
            var activeDevice = ActiveDeviceFactory.getActiveDevice();
            if(!!activeDevice && !!activeDevice.status){
                if(angular.isArray(activeDevice.status.payload.sensors)){
                    activeDevice.status.payload.sensors.map(function(sensor){
                        ctrl.data.status[sensor.name] = sensor.state;
                    });
                }

                if(!!activeDevice.status.FWVersion){
                    ctrl.data.FWVersion = activeDevice.status.FWVersion;
                }
                if(!!activeDevice.status.GsmRSSI){
                    ctrl.data.GsmRSSI = activeDevice.status.GsmRSSI;
                }
            }

            ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();

        });

        $rootScope.$on('activeDeviceSensorDataReceived', function(event, regular) {
            regular.payload.sensors.map(function(sensor){
                if(sensor.name == 'GPS'){
                    ctrl.data.GPS.latitude = sensor.latitude;
                    ctrl.data.GPS.longitude = sensor.longitude;
                    ctrl.data.GPS.time = sensor.time;
                    ctrl.data.GPS.satelliteNumber = sensor.satelliteNumber;
                }

                if(sensor.name == 'TempPCB'){
                    ctrl.data.TempPCB = sensor.temperature;
                }
                if(sensor.name == 'TempCPU'){
                    ctrl.data.TempCPU = sensor.temperature;
                }
                if(sensor.name == 'AccCore'){
                    ctrl.data.AccCore = sensor.state;
                }
                if(sensor.name == 'AccPCB'){
                    ctrl.data.AccPCB = sensor.state;
                }
                if(sensor.name == 'TamperSwitch'){
                    ctrl.data.TamperSwitch = angular.isObject(sensor.closed) ? sensor.closed.current_value : sensor.closed;
                }
                if(sensor.name == 'LteModem'){
                    ctrl.data.status.LTE = sensor.state;

                    ctrl.data.LTE.rx = sensor.rx;
                    ctrl.data.LTE.tx = sensor.tx;
                    ctrl.data.LTE.time = sensor.timestamp;
                }
            });

        });

        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            Reset();
            ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();
        });
        // endregion

    }
})();