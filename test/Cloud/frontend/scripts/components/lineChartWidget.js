'use strict';
var tt;

(function() {
    angular
        .module('NgmAdmin')
        .component('lineChartWidget', {
            templateUrl: 'scripts/components/lineChartWidget-tmpl.html',
            controller: LineChartWidgetController,
            bindings: {
                sensorsNames: '=',
                labels: '=',
                borderColors: '=',
                emptyTitle: '@'
            }
        });

    function LineChartWidgetController($scope, $timeout, $window, BufferFactory) {

        //region Init
        const ctrl = this;

        ctrl.expanded = false;
        ctrl.frozen = false;
        ctrl.paramForShow = {
            'name': '',
            'title': ''
        };

        var theChart = null;
        var chartRange = 20;

        var options = {
            animation: {
                duration: 0,
                easing: 'linear'
            },
            legend: {
                display: true
            },
            elements: {
                line: {
                    tension: 0
                }
            },
            maintainAspectRatio: false,
            scales: {
                yAxes: [
                    {
                        id: 'y-axis-1',
                        type: 'linear',
                        display: true,
                        position: 'right'
                    }
                ]
            }
        };

        var chartLineOptions = {
            data: [],
            yAxisID: 'y-axis-1',
            fill: true,
            borderColor: ''
        };

        var bgColors = [
            'rgba(229, 57, 53, .02)', 'rgba(30, 136, 229, .02)', 'rgba(46, 125, 50, .02)',
            'rgba(229, 57, 53, .02)', 'rgba(30, 136, 229, .02)', 'rgba(46, 125, 50, .02)'
        ];

        Clear();
        // endregion

        // region Private methods
        function Clear(){
            var data = Array.apply(null, Array(ctrl.labels.length)).map(Array.prototype.valueOf, []);
            var labels = Array.apply(null, Array(chartRange)).map(String.prototype.valueOf, '');

            var datasets = [];
            data.forEach(function(line, idx){
                datasets.push(angular.copy(chartLineOptions));
                datasets[datasets.length - 1].data = angular.copy(line);
                datasets[datasets.length - 1].borderColor = ctrl.borderColors[idx];
                datasets[datasets.length - 1].backgroundColor = bgColors[idx];
                datasets[datasets.length - 1].label = ctrl.labels[idx];
            }, []);

            $timeout(function () {
                if(document.getElementById("overviewChart")) {
                    if(!!theChart){
                        theChart.config.data = {
                            datasets: datasets,
                            labels: labels,
                        };
                    }
                    else{
                        theChart = new Chart(document.getElementById("overviewChart"), {
                            type: 'line',
                            data: {
                                datasets: datasets,
                                labels: labels,
                            },
                            options: options
                        });
                    }
                }

            }, 300);
        }

        function ApplyData(buffer){
            if(!!theChart && !ctrl.frozen){
                buffer.labels.forEach(function(label, label_idx){
                    theChart.data.labels[label_idx] = label;
                });

                buffer.data.forEach(function(line, line_idx){
                    line.forEach(function(value, value_idx){
                        theChart.data.datasets[line_idx].data[value_idx] = parseFloat(angular.isObject(value) ? value.current_value : value).toFixed(3);
                    });
                });

                theChart.update();
            }
        }

        function RefreshChartContainer(){
            var height = document.querySelectorAll('line-chart-widget md-content')[0].offsetHeight;
            document.querySelectorAll('line-chart-widget md-content')[0].style.lineHeight = (height - 32) + 'px';
        }
        // endregion

        // region UI events
        $scope.onClick = function (points, evt) {
            console.log(points, evt);
        };

        $scope.freezeChart = function(freeze){
            ctrl.frozen = freeze;
        };

        ctrl.expand = function(){
            ctrl.expanded = !ctrl.expanded;
            $timeout(function(){
                window.dispatchEvent(new Event('resize'));
            }, 300);

        };
        // endregion

        // region Event Handlers
        $scope.$on('panelItemSelected', function(event, param){
            if(ctrl.paramForShow.name != param.name){
                ctrl.paramForShow = angular.copy(param);
                ApplyData(BufferFactory.getBuffer(ctrl.paramForShow.sensor_name, ctrl.paramForShow.name, chartRange));
            }
        });
        $scope.$on('$receivedDeviceMeter', function(){
            ApplyData(BufferFactory.getBuffer(ctrl.paramForShow.sensor_name, ctrl.paramForShow.name, chartRange));
        });
        $scope.$on('$activeDeviceChanged', function(event, regular){
            Clear();
            ApplyData(BufferFactory.getBuffer(ctrl.paramForShow.sensor_name, ctrl.paramForShow.name, chartRange));
        });

        //$scope.$on("$destroy", function(){
        //    angular.element($window).unbind('resize');
        //});
        $scope.$on("$windowResized", function(){
            RefreshChartContainer()
        });
        // endregion

        //angular.element($window).bind('resize', function(){
        //    RefreshChartContainer()
        //});
  }
})();