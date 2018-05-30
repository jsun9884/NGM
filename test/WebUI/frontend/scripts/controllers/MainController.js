'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('MainController', [
            '$scope',
            '$rootScope',
            '$state',
            'ssSideNav',
            'AuthorizationFactory',
            MainController
        ]);

    function MainController($scope, $rootScope, $state, ssSideNav, AuthorizationFactory) {
        // TODO Refactor this controller, remove unused dependencies
        $scope.$state = $state;

        $scope.menu = ssSideNav;

        $scope.accountBoxExpanded = false;
        $rootScope.userProfile = JSON.parse(localStorage.getItem('profile'));

        $rootScope.shouldLockOpen = false;
        $rootScope.dataLoaded = true;

        $scope.toggleAccountBoxExpansion = function(){
            $scope.accountBoxExpanded = !$scope.accountBoxExpanded;
        };

        $scope.signout = function() {
            AuthorizationFactory.UnAuthenticate();
            $state.go('signin');
        };


    }
}());