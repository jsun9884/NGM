'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .directive('fileInput',[
            '$q',
            'ApiFactory',
            fileInput
        ]);

    function fileInput($q, ApiFactory){
        return {
            restrict: 'A',
            scope: {
                control: '='
            },
            link: function($scope, elm, attrs){
                var fileData = [];
                var fileMeta = {};
                // endregion

                // region Published methods
                $scope.control.uploadFile = function(){
                    fileData = elm[0].files[0];

                    return $q(function(resolve, reject){
                        if(!!fileData){
                            var fd = new FormData();
                            fd.append('file', fileData);

                            // trying to upload files
                            return ApiFactory
                                .uploadFile(fd)
                                .then(function(result){
                                    resolve()
                                })
                                .catch(function(result){
                                    reject();
                                })
                            ;
                        }
                        else{
                            reject();
                        }
                    })
                };

                $scope.control.getFileData = function(){
                    return fileData;
                };

                $scope.control.isDefined = function(){
                    return elm[0].files.length > 0;
                };


                // endregion

                elm.bind('change',function(event){
                    elm.blur();
                });
            }
        };
    }
})(angular);