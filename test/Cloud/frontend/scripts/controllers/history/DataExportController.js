'use strict';

(function(angular) {
    angular
        .module('NgmAdmin')
        .controller('DataExportController',[
            '$scope',
            'FileSaver',
            'HistoryFactory',
            'ActiveDeviceFactory',
            DataExportController
        ]);

    function DataExportController($scope, FileSaver, HistoryFactory, ActiveDeviceFactory){

        //region Init
        $scope.eventsFromMinDate = '00:00:00 01/01/2015';
        $scope.eventsFromMaxDate = '00:00:00 01/01/2115';
        $scope.waitingForEvents = false;
        $scope.waitingForRegData = false;

        var activeDevice = ActiveDeviceFactory.getActiveDevice();
        $scope.preQuery = {
            deviceId    : activeDevice ? activeDevice.thingName : null,
            rangerValue : 40,
            params      : {
                eventsDateFrom  :   '',
                eventsDateTo    :   '',
                regDataDateFrom :   '',
                regDataDateTo   :   ''
            }
        };
        //endregion

        //region Private methods
        //endregion

        //region UI actions
        $scope.downloadEvents = function () {
            // replace by rising button
            if(!$scope.preQuery.params.eventsDateFrom || !$scope.preQuery.params.eventsDateTo){
                return;
            }
            //
    
            $scope.waitingForEvents = true;
            var query = {
                deviceId    : $scope.preQuery.deviceId,
                params      : {
                    dateFrom    : moment.utc($scope.preQuery.params.eventsDateFrom).unix(),
                    dateTo      : moment.utc($scope.preQuery.params.eventsDateTo).unix()
                }
            };

            $scope.promise = HistoryFactory
                .exportEvents(query)
                .then(function(result){
                    var blob = new Blob([result.fileContent], {
                        type: 'text/csv'
                    });
                    $scope.waitingForEvents = false;
                    FileSaver.saveAs(blob, result.fileName);
                })
            ;
        };

        $scope.downloadRegData = function () {
            // replace by rising button
            if(!$scope.preQuery.params.regDataDateFrom){
                return;
            }
            //
    
            $scope.waitingForRegData = true;
            var query = {
                deviceId    : $scope.preQuery.deviceId,
                params      : {
                    dateFrom    : moment.utc($scope.preQuery.params.regDataDateFrom).unix(),
                    dateTo      : moment.utc($scope.preQuery.params.regDataDateTo).unix(),
                }
            };

            $scope.promise = HistoryFactory
                .exportRegularData(query)
                .then(function(result){
                    var blob = new Blob([result.fileContent], {
                        type: 'text/csv'
                    });
                    $scope.waitingForRegData = false;
                    FileSaver.saveAs(blob, result.fileName);
                })
            ;
        };

        //endregion

        //region Event Handlers
        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            $scope.preQuery.deviceId = activeDevice.thingName;
        });
        //endregion

    }
})(angular);