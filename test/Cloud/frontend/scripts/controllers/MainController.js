'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('MainController', [
            '$scope',
            '$rootScope',
            '$state',
            'ssSideNav',
            'PollingFactory',
            'DeviceListFactory',
            'ActiveDeviceFactory',
            MainController
        ]);

    function MainController($scope, $rootScope, $state, ssSideNav, PollingFactory, DeviceListFactory, ActiveDeviceFactory) {

        // region Init
        $scope.$state = $state;

        $scope.menu = ssSideNav;

        $scope.accountBoxExpanded = false;
        $rootScope.userProfile = JSON.parse(localStorage.getItem('profile'));

        $rootScope.shouldLockOpen = false;
        $rootScope.dataLoaded = true;

        // devices list polling per minute
        PollingFactory.getPoller().go(function(){
            RefreshDevices();
        }, 60000, 0);
        // first request result initiates device persistence
        RefreshDevices()
            .then(function(){
                PollingFactory.getPoller().go(function(){
                    DeviceListFactory.refreshDevicesPresence().then(function(){
                        DeviceListFactory.refreshDevicesStatus().then(function(){
                            if(ActiveDeviceFactory.isActiveDeviceOnline() && !!$state.current.rightSideNav){
                                ActiveDeviceFactory.refreshActiveDeviceRegularData();
                            }
                        });
                    });
                }, 2000, 0);
            })
        ;
        // endregion

        // region Private functions
        function RefreshDevices(){
            return DeviceListFactory
                .refreshDeviceList()
                .then(function(devices){
                    $rootScope.$broadcast('deviceListRefreshed', devices);
                })
            ;
        }
        
        // region UI events
        $scope.toggleAccountBoxExpansion = function(){
            $scope.accountBoxExpanded = !$scope.accountBoxExpanded;
        };

        $scope.signout = function() {
            $state.go('signin');
        };
        // region
    }
}());