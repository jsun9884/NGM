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

        // region Init
        var signInCtrl = this;
        $rootScope.dataLoaded = true;

        $scope.identity = {
            email: null,
            password: null
        };

        AuthorizationFactory.UnAuthenticate();
        // endregion

        // region UI events
        signInCtrl.signin = function() {
            $scope.invalid_identity = false;

            // here, we fake authenticating and give a fake user 
            AuthorizationFactory
                .login({
                    username: $scope.identity.email,
                    password: $scope.identity.password
                })
                .then(function(){
                    $state.go('dashboard.overview');
                })
                .catch(function(response){
                    $scope.invalid_identity = true;
                })
            ;
        };
        // endregion
    }
})();