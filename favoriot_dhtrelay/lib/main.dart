import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';

import 'package:intl/intl.dart';

void main() => runApp(FavoriotApp());

class FavoriotApp extends StatelessWidget {
  const FavoriotApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Favoriot ESP32 Monitor',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: SensorDashboard(),
    );
  }
}

class SensorDashboard extends StatefulWidget {
  const SensorDashboard({super.key});

  @override
  _SensorDashboardState createState() => _SensorDashboardState();
}

class _SensorDashboardState extends State<SensorDashboard> {
  List<SensorData> _sensorData = [];
  bool _relayStatus = false;

  final String apiKey = '';
  final String deviceId = 'esp32_project_1_device@ahmadhanis';
  final String baseUrl = 'https://apiv2.favoriot.com/v2/streams';

  @override
  void initState() {
    super.initState();
    fetchSensorData();
    // fetchRelayStatus();
  }

  Future<void> fetchSensorData() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl?device_developer_id=$deviceId&max=20'),
        headers: {'apikey': apiKey},
      );

      // log(response.body);

      if (response.statusCode == 200) {
        final List data = json.decode(response.body)['results'];
        // log('Fetched ${data.length} sensor data points');
        setState(() {
          _sensorData =
              data
                  .map((item) => SensorData.fromJson(item))
                  .toList()
                  .reversed
                  .toList();
        });
      }
    } catch (e) {
      print('❌ fetchSensorData failed: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('ESP32 Sensor Monitor'),
        actions: [
          IconButton(
            icon: Icon(Icons.refresh),
            onPressed: () {
              fetchSensorData();
              // fetchRelayStatus();
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(12.0),
        child: Column(
          children: [
            // Row(
            //   mainAxisAlignment: MainAxisAlignment.spaceBetween,
            //   children: [
            //     Text('Relay', style: TextStyle(fontSize: 18)),
            //     Switch(
            //       value: _relayStatus,
            //       onChanged: (value) => toggleRelay(value),
            //     ),
            //   ],
            // ),
            SizedBox(height: 20),
            Expanded(child: LineChart(getLineChartData())),
          ],
        ),
      ),
    );
  }

  List<FlSpot> getTemperatureData() {
    List<FlSpot> spots = [];
    for (int i = 0; i < _sensorData.length; i++) {
      spots.add(FlSpot(i.toDouble(), _sensorData[i].temperature));
    }
    return spots;
  }

  List<FlSpot> getHumidityData() {
    List<FlSpot> spots = [];
    for (int i = 0; i < _sensorData.length; i++) {
      spots.add(FlSpot(i.toDouble(), _sensorData[i].humidity));
    }
    return spots;
  }

  LineChartData getLineChartData() {
    final timeLabels = getTimeLabels();

    return LineChartData(
      minY: 0,
      titlesData: FlTitlesData(
        leftTitles: AxisTitles(
          sideTitles: SideTitles(showTitles: true, reservedSize: 40),
          axisNameWidget: Text('Value', style: TextStyle(fontSize: 12)),
          axisNameSize: 30,
        ),
        bottomTitles: AxisTitles(
          sideTitles: SideTitles(
            showTitles: true,
            interval: 2,
            getTitlesWidget: (value, meta) {
              int index = value.toInt();
              if (index < 0 || index >= timeLabels.length) return Container();
              return Text(timeLabels[index], style: TextStyle(fontSize: 10));
            },
          ),
          axisNameWidget: Text('Time', style: TextStyle(fontSize: 12)),
          axisNameSize: 30,
        ),
      ),
      borderData: FlBorderData(show: true),
      lineBarsData: [
        LineChartBarData(
          isCurved: true,
          spots: getTemperatureData(),
          color: Colors.blue,
          barWidth: 2,
          belowBarData: BarAreaData(show: false),
          dotData: FlDotData(show: true),
          showingIndicators: List.generate(
            _sensorData.length,
            (index) => index,
          ),
        ),
        LineChartBarData(
          isCurved: true,
          spots: getHumidityData(),
          color: Colors.green,
          barWidth: 2,
          belowBarData: BarAreaData(show: false),
          dotData: FlDotData(show: true),
          showingIndicators: List.generate(
            _sensorData.length,
            (index) => index,
          ),
        ),
      ],
      lineTouchData: LineTouchData(
        enabled: true,
        touchTooltipData: LineTouchTooltipData(
          getTooltipItems: (List<LineBarSpot> touchedSpots) {
            return touchedSpots.map((barSpot) {
              final value = barSpot.y;
              final unit = barSpot.bar.color == Colors.blue ? '°C' : '%';
              return LineTooltipItem(
                '${value.toStringAsFixed(1)} $unit',
                TextStyle(color: barSpot.bar.color),
              );
            }).toList();
          },
        ),
      ),
    );
  }

  List<String> getTimeLabels() {
    return _sensorData
        .map((e) => DateFormat.Hms().format(e.timestamp))
        .toList();
  }

  Future<void> fetchRelayStatus() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl?device_developer_id=$deviceId&max=1'),
        headers: {'apikey': apiKey},
      );
      print('GET /relayStatus → status: ${response.statusCode}');
      print('Body: ${response.body}');

      if (response.statusCode == 200) {
        final data = json.decode(response.body)['results'][0]['data'];
        setState(() {
          _relayStatus = data['relay'] == 'on';
        });
      }
    } catch (e) {
      print('❌ fetchRelayStatus failed: $e');
    }
  }

  Future<void> toggleRelay(bool value) async {
    final status = value ? 'on' : 'off';
    final body = jsonEncode({
      'device_developer_id': deviceId,
      'data': {'relay': status},
    });

    try {
      final response = await http.post(
        Uri.parse(baseUrl),
        headers: {'Content-Type': 'application/json', 'apikey': apiKey},
        body: body,
      );
      print('POST /toggleRelay → status: ${response.statusCode}');
      print('Body: ${response.body}');

      if (response.statusCode == 201) {
        setState(() => _relayStatus = value);
      }
    } catch (e) {
      print('❌ toggleRelay failed: $e');
    }
  }
}

class SensorData {
  final DateTime timestamp;
  final double temperature;
  final double humidity;

  SensorData({
    required this.timestamp,
    required this.temperature,
    required this.humidity,
  });

  factory SensorData.fromJson(Map<String, dynamic> json) {
    return SensorData(
      timestamp: DateTime.fromMillisecondsSinceEpoch(
        json['timestamp'],
      ).add(Duration(hours: 8)),
      temperature:
          double.tryParse(json['data']['temperature'].toString()) ?? 0.0,
      humidity: double.tryParse(json['data']['humidity'].toString()) ?? 0.0,
    );
  }
}
