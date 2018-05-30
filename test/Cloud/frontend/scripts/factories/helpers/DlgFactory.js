'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('DlgFactory',[
            '$mdDialog',
            '$log',
            DlgFactory
        ]);

    function DlgFactory($mdDialog, $log) {
        function DialogController(){
            const dlgCtrl = this;

            dlgCtrl.ok = function() {
                $mdDialog.hide(false);
            };
            dlgCtrl.cancel = function() {
                $mdDialog.hide(false);
            };
        }

        function _showMessage(caption, content){
            $mdDialog
                .show({
                    controller              : DialogController,
                    controllerAs            : 'dlgCtrl',
                    bindToController        : true,
                    templateUrl             : '/views/partials/message-dialog.tmpl.html',
                    parent                  : angular.element(document.body),
                    clickOutsideToClose     : true,
                    autoWrap: false,
                    skipHide: true,
                    locals: {
                        dlgCaption: caption,
                        dlgContent: content
                    }
                })
                .then(function(config) {
                    return config;
                })
                .catch($log.error)
            ;
        }

        return {
            showMessage         : _showMessage
        }
    }
})(angular);