'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('SignInController',[
            '$scope',
            '$rootScope',
            '$state',
            'AuthorizationFactory',
            SignInController
        ]);

    function SignInController($scope, $rootScope, $state, AuthorizationFactory){
        const signInCtrl = this;
        $rootScope.dataLoaded = true;

        $scope.identity = {
            email: null,
            password: null
        };

        signInCtrl.signin = function() {
            $scope.invalid_identity = false;

            AuthorizationFactory
                .authorize({
                    username: $scope.identity.email,
                    password: $scope.identity.password
                })
                .then(function(){
                    $state.go('dashboard.device.general');
                })
                .catch(function(response){
                    $scope.invalid_identity = true;
                });
        }
    }
})();