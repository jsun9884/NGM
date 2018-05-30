'use strict';

// TODO Refactor this whole file!

angular
    .module('NgmAdmin', [
        'ngAnimate',
        'ngCookies',
        'ngMessages',
        'ngResource',
        'ngRoute',
        'ngSanitize',
        'ngMaterial',
        'ui.router',
        'sasrio.angular-material-sidenav'
    ])

    .config(['$stateProvider', '$urlRouterProvider', '$mdThemingProvider', 'ssSideNavSectionsProvider','$mdIconProvider', '$httpProvider', '$compileProvider',
        function($stateProvider, $urlRouterProvider, $mdThemingProvider, ssSideNavSectionsProvider, $mdIconProvider, $httpProvider, $compileProvider) {
            $httpProvider.interceptors.push('AuthInterceptorFactory');

            // TODO In 1.6.1 pre assigned binding are deprecated. change controllers to use onInit to use bindings (https://docs.angularjs.org/guide/migration#commit-bcd0d4)
            $compileProvider.preAssignBindingsEnabled(true);

            // region Theme settings
            $mdThemingProvider
                .definePalette('f1', {
                    '50': '#ececee',
                    '100': '#c5c6cb',
                    '200': '#9ea1a9',
                    '300': '#7d818c',
                    '400': '#5c616f',
                    '500': '#3c4252',
                    '600': '#353a48',
                    '700': '#2d323e',
                    '800': '#262933',
                    '900': '#1e2129',
                    'A100': '#c5c6cb',
                    'A200': '#9ea1a9',
                    'A400': '#5c616f',
                    'A700': '#2d323e',
                    'B100': '#ffffff',
                    'contrastDefaultColor': 'light',
                    'contrastDarkColors': '50 100 200 A100',
                    'contrastStrongLightColors': '300 400'
            });

            $mdThemingProvider
                .theme('default')
                .primaryPalette('f1',{
                    'default': '700',
                    'hue-1': '500',
                    'hue-2': '600',
                    'hue-3': 'A100'
                })
                .accentPalette('light-blue',{
                    'default': '600',
                    'hue-1'  : '400',
                    'hue-2'  : '600',
                    'hue-3'  : 'A700'
                })
                .warnPalette('red')
                .backgroundPalette('grey',{
                    'default': 'A100',
                    'hue-1'  : '100',
                    'hue-2'  : '50',
                    'hue-3'  : '300'
                });

            $mdIconProvider
                .icon('md-toggle-arrow', 'images/icons/toggle-arrow.svg', 48);
            // endregion

            // region State routes
            $urlRouterProvider
                .otherwise('/device/general');

            $stateProvider
                .state('site', {
                    'abstract': true
                })
                .state('signin', {
                    parent: 'site',
                    url: '/signin',
                    data: {
                        roles: []
                    },
                    views: {
                        'content@': {
                            templateUrl: 'views/signin.html',
                            controller: 'SignInController',
                            controllerAs : 'signInCtrl'
                        }
                    }
                })
                .state('restricted', {
                    parent: 'site',
                    url: '/restricted',
                    data: {
                        roles: ['Admin']
                    },
                    views: {
                        'content@': {
                            templateUrl: 'restricted.html'
                        }
                    }
                })
                .state('accessdenied', {
                    parent: 'site',
                    url: '/denied',
                    data: {
                        roles: []
                    },
                    views: {
                        'content@': {
                            templateUrl: 'denied.html'
                        }
                    }
                })
                .state('dashboard', {
                    parent: 'site',
                    title: 'Monitoring',
                    abstract: true,
                    resolve: {
                        authorize: ['AuthorizationFactory',
                            function(AuthorizationFactory) {
                                return AuthorizationFactory.isAuthenticated();
                            }
                        ]
                    },
                    data: {
                        roles: ['Admin','User']
                    },
                    views: {
                        'content@': {
                            templateUrl: 'views/main.html',
                            controller: 'MainController'
                        }
                    }
                })
                .state('dashboard.device',{
                    abstract: true,
                    title: 'Device',
                    url: '/device',
                    controller: 'DevicePageController',
                    controllerAs : 'vm',
                    templateUrl: 'views/pages/device.html'
                })
                .state('dashboard.device.general',{
                    url: '/general',
                    title: 'General',
                    views: {
                        deviceGeneral: {
                            templateUrl: 'views/partials/device-general-tmpl.html',
                            controller: 'DeviceGeneralController',
                            controllerAs : 'generalCtrl'
                        }
                    }
                })
                .state('dashboard.device.power',{
                    url: '/power',
                    title: 'Power data',
                    views: {
                        devicePower: {
                            templateUrl: 'views/partials/device-power-tmpl.html',
                            controller: 'DevicePowerController',
                            controllerAs : 'powerCtrl'
                        }
                    }
                })
                .state('dashboard.device.seismic',{
                    url: '/seismic',
                    title: 'Seismic',
                    views: {
                        deviceSeismic: {
                            templateUrl: 'views/partials/device-seismic-tmpl.html',
                            controller: 'DeviceSeismicController',
                            controllerAs : 'seismicCtrl'
                        }
                    }
                })
                .state('dashboard.device.config',{
                    url: '/config',
                    title: 'Configuration',
                    views: {
                        deviceConfig: {
                            templateUrl: 'views/partials/device-config-tmpl.html',
                            controller: 'DeviceConfigController',
                            controllerAs : 'configCtrl'
                        }
                    }
                })
                .state('dashboard.device.commands',{
                    url: '/commands',
                    title: 'Commands',
                    views: {
                        deviceCommands: {
                            templateUrl: 'views/partials/device-commands-tmpl.html',
                            controller: 'DeviceCommandController',
                            controllerAs : 'commandsCtrl'
                        }
                    }
                })
            ;

            // endregion

            // region Sidenav settings
            ssSideNavSectionsProvider
                .initWithTheme($mdThemingProvider);

            ssSideNavSectionsProvider
                .initWithSections([
                    {
                        id: 'MonitoringSection',
                        name: 'Monitoring',
                        type: 'heading',
                        children:
                            [
                                {
                                    name: 'Status',
                                    type: 'toggle',
                                    icon: 'apps',
                                    pages: [
                                        {
                                            id: 'deviceStatus',
                                            name: 'Device',
                                            state: 'dashboard.device.general',
                                            icon: 'devices'
                                        }
                                    ]
                                }
                            ]
                    }
                ])
            ;
            // endregion
        }
    ])  
    .run(['$rootScope', '$state', '$stateParams',
        function($rootScope, $state, $stateParams) {
            $rootScope.$on('$stateChangeStart', function(event, toState, toStateParams) {
                $rootScope.toState = toState;
                $rootScope.toStateParams = toStateParams;
            });

            $rootScope.$on('$stateChangeSuccess', function(event, toState, toStateParams) {
                $rootScope.current_state_path = $state.$current.path;
            });

            $rootScope.$on('$stateChangeError', function(event, toState, toStateParams, fromState, fromParams, error) {
                if (typeof error !== 'undefined' && typeof error.redirectTo !== 'undefined') {
                    $state.go(error.redirectTo);
                } else {
                    //$state.go('restricted', {status: error.status})
                    $log.error('Restricted:', error.message || '')
                }
            });
        }
    ]);
