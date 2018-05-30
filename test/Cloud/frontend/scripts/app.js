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
        'nvd3',
        'sasrio.angular-material-sidenav',
        'md.data.table',
        'ngMaterialDatePicker',
        'uiGmapgoogle-maps',
        'angularMoment',
        'chart.js',
        'ngFileSaver',
        'angular-ranger'
    ])
    .config([
        '$stateProvider',
        '$urlRouterProvider',
        '$mdThemingProvider',
        'ssSideNavSectionsProvider',
        '$mdIconProvider',
        'uiGmapGoogleMapApiProvider',
        '$httpProvider',
        '$compileProvider',
        'GOOGLE_MAP',
        function(
            $stateProvider,
            $urlRouterProvider,
            $mdThemingProvider,
            ssSideNavSectionsProvider,
            $mdIconProvider,
            uiGmapGoogleMapApiProvider,
            $httpProvider,
            $compileProvider,
            GOOGLE_MAP
        ) {
            $httpProvider.interceptors.push('AuthInterceptorFactory');

            // TODO In 1.6.1 pre assigned binding are deprecated. change controllers to use onInit to use bindings (https://docs.angularjs.org/guide/migration#commit-bcd0d4)
            $compileProvider.preAssignBindingsEnabled(true);

            uiGmapGoogleMapApiProvider.configure(GOOGLE_MAP.config);

            // region Theme Palettes
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
                    'A300': '#727272',
                    'A400': '#5c616f',
                    'A700': '#2d323e',
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

            $mdThemingProvider
                .definePalette('f2', {
                    '50': '#ffffff',
                    '100': '#ffeeee',
                    '200': '#eeeeee',
                    '300': '#eeeeee',
                    '400': '#eeeeee',
                    '500': '#eeeeee',
                    '600': '#696969',
                    '700': '#eeeeee',
                    '800': '#eeeeee',
                    '900': '#000000',
                    'A100': '#f5f5f5',
                    'A200': '#d8d8d8',
                    'A400': '#dddddd',
                    'A700': '#eeeeee'
                })
            ;

                // endregion

            $mdIconProvider
                .icon('md-toggle-arrow', 'images/icons/toggle-arrow.svg', 48);

            //$urlRouterProvider
            //    .otherwise('/overview');
            $urlRouterProvider.otherwise(function ($injector, $location) {
                var $state = $injector.get('$state');

                $state.go('dashboard.overview', {
                    title: "Page not found",
                    message: 'Could not find a state associated with url "'+$location.$$url+'"'
                });
            });
            $urlRouterProvider.when('/devices', '/devices/general');

            // region State routes
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
                .state('dashboard.overview',{
                    url: '/overview',
                    title: 'Overview',
                    templateUrl: 'views/pages/overview.html',
                    controller: 'OverviewController',
                    controllerAs : 'vm'
                })
                .state('dashboard.devices',{
                    //abstract: true,
                    title: 'NGM devices',
                    url: '/devices',
                    controller: 'DevicesController',
                    controllerAs : 'vm',
                    templateUrl: 'views/pages/devices.html',
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.devices.power',{
                    url: '/power',
                    title: 'Power data',
                    views: {
                        devicePower: {
                            templateUrl: 'views/partials/device-power-tmpl.html',
                            controller: 'DevicePowerController',
                            controllerAs : 'devPowerCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.devices.seismic',{
                    url: '/seismic',
                    title: 'Seismic data',
                    views: {
                        deviceSeismic: {
                            templateUrl: 'views/partials/device-seismic-tmpl.html',
                            controller: 'DeviceSeismicController',
                            controllerAs : 'devSeismicCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.devices.general',{
                    url: '/general',
                    title: 'General',
                    views: {
                        deviceStatus: {
                            templateUrl: 'views/partials/device-general-tmpl.html',
                            controller: 'DeviceGeneralController',
                            controllerAs : 'generalCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.devices.config',{
                    url: '/config',
                    title: 'Configuration',
                    views: {
                        deviceConfig: {
                            templateUrl: 'views/partials/device-config-tmpl.html',
                            controller: 'DeviceConfigController',
                            controllerAs : 'configCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.devices.commands',{
                    url: '/commands',
                    title: 'Commands',
                    views: {
                        deviceCommands: {
                            templateUrl: 'views/partials/device-commands-tmpl.html',
                            controller: 'DeviceCommandController',
                            controllerAs : 'commandsCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                // .state('dashboard.devices.zigbee',{
                //     url: '/zigbee',
                //     title: 'ZigBee',
                //     views: {
                //         deviceZigBee: {
                //             templateUrl: 'views/partials/device-zigbee-tmpl.html',
                //             controller: 'DeviceZigBeeController',
                //             controllerAs : 'zigbeeCtrl'
                //         }
                //     },
                //     rightSideNav: 'device-list-side-nav'
                // })
                .state('dashboard.devices.dataswitch',{
                    url: '/dataswitch',
                    title: 'Data switch',
                    views: {
                        deviceDataSwitch: {
                            templateUrl: 'views/partials/device-dataswitch-tmpl.html',
                            controller: 'DeviceDataSwitchController',
                            controllerAs : 'dataswitchCtrl'
                        }
                    },
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.events',{
                    url: '/events',
                    title: 'Events',
                    templateUrl: 'views/pages/events.html',
                    controller: 'EventsController',
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.export',{
                    url: '/data-export',
                    title: 'Data export',
                    templateUrl: 'views/pages/data-export.html',
                    controller: 'DataExportController',
                    rightSideNav: 'device-list-side-nav'
                })
                .state('dashboard.logs',{
                    url: '/logs',
                    title: 'Logs',
                    templateUrl: 'views/pages/logs.html',
                    controller: 'LogsController',
                    rightSideNav: 'device-list-side-nav'
                });

            // endregion

            ssSideNavSectionsProvider
                .initWithTheme($mdThemingProvider);

            ssSideNavSectionsProvider
                .initWithSections(
                [
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
                                            id: 'Overview',
                                            name: 'Overview',
                                            state: 'dashboard.overview',
                                            icon: 'widgets'
                                        },
                                        {
                                            id: 'devicesPower',
                                            name: 'NGM devices',
                                            state: 'dashboard.devices',
                                            icon: 'devices'
                                        }
                                    ]
                                },
                                {
                                    name: 'History',
                                    type: 'toggle',
                                    icon: 'history',
                                    pages: [
                                        {
                                            id: 'Events',
                                            name: 'Events',
                                            state: 'dashboard.events',
                                            icon: 'event_note'
                                        },
                                        {
                                            id: 'dataExport',
                                            name: 'Data export',
                                            state: 'dashboard.export',
                                            icon: 'cloud_download'
                                        },
                                        //{
                                        //    id: 'Logs',
                                        //    name: 'Logs',
                                        //    state: 'dashboard.logs',
                                        //    icon: 'toc'
                                        //},
                                    ]
                                }
                            ]
                    },
                ]);
        }
    ])
    .run(['$rootScope', '$state', '$stateParams', 'AuthorizationFactory', '$window',
        function($rootScope, $state, $stateParams, AuthorizationFactory, $window) {
            $rootScope.$on('$stateChangeStart', function(event, toState, toStateParams) {
                $rootScope.toState = toState;
                $rootScope.toStateParams = toStateParams;
            });

            $rootScope.$on('$stateChangeSuccess', function(event, toState, toStateParams) {
                $rootScope.current_state_path = $state.$current.path;
                $rootScope.current_state_title = !!$state.$current.path[2] ? $state.$current.path[2].title : '';
            });

            $rootScope.$on('$stateChangeError', function(event, toState, toStateParams, fromState, fromParams, error) {
                if (typeof error !== 'undefined' && typeof error.redirectTo !== 'undefined') {
                    $state.go(error.redirectTo);
                } else {
                    //$state.go('restricted', {status: error.status})
                    console.log('restricted');
                }
            });

            angular.element($window).bind('resize', function(){
                $rootScope.$emit('$windowResized');
            });

        }
    ])
;
