'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('deviceConfigWidget', {
            templateUrl: 'scripts/components/deviceConfigWidget-tmpl.html',
            controller: DeviceConfigWidgetController
        });

    function DeviceConfigWidgetController($rootScope, $scope, $log, $mdDialog, $timeout, DeviceFactory) {
        //region Init
        const ctrl = this;

        console.log('deviceConfigWidget is loaded');

        ctrl.refresh = DeviceFactory.getDeviceConfig;

        function init() {
            ctrl.device = DeviceFactory.getDevice();
            ctrl.refresh();
        }

        init();

        ctrl.editConfig = function(ev){

            function filterOnlyChanges(newConfig){
                var filteredConfig = {};

                var General = {};

                // region General
                if(newConfig.General.GsmApn != ctrl.device.config.General.GsmApn){
                    General.GsmApn = newConfig.General.GsmApn;
                }
                if(newConfig.General.GsmNumber != ctrl.device.config.General.GsmNumber){
                    General.GsmNumber = newConfig.General.GsmNumber;
                }
                if(newConfig.General.GsmUsername != ctrl.device.config.General.GsmUsername){
                    General.GsmUsername = newConfig.General.GsmUsername;
                }
                if(newConfig.General.GsmPassword != ctrl.device.config.General.GsmPassword){
                    General.GsmPassword = newConfig.General.GsmPassword;
                }

                if(Object.getOwnPropertyNames(General).length){
                    filteredConfig.General = General;
                }
                // endregion


                return filteredConfig;
            }

            function postDeviceConfig(newConfig) {
                if (newConfig) {
                    newConfig = filterOnlyChanges(newConfig);
                    var config = {
                        payload : {
                            state: {
                                desired : newConfig
                            }
                        }
                    };

                    return DeviceFactory
                        .setDeviceConfig(config)
                        .then(function(response){
                            $timeout(function() {
                                ctrl.refresh();
                            }, 2000);
                            //return response.data
                        })
                    ;
                }
            }

            function DialogController($scope, $mdDialog, DeviceFactory) {

                $scope.config = {};

                var defaultConfig = {
                };

                ctrl.device = DeviceFactory.getDevice();

                if (ctrl.device.config) {
                    angular.copy(ctrl.device.config, $scope.config)
                } else {
                    angular.copy(ctrl.device.config, defaultConfig)
                }

                $scope.cancel = function() {
                    $mdDialog.hide(false);
                };
                $scope.answer = function() {
                    $mdDialog.hide($scope.config);
                };
            }
            $mdDialog
                .show({ controller          : DialogController,
                    templateUrl             : '/views/partials/config-dialog.tmpl.html',
                    parent                  : angular.element(document.body),
                    targetEvent             : ev,
                    clickOutsideToClose     : true
                })
                .then(function(config) {
                    $log.info("New Configuration:", config);
                    return config;
                })
                .then(postDeviceConfig)
                .catch($log.error)
        };

        //region Events Handling
        $rootScope.$on('deviceStatusDataReceived', function(event) {
            ctrl.device = DeviceFactory.getDevice();
        });
        //endregion

  }
})();