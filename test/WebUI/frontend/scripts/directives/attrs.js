'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .directive('dynamicAttrs',[
            '$compile',
            dynamicAttrs
        ]);

    function dynamicAttrs($compile){
        return {
            restrict: 'A',
            replace: false,
            terminal: true,
            priority: 1000,

            link: function(scope, element, attributes){
                element.removeAttr("dynamic-attrs");

                var attrs = angular.copy(scope.param.attributes);
                element.attr(attrs);
                $compile(element)(scope);
            }
        };
    }
})(angular);