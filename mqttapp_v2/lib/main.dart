import 'package:flutter/material.dart';
import 'mqtt_service.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final mqttService = MQTTService();
  String temperature = '-';
  String humidity = '-';

  @override
  void initState() {
    super.initState();

    mqttService.onMessageReceived = (topic, message) {
      print('📩 MQTT: $topic → $message');
      setState(() {
        temperature = topic.contains('temperature') ? message : temperature;
        humidity = topic.contains('humidity') ? message : humidity;
        print(temperature);
        print(humidity);
        // if (topic.contains('temperature')) {
        //   temperature = message;
        // } else if (topic.contains('humidity')) {
        //   humidity = message;
        // }
      });
    };

    mqttService
        .connect()
        .then((_) {
          print('✅ MQTT connected and ready');
        })
        .catchError((err) {
          print('❌ MQTT connection error: $err');
        });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('ESP32 DHT11 Monitor')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                'Temperature: $temperature °C',
                style: const TextStyle(fontSize: 24),
              ),
              const SizedBox(height: 20),
              Text(
                'Humidity: $humidity %',
                style: const TextStyle(fontSize: 24),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
