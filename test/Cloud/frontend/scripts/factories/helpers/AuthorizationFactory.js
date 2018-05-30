'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('AuthorizationFactory',[
            '$q',
            '$rootScope',
            'ApiFactory',
            AuthorizationFactory
        ]);

    function AuthorizationFactory($q, $rootScope, ApiFactory){
        function _isAuthenticated() {
            var deferred = $q.defer();

            if (!!localStorage.getItem('token')) {
                deferred.resolve();
            }
            else{
                deferred.reject({redirectTo: 'signin'});
            }
            return deferred.promise;
        }

        function _UnAuthenticate(){
            localStorage.removeItem('token');
            localStorage.removeItem('profile');

            $rootScope.$broadcast('logout');
        }

        function _login(identity) {
            return ApiFactory
                .postLogin(identity)
                .then(function(response){
                    if(typeof response.data !== 'undefined' && typeof response.data.token !== 'undefined'){
                        localStorage.setItem('token', response.data.token);
                        localStorage.setItem('profile', JSON.stringify(response.data.profile));
                        return response;
                    }
                    else{
                        return $q.reject(response);
                    }
                })
                .catch(function(response){
                    localStorage.removeItem('token');
                    localStorage.removeItem('profile');
                    return $q.reject(response);
                })
            ;
        }

        return {
            login           :  _login,
            isAuthenticated :  _isAuthenticated,
            UnAuthenticate  :  _UnAuthenticate
        };
    }
})(angular);