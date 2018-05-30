'use strict';

(function(angular){
    angular
        .module('NgmAdmin')
        .factory('BufferFactory',[
            '$rootScope',
            BufferFactory
        ]);

    function BufferFactory($rootScope){
        // region Init
        var buffer = [];
        var bufferSize = 100;
        // endregion

        // region private methods
        function pushInBuffer(slice){
            buffer.push(slice);
            if(buffer.length > bufferSize){
                buffer.splice(0, 1);
            }
        }
        // endregion

        // region Public methods
        function _getBuffer(sensor_name, param, size) {
            var selectedBySensor = buffer.reduce(function(accum, current){
                var item = current.find(function(item){
                    return item.name == sensor_name;
                });
                if(item){
                    accum.push(item);
                }
                return accum;
            }, []);

            var isAccelerometer = ('AccPCB' == sensor_name || 'AccCore' == sensor_name);
            var reduced_buffer = selectedBySensor.reduce(function(accum, current){
                var tempValue = (angular.isObject(current['timestamp']) ? current['timestamp'].current_value : current['timestamp']);
                accum.labels.push((!!tempValue) ? moment.utc(tempValue).format('hh:mm:ss') : '');

                if(isAccelerometer){
                    tempValue = (angular.isObject(current['accX']) ? current['accX'].current_value : current['accX']);
                    accum.data[0].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current['accY']) ? current['accY'].current_value : current['accY']);
                    accum.data[1].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current['accZ']) ? current['accZ'].current_value : current['accZ']);
                    accum.data[2].push((!!tempValue) ? tempValue : 0);


                    tempValue = (angular.isObject(current['gyroX']) ? current['gyroX'].current_value : current['gyroX']);
                    accum.data[3].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current['gyroY']) ? current['gyroY'].current_value : current['gyroY']);
                    accum.data[4].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current['gyroZ']) ? current['gyroZ'].current_value : current['gyroZ']);
                    accum.data[5].push((!!tempValue) ? tempValue : 0);

                }
                else{
                    tempValue = (angular.isObject(current[param + '1']) ? current[param + '1'].current_value : current[param + '1']);
                    accum.data[0].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current[param + '2']) ? current[param + '2'].current_value : current[param + '2']);
                    accum.data[1].push((!!tempValue) ? tempValue : 0);

                    tempValue = (angular.isObject(current[param + '3']) ? current[param + '3'].current_value : current[param + '3']);
                    accum.data[2].push((!!tempValue) ? tempValue : 0);
                }
                return accum;
            }, ({
                'labels': [],
                data: isAccelerometer ? [[], [], [], [], [], []] : [[], [], []],
                                    //  Array.apply(null, Array(isAccelerometer ? 6 : 3)).map(Array.prototype.valueOf, [])   - does not work
            }));


            if(!!size){
                reduced_buffer.labels = reduced_buffer.labels.slice(-size);
                reduced_buffer.data = reduced_buffer.data.map(function(phase){
                    return phase.slice(-size);
                });
            }

            return reduced_buffer;
        }
        // endregion

        // region Event handlers
        $rootScope.$on('activeDeviceSensorDataReceived', function(event, regular){
            pushInBuffer(angular.copy(regular.payload.sensors));
            $rootScope.$broadcast('$receivedDeviceMeter');
        });

        $rootScope.$on('$activeDeviceChanged', function(event, regular){
            buffer = [];
        });
        // endregion

        return {
            getBuffer                   :  _getBuffer
        }
    }
})(angular);