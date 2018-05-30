'use strict';

(function(angular) {
    angular
        .module('NgmAdmin')
        .controller('EventsController',[
            '$scope',
            'HistoryFactory',
            'ActiveDeviceFactory',
            EventsController
        ]);

    function EventsController($scope, HistoryFactory, ActiveDeviceFactory){

        //region Init
        $scope.selected = [];
        $scope.filter = {
            options: {
                debounce: 500
            }
        };

        var activeDevice = ActiveDeviceFactory.getActiveDevice();
        $scope.query = {
            deviceId:  activeDevice ? activeDevice.thingName : null,
            params:     {
                dateFrom:   '',
                dateTo:     '',
                max_id:     null,
                since_id:   null,
                limit:      25
            }
        };

        $scope.eventTypes = [
            'BLE_DISCONNECT',
            'BLE_CONNECT',
            'DEVICE_CONNECTED',
            'DEVICE_DISCONNECTED',
            'HIGH_RESOLUTION_DATA_REQUESTED',
            'HIGH_RESOLUTION_DATA_RECEIVED',
            'CONFIGURATION_CHANGED',
        ];

        $scope.eventSources = [
            'DEVICE',
            'WEB_CONSOLE',
            'WEB_UI'
        ];
        //endregion

        $scope.clearData = function() {
            $scope.events = {};
        };

        //region UI actions
        $scope.removeFilter = function () {
            $scope.filter.show = false;

            if($scope.filter.form.$dirty) {
                $scope.filter.form.$setPristine();
            }
        };

        $scope.postRequest = function(){

            // replace by rising button
            if(!$scope.query.params.dateFrom || !$scope.query.params.dateTo){
                return;
            }
            //

            var query = angular.copy($scope.query);
            query.params.dateFrom = moment.utc(query.params.dateFrom).unix();
            query.params.dateTo = moment.utc(query.params.dateTo).unix();

            $scope.promise = HistoryFactory
                .getEvents(query)
                .then(function(result){
                    $scope.events = result.data;
                    $scope.query.params.max_id = result.search_metadata.max_id;
                    $scope.query.params.since_id = result.search_metadata.since_id;
                })
            ;

        };

        $scope.newRequest = function(){
            $scope.query.params.max_id = null;
            $scope.query.params.since_id = null;
            $scope.postRequest();
        };

        $scope.prevPage = function(){
            $scope.query.params.since_id = null;
            $scope.postRequest();
        };

        $scope.nextPage = function(){
            $scope.query.params.max_id = null;
            $scope.postRequest();
        };
        //endregion

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            $scope.query.deviceId = activeDevice.thingName;
        });

        //endregion

    }
})(angular);