'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('commandCardWidget', {
            templateUrl: 'scripts/components/commandCardWidget-tmpl.html',
            controller: CmdCardWidgetController,
            bindings: {
                card: '=',       // Two way
                parameter: '@parameter',
            }
        });

    function CmdCardWidgetController($scope, $rootScope, ActiveDeviceFactory, CommandFlowFactory, CMD_STATES) {

        //region Init
        const ctrl = this;

        ctrl.fileControl = {};
        ctrl.paramsExpanded = false;
        ctrl.activeDeviceIsOnline = false;

        ctrl.commandFlow = CommandFlowFactory.init(ctrl.card, ctrl.fileControl);

        ctrl.CMD_STATES = CMD_STATES;
        //endregion

        //region UI Methods
        ctrl.send = function(){
            if(ctrl.card.defaultParameter)
            {
                ctrl.commandFlow.cmd.params[0].value = ctrl.card.defaultParameter;
            }
            ctrl.commandFlow.start();
        };

        //endregion

        //region Event Handlers
        // $scope.$on('deviceListRefreshed', function(event) {
        $scope.$on('devicesPresenceRefreshed', function() {
            ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();
        });

        $scope.$on('$activeDeviceChanged', function(event, activeDevice) {
            ctrl.commandFlow.stop();
            ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();
        });

        ctrl.$onInit = function() {
            ctrl.activeDeviceIsOnline = ActiveDeviceFactory.isActiveDeviceOnline();
        };

        ctrl.$onDestroy = function () {
            ctrl.commandFlow.stop();
        };
        //endregion
  }
})();