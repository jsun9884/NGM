'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('googleMapWidget', {
            templateUrl: 'scripts/components/googleMapWidget-tmpl.html',
            controller: GoogleMapWidgetController,
            bindings: {
                command: '@'
            }
        });

    function GoogleMapWidgetController($scope, $state, $timeout, DeviceFactory, DeviceListFactory, ActiveDeviceFactory) {

        //region Init
        const ctrl = this;

        ctrl.center = {
            latitude: 31.727118,
            longitude: 34.917148
        };

        ctrl.zoom = 3;
        ctrl.markers = [];
        ctrl.expanded = false;
        // endregion

        // region private functions
        function RefreshOnlineStatus(devices){
            var devicesStatuses = devices.reduce(function(accum, devStatus){
                if(!!devStatus.location){

                    var latitude = parseFloat(angular.isObject(devStatus.location.latitude) ? devStatus.location.latitude.current_value : devStatus.location.latitude);
                    var longitude = parseFloat(angular.isObject(devStatus.location.longitude) ? devStatus.location.longitude.current_value : devStatus.location.longitude);

                    if(devStatus.online && Math.abs(latitude) && Math.abs(longitude)){
                        var status = {
                            location:{
                                latitude: latitude,
                                longitude: longitude,
                                altitude: 0
                            },
                            thingName: devStatus.thingName
                        };
                        accum.push(status);
                    }
                }
                return accum;
            }, []);

            devicesStatuses.map(function(devStatus){
                devStatus.options = {
                    labelClass: 'marker_labels',
                    labelAnchor: '12 60',
                    labelContent: devStatus.thingName,
                    title: 'Show devices status'
                };

                var flagExistence = false;
                ctrl.markers.forEach(function(marker_item, idx){
                    if(devStatus.thingName == marker_item.options.labelContent){
                        flagExistence = true;
                        ctrl.markers[idx].location = devStatus.location;
                    }
                });

                if(!flagExistence){
                    ctrl.markers.push(devStatus)
                }
            });
        }
        // endregion

        // region UI events
        ctrl.onMarkerClicked = function (thingName) {
            var device = ctrl.markers.find(function(marker){
                return marker.thingName === thingName
            });
            ActiveDeviceFactory.setActiveDevice(device);
            $state.go('dashboard.devices.general');
        };

        ctrl.expand = function(){
            ctrl.expanded = !ctrl.expanded;
            $timeout(function(){
                window.dispatchEvent(new Event('resize'));
            }, 300);
        };

        ctrl.widgetShowed = function(){
            $timeout(function(){
                window.dispatchEvent(new Event('resize'));
            }, 300);
        };

        //region Event Handlers
        $scope.$on('devicesPresenceRefreshed', function() {
            DeviceFactory
                .getDevicesLocation(DeviceListFactory.getDevicesForce())
                .then(function(devices){
                    RefreshOnlineStatus(devices);
                })
            ;
        });
        //endregion
  }
})();