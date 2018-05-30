angular.module('NgmAdmin')
    .constant('CMD_STATES', {
        IDLE        : "IDLE",
        POSTED      : "POSTED",
        PENDING     : "PENDING",
        DONE        : "DONE",
        REJECTED    : "REJECTED",
        TIMEOUT     : "TIMEOUT",
        ACCEPTED    : "ACCEPTED",
        PROGRESS    : "PROGRESS",
        UPLOADING   : "UPLOADING"
    })
    .constant('COMMANDS_LIST', [
        {
            displayName : 'Upgrade',
            commandCode   : 100,
            description : 'Perform firmware upgrade.',
            hasParams : true,
            roles: ['ADMIN'],
            params : [
                {
                    name: 'filename',
                    caption: 'Firmware file',
                    required: true,
                    filename: '',
                    attributes: {
                        'type' : 'file',
                        'repository': 'FIRMWARE',
                        'file-input' : "files",
                        'control': "$ctrl.fileControl"
                    },
                },
            ]
        },
        {
            displayName : 'Reset counters',
            commandCode   : 130,
            description : 'Reset counters.',
            hasParams  : true,
            roles: ['ADMIN', 'TECH'],
            params : [
                {
                    name: 'sensor',
                    caption: 'Sensor',
                    value: '',
                    options: {
                        '': 'All',
                        'DataSwitch': 'Data switch',
                        'PowerMeter': 'TI'
                    }
                },
            ]
        },
        {
            displayName : 'SOM Reset',
            commandCode   : 120,
            description : 'Perform software reset of SOM.',
            hasParams : false,
            roles: ['ADMIN', 'TECH'],
            params : []
        },
        {
            displayName : 'Upload cfg',
            commandCode   : 310,
            description : 'Upload configurations file.',
            hasParams : true,
            roles: ['ADMIN'],
            params : [
                {
                    name: 'filename',
                    caption: 'Config file',
                    required: true,
                    filename: '',
                    attributes: {
                        'type' : 'file',
                        'repository': 'CONFIG_FILES',
                        'file-input' : "files",
                        'control': "$ctrl.fileControl"
                    },
                },
            ]
        },
        {
            displayName : 'Download cfg',
            commandCode   : 320,
            description : 'Download configurations file.',
            hasParams : false,
            roles: ['ADMIN'],
            params : []
        },
        {
            displayName : 'Read registers',
            commandCode   : 301,
            description : 'Download registers data.',
            hasParams : false,
            roles: ['ADMIN'],
            params : []
        },
        {
            displayName : 'Calibration',
            commandCode   : 302,
            description : 'Calibration',
            hasParams : false,
            roles: ['ADMIN'],
            params : []
        },
        {
            displayName     : 'Request HR Data',
            commandCode     : 300,
            description     : 'Get High Resolution data.',
            hasParams       : true,
            //chainCommands   : [330],
            roles           : ['ADMIN', 'TECH'],
            params          : [
                {
                    name: 'sensor',
                    caption: 'Meter',
                    value: 'PowerMeter',
                    options: {
                        'PowerMeter': 'Power meter',
                        'AccCore': 'Accelerometer (NGM&nbsp;internal)',
                        'AccPCB': 'Accelerometer (NGM&nbsp;enclosure)'
                    }
                },
            ]
        },
        {
            displayName : 'Prepare HR Data',
            commandCode : 330,
            description : 'Prepare High Resolution data to download.',
            roles: ['ADMIN', 'TECH'],
            hidden: true
        },
    ])
    .constant('COMMANDS_CARDS', [
        {
            title: 'Upgrade',
            description: 'Perform firmware upgrade.',
            cmdChains: [100],
        },
        {
            title: 'Reset counters',
            description: 'Reset counters.',
            cmdChains: [130]
        },
        {
            title: 'SOM Reset',
            description: 'Perform software reset of SOM.',
            cmdChains: [120]
        },
        {
            title: 'Upload config',
            description: 'Upload configurations file.',
            cmdChains: [310]
        },
        {
            title: 'Download config',
            description: 'Download configurations file.',
            cmdChains: [320]
        },
        {
            title: 'Request HR Data',
            description: 'Get High Resolution data.',
            cmdChains: [300, 330]
        },
        {
            title: 'Registers',
            description: 'Get registers data.',
            cmdChains: [301]
        },

    ])
    .constant('SEISMIC_COMMANDS_CARDS', [
        {
            title: 'Request HR Data',
            description: 'Get High Resolution data.',
            cmdChains: [300, 330],
            defaultParameter:'AccCore'
        },
        {
            title: 'Registers',
            description: 'Get registers data.',
            cmdChains: [301]
        },
        {
            title: 'Calibration',
            description: 'Calibration',
            cmdChains: [302]
        },
    ])

    .constant('SENSORS_CARDS', [
        {
            name: 'power',
            title: 'Power meter',
            sensors: [
                {
                    name: 'PowerMeter',
                    scheme: [
                        {
                            name: 'phase',
                            data: [
                                {
                                    name: 'voltage',
                                    title: 'Voltage',
                                    unit: 'V'
                                },
                                {
                                    name: 'current',
                                    title: 'Current',
                                    unit: 'A'
                                },
                                {
                                    name: 'active_power',
                                    title: 'Active Power',
                                    unit: 'W',
                                    total: true
                                },
                                {
                                    name: 'reactive_power',
                                    title: 'Reactive Power',
                                    unit: 'VAr',
                                    total: true
                                },
                                {
                                    name: 'apparent_power',
                                    title: 'Apparent Power',
                                    unit: 'VA',
                                    total: true
                                },
                                {
                                    name: 'consumed_active_power',
                                    title: 'Active Energy',
                                    unit: 'Wh',
                                    total: true
                                },
                                {
                                    name: 'consumed_reactive_power',
                                    title: 'Reactive Energy',
                                    unit: 'VArh',
                                    total: true
                                },
                                {
                                    name: 'power_factor',
                                    title: 'Power Factor',
                                    unit: ''
                                },
                                {
                                    name: 'frequency',
                                    title: 'Frequency',
                                    unit: 'Hz'
                                },
                            ]
                        },
                        {
                            name: 'state',
                            title: 'Meter State',
                        },
                        {
                            name: 'timestamp',
                            title: 'Time (UTC)'
                        }
                    ]
                }
            ],
        },
        {
            name: 'seismic',
            title: 'Seismic meters',
            sensors: [
                {
                    name: 'AccCore',
                    title: 'Accelerometer (NGM&nbsp;internal)',
                    scheme: [
                        {
                            name: 'accX',
                            title: 'Acc X',
                            color: '#e61510'
                        },
                        {
                            name: 'accY',
                            title: 'Acc Y',
                            color: '#0d74ce'
                        },
                        {
                            name: 'accZ',
                            title: 'Acc Z',
                            color: '#08860e'
                        },
                        {
                            name: 'accRawX',
                            title: 'Acc Raw X',
                            color: '#e67976'
                        },
                        {
                            name: 'accRawY',
                            title: 'Acc Raw Y',
                            color: '#69a8e0'
                        },
                        {
                            name: 'accRawZ',
                            title: 'Acc Raw Z',
                            color: '#68af6c'
                        },
                        {
                            name: 'timestamp',
                            title: 'Time (UTC)'
                        }
                    ]
                },
                {
                    name: 'AccPCB',
                    title: 'Accelerometer (NGM&nbsp;enclosure)',
                    scheme: [
                        {
                            name: 'accX',
                            title: 'Acc X',
                            color: '#e61510'
                        },
                        {
                            name: 'accY',
                            title: 'Acc Y',
                            color: '#0d74ce'
                        },
                        {
                            name: 'accZ',
                            title: 'Acc Z',
                            color: '#08860e'
                        },
                        {
                            name: 'accRawX',
                            title: 'Acc Raw X',
                            color: '#e67976'
                        },
                        {
                            name: 'accRawY',
                            title: 'Acc Raw Y',
                            color: '#69a8e0'
                        },
                        {
                            name: 'accRawZ',
                            title: 'Acc Raw Z',
                            color: '#68af6c'
                        },
                        {
                            name: 'timestamp',
                            title: 'Time (UTC)'
                        }
                    ]
                },
            ]
        },
        {
            name: 'dataSwitch',
            title: 'Data Switch',
            sensors: [
                {
                    name: 'DataSwitch',
                    scheme: [

                        {
                            name: 'port',
                            data: [
                                {
                                    name: 'rx',
                                    title: 'Incoming traffic',
                                    unit: 'B/sec'
                                },
                                {
                                    name: 'tx',
                                    title: 'Outgoing traffic',
                                    unit: 'B/sec'
                                },
                            ]
                        },
                        {
                            name: 'state',
                            title: 'Meter State',
                        },
                        {
                            name: 'timestamp',
                            title: 'Time (UTC)'
                        }
                    ]
                }
            ],
        },
        {
            name: 'powerLagLead',
            title: 'Power Lag/Lead Delivered/Received',
            sourceType: 'Power',
            sensors: [
                {
                    name: 'PowerMeter',
                    type: 'power'
                }
            ],
        },
        {
            name: 'energyLagLead',
            title: 'Energy Lag/Lead Delivered/Received',
            sourceType: 'Energy',
            sensors: [
                {
                    name: 'PowerMeter',
                    type: 'power'
                }
            ],
        },
    ])
    .constant('GOOGLE_MAP', {
        'config' : {
            key: 'AIzaSyB_PQ_VO1H6RK0BtK0LNtYGC_Hdio6FG00',
            v: '3.23',
            libraries: 'weather,geometry,visualization'
        }
    })
    .constant('SEVERITIES', [
        {
            'id': 0,
            'name': 'Emergency',
            'color': '#e53935'
        },
        {
            'id': 1,
            'name': 'Alert',
            'color': '#e53935'
        },
        {
            'id': 2,
            'name': 'Critical',
            'color': '#e53935'
        },
        {
            'id': 3,
            'name': 'Error',
            'color': '#e53935'
        },
        {
            'id': 4,
            'name': 'Warning',
            'color': '#666666'
        },
        {
            'id': 5,
            'name': 'Notice',
            'color': '#666666'
        },
        {
            'id': 6,
            'name': 'Informational',
            'color': '#666666'
        },
        {
            'id': 7,
            'name': 'Debug',
            'color': '#aaaaaa'
        }
    ])
;