'use strict';

(function() {
    angular
        .module('NgmAdmin')
        .controller('DeviceSeismicController',[
            '$scope',			
            'ActiveDeviceFactory',
            'DeviceFactory',
            'SEISMIC_COMMANDS_CARDS',
            DeviceSeismicController			
        ]);

    function DeviceSeismicController($scope, ActiveDeviceFactory, DeviceFactory, SEISMIC_COMMANDS_CARDS) {
        //region Init
        var devSeismicCtrl = this;
		
		devSeismicCtrl.sendingConfig = {
			AccAutoHr:false,
			AccEvents:false
		}
        devSeismicCtrl.sensors = ['AccCore', 'AccPCB'];
        devSeismicCtrl.commandsCards = SEISMIC_COMMANDS_CARDS;
		//devSeismicCtrl.activeDevice.config.General.AccAutoHrEnabled
		devSeismicCtrl.AccAutoHrEnabled = false;
		devSeismicCtrl.AccEventsEnabled = false;
		
        devSeismicCtrl.sensors = [];

		
		
        function init() {
            devSeismicCtrl.activeDevice = ActiveDeviceFactory.getActiveDevice();			
        }

        async function postDeviceConfig(newConfig) {
        	return new Promise(function(resolve,reject){
        		var config = {
					payload: {
						state: {
							desired: newConfig
						}
					}
				};			
				resolve(DeviceFactory.setDeviceConfigById(devSeismicCtrl.activeDevice.thingName, config));
			});			
         }

        function refresh()
        {		
			console.log('...............refresh',!devSeismicCtrl.sendingConfig.AccAutoHr,devSeismicCtrl.sendingConfig.AccEvents);
			
			
										
				devSeismicCtrl.widgets = [];
				devSeismicCtrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
			
				if(!!devSeismicCtrl.activeDevice) {
					
					ActiveDeviceFactory.refreshActiveDeviceConfig().then(function(){
						if(!devSeismicCtrl.sendingConfig.AccAutoHr){	
							devSeismicCtrl.AccAutoHrEnabled = devSeismicCtrl.activeDevice.config.General.AccAutoHrEnabled;
						}
						if(!devSeismicCtrl.sendingConfig.AccEvents){	
							devSeismicCtrl.AccEventsEnabled = devSeismicCtrl.activeDevice.config.General.AccEventsEnabled;		
						}
					});							
				}
						
        }

		$scope.toggleAccAutoHr = function(){				
			if(devSeismicCtrl.activeDevice.config) {
				devSeismicCtrl.sendingConfig.AccAutoHr = true;				
				
                var newConfig = {
                    General: {
                        AccAutoHrEnabled: devSeismicCtrl.AccAutoHrEnabled//.activeDevice.config.General.AccAutoHrEnabled
                    }
                }				
				//console.log('...before sending toggleAccAutoHr',devSeismicCtrl.AccAutoHrEnabled, new Date());
                postDeviceConfig(newConfig).then(function(){
                	setTimeout(function(){
						devSeismicCtrl.sendingConfig.AccAutoHr = false;
						//console.log('...after sending toggleAccAutoHr',devSeismicCtrl.AccAutoHrEnabled, new Date());
					},5000);
                });				
            }
		}
		
		$scope.toggleAccEvents = function(){			
			if(devSeismicCtrl.activeDevice.config) {
				devSeismicCtrl.sendingConfig.AccEvents = true;
				
                var newConfig = {
                    General: {
                        AccEventsEnabled: devSeismicCtrl.AccEventsEnabled//.activeDevice.config.General.AccEventsEnabled
                    }
                }				
				//console.log('...before sending AccEventsEnabled',devSeismicCtrl.AccEventsEnabled, new Date());
                postDeviceConfig(newConfig).then(function(){
                	setTimeout(function(){
						devSeismicCtrl.sendingConfig.AccEvents = false;
						//console.log('...after sending AccEventsEnabled',devSeismicCtrl.AccEventsEnabled, new Date());
					},5000);					
                });				
            }
		}
		 
        init();
        //endregion

        //region Event Handlers
		$scope.$on('$onSetDeviceConfigById',function(event, config){});
		
        $scope.$on('$activeDeviceChanged', function(event, activeDevice){			
            devSeismicCtrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
            refresh();
        });
		
        /*$scope.$watch('devSeismicCtrl.activeDevice.config.General.AccAutoHrEnabled', function() {
            if(devSeismicCtrl.canUpdateConfiguration && devSeismicCtrl.activeDevice.config) {
                var newConfig = {
                    General: {
                        AccAutoHrEnabled: devSeismicCtrl.activeDevice.config.General.AccAutoHrEnabled
                    }
                }
				console.log('[r][devSeismicCtrl] newConfig',newConfig);
                postDeviceConfig(newConfig);
            }
        },true);*/

        /*$scope.$watch('devSeismicCtrl.activeDevice.config.General.AccEventsEnabled', function() {
            if(devSeismicCtrl.canUpdateConfiguration && devSeismicCtrl.activeDevice.config) {
                var newConfig = {
                    General: {
                        AccEventsEnabled: devSeismicCtrl.activeDevice.config.General.AccEventsEnabled
                    }
                }
                postDeviceConfig(newConfig);

            }
        }, true);*/

        $scope.$on('devicesPresenceRefreshed', function(){
            devSeismicCtrl.activeDevice = ActiveDeviceFactory.getActiveDevice();
            refresh();
        });
        //endregion

    }
})();