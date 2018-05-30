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

                // region Private methods
                function getUploadUrl(filename, repository){
                    return ApiFactory
                        .requestUploadLink({
                            'filename': filename,
                            'repository': repository,
                            'operation': 'UPLOAD'
                        })
                        .then(function(result){
                            return result.data.url;
                        })
                        .catch(function(){
                            console.log('Can\'t get upload url');
                        })
                    ;
                }

                function assignUrl(file, repository, name){
                    fileData = file[0];
                    fileMeta = {};

                    name = !!name ? name : fileData.name;
                    return getUploadUrl(name, repository)
                        .then(function(url){
                            fileMeta = {
                                'name': name,
                                'url': url
                            };
                        })
                    ;
                }
                // endregion

                // region Published methods
                $scope.control.prepareFile = function(name){
                    return assignUrl(elm[0].files, elm.attr('repository'), name);
                };

                $scope.control.uploadFile = function(){
                    return $q(function(resolve, reject){
                        // if no files selected
/*
                        if(!fileData || !fileData.length){
                            return reject();
                        }
*/
                        if(!!fileData){

                            //var fd = new FormData();
                            //fd.append('file', fileData);

                            // trying to upload files
                            ApiFactory
                                .uploadFile(fileMeta.url, fileData)//fd)
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
                    //    return fileData;
                    return fileMeta;
                };

                $scope.control.isDefined = function(){
                    return elm[0].files.length > 0;
                };
                // endregion

                elm.bind('change',function(event){
                    //assignUrl(elm[0].files, elm.attr('repository'));
                    elm.blur();
                });

                elm.bind('focus',function(event){
                    //elm.blur();
                });
            }
        };
    }
})(angular);