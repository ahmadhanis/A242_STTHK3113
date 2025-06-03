import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

class MQTTService {
  static const String mqttBroker = 'test.mosquitto.org';
  static const int mqttPort = 1883;
  static const String mqttClientId = 'ESP32_DHT11_Client';

  final client = MqttServerClient(mqttBroker, mqttClientId);

  Function(String topic, String message)? onMessageReceived;

  Future<void> connect() async {
    client.port = mqttPort;
    client.keepAlivePeriod = 60;
    client.onDisconnected = _onDisconnected;
    client.logging(on: true); // Turn ON logging to help debug
    client.onConnected = _onConnected;
    client.onSubscribed = _onSubscribed;

    client.connectionMessage = MqttConnectMessage()
        .withClientIdentifier(mqttClientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    try {
      print('🔌 Connecting to MQTT broker...');
      final result = await client.connect();

      if (client.connectionStatus?.state == MqttConnectionState.connected) {
        print('✅ Connected to MQTT broker');

        // Subscribe to topic
        subscribeToTopic('esp32/dht11/temperature');
        subscribeToTopic('esp32/dht11/humidity');

        // Listen to messages only when connected
        client.updates?.listen((List<MqttReceivedMessage<MqttMessage>> c) {
          final recMess = c[0].payload as MqttPublishMessage;
          final payload = MqttPublishPayload.bytesToStringAsString(
            recMess.payload.message,
          );
          final topic = c[0].topic;

          print('📩 Received message: [$topic] $payload');

          if (onMessageReceived != null) {
            onMessageReceived!(topic, payload);
          }
        });
      } else {
        print(
          '❌ Connection failed - status: ${client.connectionStatus?.state}',
        );
        client.disconnect();
      }
    } catch (e) {
      print('❌ MQTT Connect error: $e');
      client.disconnect();
    }
  }

  void subscribeToTopic(String topic) {
    client.subscribe(topic, MqttQos.atLeastOnce);
  }

  void _onConnected() => print('MQTT connected');
  void _onDisconnected() => print('MQTT disconnected');
  void _onSubscribed(String topic) => print('Subscribed to $topic');
}
