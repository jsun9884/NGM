'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('DeviceListFactory',[
            '$log',
            '$rootScope',
            'DeviceFactory',
            DeviceListFactory
        ]);

    function DeviceListFactory($log, $rootScope, DeviceFactory){
        var devices = [];

        function _getDevicesForce(){
            return devices;
        }

        function _getDeviceForceById(deviceId){
            return devices.find(function(device){
                return device.thingName === deviceId;
            });
        }
        
        function _refreshDevicesStatus(){
            return DeviceFactory
                .getDevicesStatus(devices)
                .then(function(devicesWithStatus){
                    if(!!devices){
                        devicesWithStatus.map(function(deviceWithStatus){
                            if(!devices.find(function(device, idx){
                                    if(device.thingName == deviceWithStatus.thingName){
                                        for (var key in deviceWithStatus) {
                                            if (deviceWithStatus.hasOwnProperty(key)) {
                                                devices[idx][key] = deviceWithStatus[key];
                                            }
                                        }
                                        return true;
                                    }
                                    return false;
                                })){
                                devices.push(deviceWithStatus);
                            }
                        });
                
                    }
                    else{
                        devices = devicesWithStatus;
                    }
            
                    return devicesWithStatus;
                })
                .then(function(){
                    $rootScope.$broadcast('devicesStatusRefreshed');
                })
            ;
        }

        function _refreshDevicesPresence(){
            return DeviceFactory
                .getDevicesPresence() 
                .then(function(devicesPresences){
                    devices = (!!devices && angular.isArray(devices)) ? devices : [];
                    devices = devices.map(function(device){
                        devicesPresences.find(function(devicePresence){
                            if(devicePresence.deviceId === device.thingName){
                                device.online = !!devicePresence.isOnline;
                                return true;
                            }
                            return false;
                        });
                        return device;
                    })
                })
                .then(function(){
                    $rootScope.$broadcast('devicesPresenceRefreshed');
                })
            ;
        }
        
        function _refreshDeviceList() {
            return DeviceFactory
                .getDevices(true)
                .then(function(response_devices){
                    devices = response_devices;
                    return devices;
                })
                .catch(function(error){
                    $log.error('DevicesController init Failed :', error)
                })
            ;
        }

        return {
            refreshDeviceList           :  _refreshDeviceList,
            refreshDevicesPresence      :  _refreshDevicesPresence,
            refreshDevicesStatus        :  _refreshDevicesStatus,
            getDevicesForce             :  _getDevicesForce,
            getDeviceForceById          :  _getDeviceForceById
        }
    }
})(angular);