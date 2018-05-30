'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('ApiFactory',[
            '$q',
            '$http',
            ApiFactory
        ]);

    function ApiFactory($q, $http){

        // region devices and commands API
        function _getDevices() {
            return $http.get('/api/devices');
        }

        function _getDevicesStatus() {
            return $http.get('/api/devices/status');
        }

        function _getDevicesLocation() {
            return $http.get('/api/devices/location');
        }

        function _getDevicesPresence() {
            return $http.get('/api/devices/presence');
        }

        function _getDevicePresenceById(deviceId) {
            return $http.get('/api/devices/' + deviceId + 'presence');
        }

        function _getDeviceStatusById(deviceId) {
            return $http.get('/api/devices/' + deviceId + '/status');
        }
        
        function _getDeviceRegularDataById(deviceId) {
            return $http.get('/api/devices/' + deviceId + '/regular');
        }

        function _getDeviceConfigById(deviceId) {
            return $http.get('/api/devices/' + deviceId + '/config');
        }

        function _setDeviceConfigById(deviceId, config) {
            return $http.post('/api/devices/' + deviceId + '/config', config)
        }

        function _postDeviceCommand(deviceId, command) {
            return $http.post('/api/devices/' + deviceId + '/command', command)
        }

        function _getCommandExecState(deviceId, commandId) {
            return $http.get('/api/devices/' + deviceId + '/command/?commandId=' + commandId);
        }
        // endregion

        function _requestDownloadLink(opts){
            return $http.post('/api/files/url', opts);
        }
        function _requestUploadLink(opts){
            return $http.post('/api/files/url', opts);
        }
        function _uploadFile(url, formData){
            return $http({
                url: url,
                method: "PUT", //update
                processData: false,
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded'
                },
                data: formData
            })
            .then(function(result){
                // catch 403 forbidden (CORS problem)
                if(result.status === -1 || result.status === 403){
                    return $q.reject();
                }
                return $q.resolve();
            });
        }

        // region History
        function _getEventsRecent() {
            return $http.get('/api/history/events/recent')
        }

        function _getEventsHistoryById(deviceId, params) {
            return $http.get('/api/history/events/' + deviceId, {params: params})
        }

        function _getCommandsHistoryById(deviceId) {
            return $http.get('/api/history/commands/' + deviceId)
        }

        function _exportEventsById(deviceId, params) {
            return $http.get('/api/history/exports/events/' + deviceId, {params: params})
        }

        function _exportRegularDataById(deviceId, params) {
            return $http.get('/api/history/exports/regular/' + deviceId, {params: params})
        }
        // endregion

        // region Authenticate API
        function _postLogin(creds) {
            return $http.post('/api/login', creds);
        }
        // endregion

        return {
            getDevices                  : _getDevices,
            getDevicesStatus            : _getDevicesStatus,
            getDevicesLocation          : _getDevicesLocation,
            getDevicesPresence          : _getDevicesPresence,
            getDevicePresenceById       : _getDevicePresenceById,

            getDeviceStatusById         : _getDeviceStatusById,
            getDeviceRegularDataById    : _getDeviceRegularDataById,
            getDeviceConfigById         : _getDeviceConfigById,
            setDeviceConfigById         : _setDeviceConfigById,
            postDeviceCommand           : _postDeviceCommand,
            getCommandExecState         : _getCommandExecState,

            postLogin                   : _postLogin,

            requestDownloadLink         : _requestDownloadLink,
            requestUploadLink           : _requestUploadLink,
            uploadFile                  : _uploadFile,

            getEventsRecent             : _getEventsRecent,

            getEventsHistoryById        : _getEventsHistoryById,
            getCommandsHistoryById      : _getCommandsHistoryById,

            exportEventsById            : _exportEventsById,
            exportRegularDataById       : _exportRegularDataById
        }
    }
})(angular);