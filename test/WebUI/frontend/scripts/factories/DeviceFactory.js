'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('DeviceFactory',[
            '$rootScope',
            '$interval',
            'ApiFactory',
            DeviceFactory]);

    function DeviceFactory($rootScope, $interval, ApiFactory){

        // region Init
        var onlineTimeDelta = 10;
        var device = {
            thingName   : "",
            config      : {},
            status      : {},
            regular     : {},
            online      : false
        };

        var periodicGetState;
        var periodicGetRegularData;
        // endregion

        // region private methods
        function isOnline(status) {
            return true;

            return status
                ? moment.utc().diff(moment(status.timestamp,"x"), 'seconds') < onlineTimeDelta
                : false;
        }

        function stopPeriodicCall(fn) {
            if (angular.isDefined(fn)) {
                $interval.cancel(fn);
            }
        }

        function startPeriodicCall(fn, period) {
            if(angular.isDefined(fn)) {
                fn();
                return $interval(fn, period)
            }
            return undefined;
        }

        // endregion

        // public methods
        function _getDevice() {
            return angular.copy(device)
        }


        function _getDeviceStatus() {
            return ApiFactory
                .getDeviceStatus()
                .then(function (response) {
                    device.status = response.data || {};
                    device.online = isOnline(device.status.payload);

                    $rootScope.$emit('deviceStatusDataReceived', device.status);
                })
            ;
        }

        function _getDeviceRegularData() {
            return ApiFactory
                .getDeviceRegularData()
                .then(function (response) {
                    device.regular = response.data || {};
                    $rootScope.$emit('deviceSensorDataReceived', device.regular);
                });
        }

        function _getDeviceConfig(){
            return ApiFactory
                .getDeviceConfig()
                .then(function(config){
                    if (device) {
                        device.config = config.data || {}
                    }
                });
        }

        function _setDeviceConfig(config) {
            return ApiFactory
                .setDeviceConfig(config);
        }

        function _isDeviceOnline() {
            return device.online
        }

        function _startSurveyStatus(){
            periodicGetState = startPeriodicCall(_getDeviceStatus, 1 * 1000)
        }
        function _startSurveyRegularData(){
            periodicGetRegularData = startPeriodicCall(_getDeviceRegularData, 1 * 1000)
        }
        function _stopSurveyData(){
            stopPeriodicCall(periodicGetState);
            stopPeriodicCall(periodicGetRegularData);
        }
        // endregion

        return {
            getDevice                   :  _getDevice,
            getDeviceStatus             :  _getDeviceStatus,
            getDeviceRegularData        :  _getDeviceRegularData,
            getDeviceConfig             :  _getDeviceConfig,
            setDeviceConfig             :  _setDeviceConfig,
            isDeviceOnline              :  _isDeviceOnline,
            startSurveyStatus           :  _startSurveyStatus,
            startSurveyRegularData      :  _startSurveyRegularData,
            stopSurveyData              :  _stopSurveyData
        }
    }
})(angular);