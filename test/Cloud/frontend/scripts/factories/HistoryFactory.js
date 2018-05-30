'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('HistoryFactory',[
            '$q',
            '$log',
            'ApiFactory',
            'SEVERITIES',
            HistoryFactory
        ]);

    function HistoryFactory($q, $log, ApiFactory, SEVERITIES) {
        var eventsMock = [
            {
                "timestamp": "2016-01-02T09:20:30+01:00",
                "deviceId": "TEST-DEV-01",
                "eventType": "BLE_DISCONNECT",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-01-01T19:20:30+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "BLE_CONNECT",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-01-04T19:20:30+01:00",
                "deviceId": "TEST-DEV-01",
                "eventType": "DEVICE_CONNECTED",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-02-01T18:21:02+01:00",
                "deviceId": "TEST-DEV-01",
                "eventType": "DEVICE_DISCONNECTED",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-02-04T08:11:29+01:00",
                "deviceId": "TEST-DEV-01",
                "eventType": "HIGH_RESOLUTION_DATA_REQUESTED",
                "eventSource": "WEB_CONSOLE"
            },
            {
                "timestamp": "2016-02-05T22:22:22+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "HIGH_RESOLUTION_DATA_RECEIVED",
                "eventSource": "WEB_CONSOLE"
            },
            {
                "timestamp": "2016-02-01T12:20:55+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "CONFIGURATION_CHANGED",
                "eventSource": "WEB_UI"
            },
            {
                "timestamp": "2016-02-03T12:21:05+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "HIGH_RESOLUTION_DATA_REQUESTED",
                "eventSource": "WEB_UI"
            },
            {
                "timestamp": "2016-02-01T17:10:15+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "BLE_DISCONNECT",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-02-06T19:20:30+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "BLE_CONNECT",
                "eventSource": "DEVICE"
            },
            {
                "timestamp": "2016-02-07T01:01:00+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "HIGH_RESOLUTION_DATA_REQUESTED",
                "eventSource": "WEB_UI"
            },
            {
                "timestamp": "2016-02-04T01:02:02+01:00",
                "deviceId": "TEST-DEV-02",
                "eventType": "CONFIGURATION_CHANGED",
                "eventSource": "WEB_CONSOLE"
            }
        ];
        var logsMock = [
                {
                    "timestamp": "2016-01-02T09:20:30+01:00",
                    "deviceId": "TEST-DEV-01",
                    "command": "UPGRADE",
                    "result": "DONE"
                },
                {
                    "timestamp": "2016-01-02T09:20:33+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "UPGRADE",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-01-01T19:20:30+01:00",
                    "deviceId": "TEST-DEV-01",
                    "command": "TI_RESET",
                    "result": "TIMEOUT"
                },
                {
                    "timestamp": "2016-02-04T01:02:02+01:00",
                    "deviceId": "TEST-DEV-01",
                    "command": "SOM_RESET",
                    "result": "DONE"
                },
                {
                    "timestamp": "2016-01-02T09:20:30+01:00",
                    "deviceId": "TEST-DEV-01",
                    "command": "GET_HR_DATA",
                    "result": "DONE"
                },
                {
                    "timestamp": "2016-02-04T08:11:29+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "TI_RESET",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-01-02T09:20:30+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-02-01T12:20:55+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-01-02T09:20:30+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-02-01T17:10:15+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-02-01T19:10:25+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                },
                {
                    "timestamp": "2016-02-03T12:21:05+01:00",
                    "deviceId": "TEST-DEV-02",
                    "command": "GET_HR_DATA",
                    "result": "REJECTED"
                }
            ];
        var eventsDescriptionMap = [
            {
                'type': 'BLUETOOTH_CLIENT_CONNECTED',
                'description': 'Bluetooth client connected description'
            },
        ];
        var severitiesMap = [
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
        ];

        // region Mock handlers
        function SortBy(fieldName){
            var sortOrder = 1;
            if(fieldName[0] === "-") {
                sortOrder = -1;
                fieldName = fieldName.substr(1);
            }
            return function (a,b) {
                var aValue = a[fieldName].toLowerCase();
                var bValue = b[fieldName].toLowerCase();
                var result = ((aValue < bValue) ? -1 : ((aValue > bValue) ? 1 : 0));
                return result * sortOrder;
            }
        }

        function ItemsMockHandler(itemsMock, query){
            var items = itemsMock.filter(function(event){
                return (
                    (query.filter.dateFrom ? new Date(event.timestamp).getTime() >= new Date(query.filter.dateFrom).getTime() : true) &&
                    (query.filter.dateTo ? new Date(event.timestamp).getTime() <= new Date(query.filter.dateTo).getTime() : true) &&
                    (query.filter.deviceId ? event.deviceId == query.filter.deviceId : true) &&
                    (query.filter.eventSource ? event.eventSource == query.filter.eventSource : true) &&
                    (query.filter.eventType ? event.eventType == query.filter.eventType : true)
                );
            }).sort(SortBy(query.order));
            console.log(items);
            return {
                "total" : items.length,
                "items" : items.slice((query.page - 1) * query.limit, query.page * query.limit)
            }
        }
        // endregion

        function _getLogs(query) {
            return $q(function(resolve, reject){
                resolve({'data': ItemsMockHandler(logsMock, query)});
            });
        }

        function _getEvents(query) {
            return ApiFactory
                .getEventsHistoryById(query.deviceId, query.params)
                .then(function(response){
                    if(!response.data || (response.data.data instanceof Array) == false){
                        throw new Error("Events array is empty");
                    }
                    return response.data;
                })
                .then(function(events){
                    events.data = _mapSeverities(events.data);
                    return events;
                })
            ;
        }

        function _mapSeverities(events){
            return events.map(function(event){
                eventsDescriptionMap.map(function(eventDescription){
                    if(event.eventType == eventDescription.type){
                        event.description = eventDescription.description
                    }
                });
                SEVERITIES.map(function(severity){
                    if(event.eventSeverity == severity.id){
                        event.severityTitle = severity.name;
                        event.severityColor = severity.color;
                    }
                });
                return event;
            });
        }

        function _getEventsRecent(query) {
            return ApiFactory
                .getEventsRecent()
                .then(function(response){
                    //if(response.data instanceof Array == false){
                    if(!angular.isArray(response.data)){
                        throw new Error("Events array is empty");
                    }
                    return angular.copy(response.data)
                })
                .then(function(events){
                    return _mapSeverities(events);
                })
                .catch(function(error) {
                    $log.error('getEventsRecent failed:', error);
                })
            ;
        }

        function _exportEvents(query) {
            return ApiFactory
                .exportEventsById(query.deviceId, query.params)
                .then(function(response){
                    return {
                        fileName    : response.headers()['content-disposition'].split(';')[1].trim().split('=')[1].replace(/['"]+/g, ''),
                        fileContent : !!response.data ? response.data : ''
                    };
                })
            ;
        }

        function _exportRegularData(query) {
            return ApiFactory
                .exportRegularDataById(query.deviceId, query.params)
                .then(function(response){
                    return {
                        fileName    : response.headers()['content-disposition'].split(';')[1].trim().split('=')[1].replace(/['"]+/g, ''),
                        fileContent : !!response.data ? response.data : ''
                    };
                })
            ;
        }

        return {
            getLogs             : _getLogs,
            getEvents           : _getEvents,
            getEventsRecent     : _getEventsRecent,
            exportEvents        : _exportEvents,
            exportRegularData   : _exportRegularData
        }
    }
})(angular);