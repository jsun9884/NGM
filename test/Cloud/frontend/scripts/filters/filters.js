'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .filter('dashToUCFirst', function(){
            return function(input_dasherized) {
                return input_dasherized.toLowerCase().replace(/_/g, ' ');
            };
        })
        .filter('formatTime', function(){
            return function(input_time, format) {
                if(!isNaN(input_time) && input_time.toString().length == 10){
                    return moment.utc(input_time * 1000).format(format || 'HH:mm:ss MM/DD/YYYY');
                }
                if(!isNaN(input_time) && input_time.toString().length == 13){
                    return moment.utc(input_time).format(format || 'HH:mm:ss MM/DD/YYYY');
                }
                return input_time;
            };
        })
        .filter('itemPropByProp', function(){
            return function(arr, findPropName, findPropValue, targetPropName) {
                var item = arr.find(function(item){
                    return item[findPropName] == findPropValue;
                });
                return !!item ? item[targetPropName] : '';
            };
        })
        .filter('isObject',function(){
            return function(input){
                return angular.isObject(input);
            }
        })
        .filter('toFixed',function(){
            return function(input, fractionalDigits){
                return isNaN(input) ? input : parseFloat(input).toFixed(!!fractionalDigits ? fractionalDigits : 3);
            }
        })
        .filter('byPhase',function(){
            return function(input){
                return input.substring(0, input.length - 1).replace(/_/g , " ") + ' Phase ' + String.fromCharCode(64 + parseInt(input.charAt(input.length-1)));
            }
        })

})(angular);