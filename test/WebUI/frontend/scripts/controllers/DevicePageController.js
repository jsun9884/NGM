'use strict';

(function(angular) {
    angular
        .module('NgmAdmin')
        .controller('DevicePageController',[
            'DeviceFactory',
            '$log',
            '$scope',
            DevicePageController
        ]);

    function DevicePageController(DeviceFactory, $log, $scope){
        $log.info("DevicePageController Loaded");
        
        DeviceFactory.startSurveyStatus();
        DeviceFactory.startSurveyRegularData();

        //region Event Handlers
        $scope.$on("$destroy",function(){
            $log.warn("destroy DevicePageController event");
            DeviceFactory.stopSurveyData();
        });
        //endregion
    }
})(angular);