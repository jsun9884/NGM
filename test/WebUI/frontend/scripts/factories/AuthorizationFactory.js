'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('AuthorizationFactory',[
            'ApiFactory',
            '$log',
            '$q',
            AuthorizationFactory
        ]);

    function AuthorizationFactory(ApiFactory, $log, $q){
        $log.info("AuthorizationFactory loaded");

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
        }


        function _authorize(identity) {
            return ApiFactory
                .postAuthenticate(identity)
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
            authorize       :  _authorize,
            isAuthenticated :  _isAuthenticated,
            UnAuthenticate  :  _UnAuthenticate
        };
    }
})(angular);