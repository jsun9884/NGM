'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('DeviceFactory',[			
			'$rootScope',
            'ApiFactory',
            DeviceFactory]);

    function DeviceFactory($rootScope ,ApiFactory){

        function _getDevices() {
            return ApiFactory
                .getDevices()
                .then(function(response) {
                    return angular.copy(response.data)
                })
            ;
        }
        
        function _getDevicesPresence(){
            return ApiFactory
                .getDevicesPresence()
                .then(function(response) {
                    return angular.copy(response.data)
                })
            ;
        }

        function _getDevicesLocation(devices) {
            return ApiFactory
                .getDevicesLocation()
                .then(function (response) {
                    devices.map(function(device) {
                        if(!!device.thingName && device.online && angular.isArray(response.data)){
                            var responseDevice = response.data.find(function(response_item){
                                return response_item.deviceId == device.thingName;
                            });

                            if(!!responseDevice && !!responseDevice.location){
                                device.location = responseDevice.location;
                            }
                        }
                    });
                    return devices;
                })
            ;
        }

        function _getDevicesStatus(devices) {
            return ApiFactory
                .getDevicesStatus()
                .then(function (response) {
                    devices = (!!devices && angular.isArray(devices)) ? devices : [];
                    var status_devices = response.data;
                    if(angular.isArray(status_devices)){
                        return devices.map(function(device) {
                            device.status = status_devices.find(function(devStatus){
                                return devStatus.deviceId == device.thingName
                            });

                            if(!!device.status){
                                device.location = device.online ? {
                                    'latitude': device.status.payload.latitude,
                                    'longitude': device.status.payload.longitude
                                } : {};
                            }
                            return device;
                        });
                    }
                    // if no/erroneous status data received
                    return [];
                })
        }

        function _getDeviceStatusById(deviceId) {
            return ApiFactory.getDeviceStatusById(deviceId);
        }

        function _getDeviceConfigById(deviceId) {
            return ApiFactory
                .getDeviceConfigById(deviceId)
                .then(function(response){
                    return response.data
                })
        }
		
        function _setDeviceConfigById(deviceId, config) {
            return ApiFactory
                .setDeviceConfigById(deviceId, config)
                .then(function(response){					
					$rootScope.$broadcast('$onSetDeviceConfigById', config);
                    return response.data
                })
        }

        function _getDeviceRegularDataById(deviceId){
            return ApiFactory
                .getDeviceRegularDataById(deviceId)
                .then(function (response) {
                    return angular.copy(response.data);
                })
        }

        return {
            getDevices                  :  _getDevices,
            getDevicesStatus            :  _getDevicesStatus,
            getDevicesLocation          :  _getDevicesLocation,
            getDevicesPresence          :  _getDevicesPresence,
            getDeviceStatusById         :  _getDeviceStatusById,
            getDeviceConfigById         :  _getDeviceConfigById,
            setDeviceConfigById         :  _setDeviceConfigById,
            getDeviceRegularDataById    :  _getDeviceRegularDataById,
        }
    }
})(angular);