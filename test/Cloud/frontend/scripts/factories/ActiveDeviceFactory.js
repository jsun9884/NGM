'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('ActiveDeviceFactory',[
            '$rootScope',
            'DeviceFactory',
            'DeviceListFactory',
            ActiveDeviceFactory
        ]);

    function ActiveDeviceFactory($rootScope, DeviceFactory, DeviceListFactory){

        var activeDevice = {
            thingName   : "",
            attributes  : {},
            config      : {},
            status      : {},
            regular     : {},
            online      : false,
            isSelected  : false
        };
		
		var refreshingDevice = false;

        function _setActiveDevice(device) {
            if(activeDevice.thingName != device.thingName){
                activeDevice.thingName      = device.thingName;
                activeDevice.attributes     = device.attributes;
                activeDevice.config         = {};
                activeDevice.status         = {};
                activeDevice.regular        = {};
                activeDevice.online         = device.online;
                activeDevice.isSelected     = true;

                $rootScope.$broadcast('$activeDeviceChanged', activeDevice);
            }
        }
        
        function _getActiveDevice() {
            return DeviceListFactory.getDevicesForce().find(function(device){
                return device.thingName == activeDevice.thingName;
            });
        }

        async function _refreshActiveDeviceConfig(){
			if(!refreshingDevice){
				//console.log('%c...............refreshing config'+ new Date(),'background: green; color: white');
				refreshingDevice = true;
				
				return new Promise(function(resolve,reject){
					DeviceFactory
					.getDeviceConfigById(activeDevice.thingName)
					.then(function(config){
						var activeDevice = _getActiveDevice();
						if (activeDevice) {						
							activeDevice.config = (!!config.state && !!config.state.reported) ? config.state.reported : {};	
							//activeDevice.config_delta = (!!config.state && !!config.state.delta) ? config.state.delta: {}													
							//return activeDevice.config;
							resolve( activeDevice.config );
						}
						refreshingDevice = false;
					})
				});
			}else{
				//console.log('%c...........not refreshing config'+ new Date(),'background: blue; color: white');
			}
        }

        function _refreshActiveDeviceRegularData() {
            return DeviceFactory
                .getDeviceRegularDataById(activeDevice.thingName)
                .then(function(regularData){
                    if (activeDevice) {
                        activeDevice.regular = regularData || {};
                        $rootScope.$broadcast('activeDeviceSensorDataReceived', activeDevice.regular);
                    }
                })
        }
        
        function _isActiveDeviceOnline() {
            return activeDevice.online
        }

        function _isActiveDeviceSelected() {
            return activeDevice.isSelected
        }

        function _isDeviceActive(device) {
            return device.thingName == activeDevice.thingName
        }

        // region Event handlers
        $rootScope.$on('deviceListRefreshed', function(event, devices){
            var device = null;
            if(!!activeDevice.thingName){
                device = devices.find(function(device){
                    return device.thingName == activeDevice.thingName;
                });
            }

            if(!!device){
                if(activeDevice.thingName == device.thingName){
                    for (var device_prop in device) {
                        if (device.hasOwnProperty(device_prop)) {
                            activeDevice[device_prop] = device[device_prop];
                        }
                    }
                }
                else{
                    activeDevice = device;
                }
            }
        });
        // endregion

        return {
            setActiveDevice                 : _setActiveDevice,
            getActiveDevice                 : _getActiveDevice,
            refreshActiveDeviceRegularData  : _refreshActiveDeviceRegularData,
            refreshActiveDeviceConfig       : _refreshActiveDeviceConfig,
            isActiveDeviceOnline            : _isActiveDeviceOnline,
            isActiveDeviceSelected          : _isActiveDeviceSelected,
            isDeviceActive                  : _isDeviceActive,
        }
    }
})(angular);