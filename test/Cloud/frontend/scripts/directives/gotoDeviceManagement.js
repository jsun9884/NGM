'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .directive('gotoDeviceManagement',[
            '$state',
            'DeviceListFactory',
            'ActiveDeviceFactory',
            gotoDeviceManagement
        ]);

    function gotoDeviceManagement($state, DeviceListFactory, ActiveDeviceFactory){
        return {
            link: function(scope, element, attributes){
                element.on('click', function(event) {

                    var device = DeviceListFactory.getDevicesForce().find(function(device){
                        return device.thingName === attributes.deviceThingName
                    });

                    ActiveDeviceFactory.setActiveDevice(device);
                    $state.go('dashboard.devices.general');
                });
            }
        };
    }
})(angular);