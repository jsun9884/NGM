'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .component('eventListWidget', {
            templateUrl: 'scripts/components/eventListWidget-tmpl.html',
            controller: [
                'HistoryFactory',
                'PollingFactory',
                'DeviceListFactory',
                EventListWidgetController
            ]
        });

    function EventListWidgetController(HistoryFactory, PollingFactory, DeviceListFactory) {

        // region Init
        const ctrl = this;
        ctrl.events = [];
        ctrl.frozen = false;
        var poller = PollingFactory.getPoller();
        // endregion

        // region UI events
        ctrl.toggleExpansion = function (evt) {
            evt.expanded = !evt.expanded;
        };

        ctrl.refreshOnlineStatus = function () {
            HistoryFactory
                .getEventsRecent()
                .then(function (events) {
                    ctrl.events = events.map(function(event){
                        var device = DeviceListFactory.getDeviceForceById(event.deviceId);
                        event.deviceLogicName = device ? device.attributes.LogicName : 'unknown';
                        return event;
                    });
                })
                .catch(function (error) {
                    console.log(error);
                });
        };

        ctrl.freezePoller = function(freeze){
            ctrl.frozen = freeze;
        };
        // endregion

        // region Init
        poller.go(function(){
            if(!ctrl.frozen || !ctrl.events.length){
                ctrl.refreshOnlineStatus();
            }
        }, 1000, 0);
        // endregion

        // region event actions
        ctrl.$onDestroy = function () {
            if(poller){
                poller.stop();
            }
        };
        // endregion
    }
})();