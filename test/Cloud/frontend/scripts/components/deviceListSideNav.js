'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('deviceListSideNav', {
            templateUrl: 'scripts/components/deviceListSideNav-tmpl.html',
            controller: DeviceListSideNavCtrl
        })
    ;

    function DeviceListSideNavCtrl($scope, DeviceListFactory, ActiveDeviceFactory) {
        //region Init
        const ctrl = this;

        ctrl.sideNavIsOpened = true;
        ctrl.searchPhrase = '';
        ctrl.devices = DeviceListFactory.getDevicesForce();
        //endregion

        //region UI Actions
        ctrl.toggle = function () {
            document.getElementById('#devices-page-container');// //content-bar-area
            ctrl.sideNavIsOpened = !ctrl.sideNavIsOpened;
        };

        ctrl.selectActiveDevice = function ($event, device) {
            ActiveDeviceFactory.setActiveDevice(device);
        };

        ctrl.isActive = function(device) {
            return ActiveDeviceFactory.isDeviceActive(device)
        };
        //endregion

        //region Event Handlers
        $scope.$on("devicesPresenceRefreshed", function(){
            // if ctrl.devices list exists push new devices only or replace props
            if(ctrl.devices.length){
                DeviceListFactory.getDevicesForce().map(function(response_device){
                    if(!ctrl.devices.find(function(device, idx){
                        if(device.thingName == response_device.thingName){
                            for (var key in response_device) {
                                if (response_device.hasOwnProperty(key)) {
                                    ctrl.devices[idx][key] = response_device[key];
                                }
                            }
                            return true;
                        }
                        return false;
                    })){
                        ctrl.devices.push(response_device);
                    }
                });
            }
            else{
                ctrl.devices = DeviceListFactory.getDevicesForce();
            }
            ctrl.devices.sort(function(a, b) {
                if (a.online && !b.online) {
                    return -1;
                }
                else if (b.online && !a.online) {
                    return 1
                }
                if (!a.thingName)
                {
                    return 1;
                }
                return a.thingName.localeCompare(b.thingName);

            });

        });
        //endregion

    }
})();