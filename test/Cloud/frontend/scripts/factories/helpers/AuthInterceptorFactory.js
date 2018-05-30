'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('AuthInterceptorFactory',[
            '$window',
            '$q',
            '$location',
            AuthInterceptorFactory
        ]);

    function AuthInterceptorFactory($window, $q, $location){
        return {
            request: function(config) {
                if(config.url.indexOf('http') !== -1){
                    return config;
                }

                config.headers = config.headers || {};
                if (localStorage.getItem('token')) {
                    config.headers.Authorization = 'Bearer ' + localStorage.getItem('token');
                }
                return config || $q.when(config);
            },
            response: function(response) {
                if (response.status === 401) {
                    $location.path('/signin');
                }
                return response || $q.when(response);
            },
            responseError: function(error){
                if (error.status === 401) {
                    $location.path('/signin');
                }

                return error || $q.when(error);
            }
        };
    }
})(angular);