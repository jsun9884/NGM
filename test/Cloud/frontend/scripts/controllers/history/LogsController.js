'use strict';

(function(angular) {
    angular
        .module('NgmAdmin')
        .controller('LogsController',[
            'HistoryFactory',
            '$scope',
            '$rootScope',
            LogsController
        ]);

    function LogsController(HistoryFactory, $scope, $rootScope){

        //region Init
        $scope.selected = [];

        $scope.filter = {
            options: {
                debounce: 500
            }
        };

        $scope.query = {
            filter: {
                dateFrom: '',
                dateTo: '',
                deviceId: '',
                command: '',
                result: '',
            },
            order: 'deviceId',
            limit: 5,
            page: 1
        };

        $scope.commands = [
            'UPGRADE',
            'TI_RESET',
            'SOM_RESET',
            'GET_HR_DATA',
        ];

        $scope.results = [
            'DONE',
            'REJECTED',
            'TIMEOUT',
        ];

        HistoryFactory
            .getLogs($scope.query)
            .then(success);
        //endregion

        $scope.clearData = function() {
            $scope.logs = {};
        };

        function success(result) {
            $scope.clearData();
            $scope.logs = result.data;
        }

        //region UI actions
        $scope.logsReorder = function (order) {
            $scope.promise = HistoryFactory
                .getLogs($scope.query)
                .then(success);
        };

        $scope.getLogs = function () {
            $scope.promise = HistoryFactory
                .getLogs($scope.query)
                .then(success);
        };

        $scope.removeFilter = function () {
            $scope.filter.show = false;

            if($scope.filter.form.$dirty) {
                $scope.filter.form.$setPristine();
            }
        };
        //endregion

        //region Watchers
        $scope.$watch('[query.filter.dateFrom, query.filter.dateTo, query.filter.commands, query.filter.results]', function(newValue, oldValue) {
            if(newValue !== oldValue){
                $scope.getLogs();
            }
        });
        //endregion

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            $scope.query.filter.deviceId = activeDevice.thingName;
            $scope.getLogs();
        });
        //endregion

    }
})(angular);