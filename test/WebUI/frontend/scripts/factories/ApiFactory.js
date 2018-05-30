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
        function _getDeviceStatus() {
            return $http.get('/api/device/status');
        }

        function _getDeviceRegularData() {
            return $http.get('/api/device/regular');
        }

        function _getDeviceConfig() {
            return $http.get('/api/device/config');
        }

        function _setDeviceConfig(config) {
            return $http.post('/api/device/config', config)
        }

        function _postDeviceCommand(command) {
            return $http.post('/api/device/command', command)
        }

        function _getCommandExecState(commandId) {
            return $http.get('/api/devices/command/?commandId=' + commandId);
        }

        function _uploadFile(formData){
            return $http({
                url: '/api/files/upload',
                method: "POST", //update
                processData: false,
                headers: {
                    'Content-Type': undefined
                },
                transformRequest: angular.identity,
                data: formData
            })
            .then(function(result){
                // catch 403 forbidden (CORS problem)
                if(result.status === -1 || result.status === 403 || !result.status){
                    return $q.reject();
                }
                return $q.resolve();
            });

/*
            return $http({
                url: '/api/files/upload',
                method: "POST", //update
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

*/

/*
            return $http({
                url: '/api/files/upload',
                method: "PUT", //update
                dataType: 'json',
                data: formData,
                processData: false,
                headers: {
                    'Content-Type': 'binary/octet-stream'
                }
            }, formData, {
                transformRequest: angular.identity,
                headers: {'Content-Type': 'binary/octet-stream'}
            });
*/
        }
        // endregion

        // region Authenticate API
        function _postAuthenticate(creds) {
            return $http.post('/api/authenticate', creds);
        }
        // endregion

        return {
            getDeviceStatus         : _getDeviceStatus,
            getDeviceRegularData    : _getDeviceRegularData,
            getDeviceConfig         : _getDeviceConfig,
            setDeviceConfig         : _setDeviceConfig,
            postDeviceCommand       : _postDeviceCommand,
            getCommandExecState     : _getCommandExecState,
            postAuthenticate        : _postAuthenticate,

            uploadFile              : _uploadFile
        }
    }
})(angular);