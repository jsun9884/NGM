'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('deviceConfigWidget', {
            templateUrl: 'scripts/components/deviceConfigWidget-tmpl.html',
            controller: DeviceConfigWidgetController,
        });

    function DeviceConfigWidgetController($scope, $mdDialog, DeviceFactory, PollingFactory, ActiveDeviceFactory) {
        //region Init
        const ctrl = this;

        var poller = PollingFactory.getPoller();

        ctrl.deltaConfig = {};
        ctrl.preConfig = {
            General : {}
        };
    
        ctrl.refresh = function(){
            var activeDevice = ActiveDeviceFactory.getActiveDevice();
            if(!!activeDevice){
                ActiveDeviceFactory
                .refreshActiveDeviceConfig()
                .then(function(delta){
                    ctrl.preConfig = ConvertToLocal(activeDevice.config);
                    ctrl.deltaConfig = !!delta ? delta : {};
                })
                ;
            }
        };
        ctrl.refresh();
        // endregion

        // region Dialog controller
        function ConvertToLocal(config){
            var preConfig = {
                General : {}
            };
            var originRate = config.General.RegularMsgTransmissionRate;
            preConfig.General.Rate = config.General.RegularMsgTransmissionRate;
            preConfig.General.Unit = 'sec';
            if(originRate > 59 && originRate < 3600 ){
                preConfig.General.Rate = Math.floor(originRate / 60);
                preConfig.General.Unit = 'min';
            }
            else if(originRate > 2599 ){
                preConfig.General.Rate = Math.floor(originRate / 3600);
                preConfig.General.Unit = 'hour';
            }
            
            return preConfig;
        }
    
        function convertFromLocal(config){
            var calcRate = 0;
            switch(ctrl.preConfig.General.Unit){
                case 'hour':
                    calcRate = ctrl.preConfig.General.Rate * 3600;
                    break;
                case 'min':
                    calcRate = ctrl.preConfig.General.Rate * 60;
                    break;
                case 'sec':
                default:
                    calcRate = ctrl.preConfig.General.Rate;
                    break;
            }
    
            if(calcRate !== ActiveDeviceFactory.getActiveDevice().config.General.RegularMsgTransmissionRate){
                config.General.RegularMsgTransmissionRate = calcRate;
            }
        
            return config;
        }
    
        ctrl.editConfig = function(ev){

            // region Private methods
            function filterOnlyChanges(newConfig){
                var filteredConfig = {};

                var General = {};
                var voltage = {};
                var current = {};
                var DataSwitch = {};
    
                // region General
                if(newConfig.General.GpsEnabled != ctrl.activeDevice.config.General.GpsEnabled){
                    General.GpsEnabled = newConfig.General.GpsEnabled;
                }
                if(newConfig.General.EventsEnabled != ctrl.activeDevice.config.General.EventsEnabled){
                    General.EventsEnabled = newConfig.General.EventsEnabled;
                }
                if(newConfig.General.AutoHrEnabled != ctrl.activeDevice.config.General.AutoHrEnabled){
                    General.AutoHrEnabled = newConfig.General.AutoHrEnabled;
                }
                if(newConfig.General.RegularMsgTransmissionRate != ctrl.activeDevice.config.General.RegularMsgTransmissionRate){
                    General.RegularMsgTransmissionRate = newConfig.General.RegularMsgTransmissionRate;
                }

                if(newConfig.General.GsmApn != ctrl.activeDevice.config.General.GsmApn){
                    General.GsmApn = newConfig.General.GsmApn;
                }
                if(newConfig.General.GsmNumber != ctrl.activeDevice.config.General.GsmNumber){
                    General.GsmNumber = newConfig.General.GsmNumber;
                }
                if(newConfig.General.GsmUsername != ctrl.activeDevice.config.General.GsmUsername){
                    General.GsmUsername = newConfig.General.GsmUsername;
                }
                if(newConfig.General.GsmPassword != ctrl.activeDevice.config.General.GsmPassword){
                    General.GsmPassword = newConfig.General.GsmPassword;
                }

                if(Object.getOwnPropertyNames(General).length){
                    filteredConfig.General = General;
                }
                // endregion

                // region Data Switch
                if(!!newConfig.DataSwitch){
                    if(newConfig.DataSwitch.PortAEnabled != ctrl.activeDevice.config.DataSwitch.PortAEnabled){
                        DataSwitch.PortAEnabled = newConfig.DataSwitch.PortAEnabled;
                    }
                    if(newConfig.DataSwitch.PortBEnabled != ctrl.activeDevice.config.DataSwitch.PortBEnabled){
                        DataSwitch.PortBEnabled = newConfig.DataSwitch.PortBEnabled;
                    }
                    if(Object.getOwnPropertyNames(DataSwitch).length){
                        filteredConfig.DataSwitch = DataSwitch;
                    }
                }
                // endregion

                // region Power meter
                if(!!newConfig.PowerMeter) {
                    if (newConfig.PowerMeter.HrData.voltage1.thresholdHigh != ctrl.activeDevice.config.PowerMeter.HrData.voltage1.thresholdHigh) {
                        voltage.thresholdHigh = parseFloat(newConfig.PowerMeter.HrData.voltage1.thresholdHigh);
                    }
                    if (newConfig.PowerMeter.HrData.voltage1.thresholdLow != ctrl.activeDevice.config.PowerMeter.HrData.voltage1.thresholdLow) {
                        voltage.thresholdLow = parseFloat(newConfig.PowerMeter.HrData.voltage1.thresholdLow);
                    }
                    if (newConfig.PowerMeter.HrData.current1.thresholdHigh != ctrl.activeDevice.config.PowerMeter.HrData.current1.thresholdHigh) {
                        current.thresholdHigh = parseFloat(newConfig.PowerMeter.HrData.current1.thresholdHigh);
                    }
                    if (newConfig.PowerMeter.HrData.current1.thresholdLow != ctrl.activeDevice.config.PowerMeter.HrData.current1.thresholdLow) {
                        current.thresholdLow = parseFloat(newConfig.PowerMeter.HrData.current1.thresholdLow);
                    }
                    if (Object.getOwnPropertyNames(voltage).length || Object.getOwnPropertyNames(current).length) {
                        filteredConfig.PowerMeter = {
                            HrData: {}
                        };

                        // apply for all phases
                        if (Object.getOwnPropertyNames(voltage).length) {
                            filteredConfig.PowerMeter.HrData.voltage1 = voltage;
                            filteredConfig.PowerMeter.HrData.voltage2 = voltage;
                            filteredConfig.PowerMeter.HrData.voltage3 = voltage;
                        }
                        if (Object.getOwnPropertyNames(current).length) {
                            filteredConfig.PowerMeter.HrData.current1 = current;
                            filteredConfig.PowerMeter.HrData.current2 = current;
                            filteredConfig.PowerMeter.HrData.current3 = current;
                        }
                    }
                }
                // endregion

                return filteredConfig;
            }

            function postDeviceConfig(newConfig) {
                if (newConfig) {
                    newConfig = filterOnlyChanges(newConfig);
                    var config = {
                        payload : {
                            state: {
                                desired : newConfig
                            }
                        }
                    };
					
                    return DeviceFactory
                        .setDeviceConfigById(ctrl.activeDevice.thingName, config)
                        .then(function(){
                            poller.go(function(){
                                return ActiveDeviceFactory
                                    .refreshActiveDeviceConfig()
                                    .then(function(delta){
                                        ctrl.deltaConfig = !!delta ? delta : {};
                                        // if(!ctrl.deltaConfig){
                                        if(!Object.getOwnPropertyNames(ctrl.deltaConfig).length){
                                            return true; // stop polling
                                        }
                                    })
                                ;
                            }, 1000, 10);
                        })
                    ;
                }
            }
            // endregion

            function DialogController($scope, $mdDialog, ActiveDeviceFactory) {

                // region Init
                $scope.config = {};
                $scope.preConfig = {};

                var defaultConfig = {
                    general : {
                        RegularMsgTransmissionRate: 1,
                        AutoHrEnabled: true,
                    },
                    PowerMeter : {
                        HrData : {
                            voltage1: {
                                thresholdHigh: 240,
                                thresholdLow: 210
                            },
                            current1: {
                                thresholdHigh: 5,
                                thresholdLow: 0
                            }
                        },
                        //zigBee : {
                        //    enabled : true
                        //}
                    },
                };

                var activeDevice = ActiveDeviceFactory.getActiveDevice();
                if (activeDevice.config) {
                    angular.copy(activeDevice.config, $scope.config);
                    $scope.preConfig = ConvertToLocal(activeDevice.config);
                } else {
                    angular.copy(activeDevice.config, defaultConfig)
                }
                // endregion

                // region Public methods
                $scope.cancel = function() {
                    $mdDialog.hide(false);
                };
                $scope.answer = function() {
                    $mdDialog.hide({
                        config: $scope.config,
                        preConfig: $scope.preConfig
                    });
                };
                // endregion

                $scope.$watch('preConfig.General.Unit', function(newValue){
                    if(newValue == 'hour' && $scope.preConfig.General.Rate > 24){
                        $scope.preConfig.General.Rate = 24;
                    }
                });
            }
            $mdDialog
                .show({
                    controller              : DialogController,
                    templateUrl             : '/views/partials/config-dialog.tmpl.html',
                    parent                  : angular.element(document.body),
                    targetEvent             : ev,
                    clickOutsideToClose     : true
                })
                .then(function(answer) {
                    if(!!answer){
                        ctrl.preConfig = answer.preConfig;
                        return postDeviceConfig(convertFromLocal(answer.config));
                    }
                })
            ;
        };

        //region Events Handling
        $scope.$on('$activeDeviceChanged', function(event, activeDevice){
            ctrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
            ctrl.refresh();
        });

        // $scope.$on('deviceListRefreshed', function(event, devices){
        $scope.$on('devicesPresenceRefreshed', function(){
            ctrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
            if(!!ctrl.activeDevice && !ctrl.activeDevice.config){
                ctrl.refresh();
            }
        });
        
        ctrl.$onDestroy = function () {
            if(poller){
                poller.stop();
            }
        };
        //endregion

  }
})();