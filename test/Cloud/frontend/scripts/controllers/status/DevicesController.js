'use strict';

(function(angular) {
    angular
        .module('NgmAdmin')
        .controller('DevicesController',[
            '$scope',
            'ActiveDeviceFactory',
            DevicesController
        ]);

    function DevicesController($scope, ActiveDeviceFactory){

        // region Init
        var vm = this;
        vm.isDeviceSelected = ActiveDeviceFactory.isActiveDeviceSelected;
        // endregion

        //region Event Handlers
        //$scope.$on("$destroy", function(){
        //    ActiveDeviceFactory.disableActiveDeviceSelection();
        //});
        //endregion
    }
})(angular);