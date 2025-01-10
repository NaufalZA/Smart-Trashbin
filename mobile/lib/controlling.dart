import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart' as mqtt;
import 'package:mqtt_client/mqtt_server_client.dart';

class ControllingPage extends StatefulWidget {
  const ControllingPage({Key? key}) : super(key: key);

  @override
  _ControllingPageState createState() => _ControllingPageState();
}

class _ControllingPageState extends State<ControllingPage> {
  bool isLidOpen = false;
  bool isConnected = false;
  MqttServerClient? client;
  bool _mounted = true;

  @override
  void initState() {
    super.initState();
    // Using Future.microtask to avoid calling setState during build
    Future.microtask(() => _initializeMQTTClient());
  }

  @override
  void dispose() {
    _mounted = false;
    _cleanupMQTT();
    super.dispose();
  }

  void _cleanupMQTT() {
    if (client?.connectionStatus?.state == mqtt.MqttConnectionState.connected) {
      try {
        final builder = mqtt.MqttClientPayloadBuilder();
        builder.addString('offline');
        client?.publishMessage(
          'trashbin/status',
          mqtt.MqttQos.atLeastOnce,
          builder.payload!,
          retain: true
        );
        client?.disconnect();
      } catch (e) {
        print('Error during cleanup: $e');
      }
    }
    client = null;
  }

  void _updateConnectionState(bool connected) {
    if (_mounted) {
      setState(() {
        isConnected = connected;
      });
    }
  }

  Future<void> _initializeMQTTClient() async {
    if (!_mounted) return;

    try {
      client = MqttServerClient('test.mosquitto.org', '');
      client?.port = 1883;
      client?.keepAlivePeriod = 60;
      client?.clientIdentifier = 'mobile_client_${DateTime.now().millisecondsSinceEpoch}';

      await client?.connect();
      
      if (client?.connectionStatus?.state == mqtt.MqttConnectionState.connected) {
        _updateConnectionState(true);
        
        // Subscribe to topic
        client?.subscribe('trashbin/pintu', mqtt.MqttQos.atLeastOnce);
        
        // Listen for updates
        client?.updates?.listen((List<mqtt.MqttReceivedMessage<mqtt.MqttMessage>> c) {
          if (!_mounted) return;
          
          final message = c[0].payload as mqtt.MqttPublishMessage;
          final payload = mqtt.MqttPublishPayload.bytesToStringAsString(message.payload.message);
          
          setState(() {
            isLidOpen = payload == '1';
          });
        });
      }
    } catch (e) {
      print('MQTT Connection failed: $e');
      _updateConnectionState(false);
      if (_mounted) {
        _showMessage('Koneksi MQTT gagal', isError: true);
      }
    }
  }

  void _setupSubscription() {
    client?.subscribe('trashbin/pintu', mqtt.MqttQos.atLeastOnce);
    
    client?.updates?.listen((List<mqtt.MqttReceivedMessage<mqtt.MqttMessage>> c) {
      if (!_mounted) return;
      
      try {
        final message = c[0].payload as mqtt.MqttPublishMessage;
        final payload = mqtt.MqttPublishPayload.bytesToStringAsString(message.payload.message);
        
        if (c[0].topic == 'trashbin/pintu') {
          setState(() {
            isLidOpen = payload == '1';
          });
        }
      } catch (e) {
        print('Error processing message: $e');
      }
    });
  }

  void _publishOnlineStatus() {
    try {
      final builder = mqtt.MqttClientPayloadBuilder();
      builder.addString('online');
      client?.publishMessage(
        'trashbin/status',
        mqtt.MqttQos.atLeastOnce,
        builder.payload!,
        retain: true
      );
    } catch (e) {
      print('Error publishing online status: $e');
    }
  }

  void _onConnected() {
    print('Connected to MQTT Broker');
    _updateConnectionState(true);
  }

  void _onDisconnected() {
    print('Disconnected from MQTT Broker');
    _updateConnectionState(false);
    _scheduleReconnect();
  }

  void _onSubscribed(String topic) {
    print('Subscribed to $topic');
  }

  void _scheduleReconnect() {
    if (!_mounted || isConnected) return;
    Future.delayed(const Duration(seconds: 5), _initializeMQTTClient);
  }

  Future<void> toggleLid() async {
    if (!isConnected || client == null) {
      _showMessage('Tidak dapat mengontrol, MQTT tidak terkoneksi', isError: true);
      return;
    }

    try {
      final newState = !isLidOpen;
      final builder = mqtt.MqttClientPayloadBuilder();
      builder.addString(newState ? '1' : '0');

      client?.publishMessage(
        'trashbin/pintu',
        mqtt.MqttQos.atLeastOnce,
        builder.payload!,
      );

      setState(() {
        isLidOpen = newState;
      });
      
      _showMessage(newState ? 'Tong sampah terbuka' : 'Tong sampah tertutup');
    } catch (e) {
      print('Error toggling lid: $e');
      _showMessage('Gagal mengirim perintah', isError: true);
    }
  }

  void _showMessage(String message, {bool isError = false}) {
    if (!_mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: isError ? Colors.red : Colors.green,
        duration: const Duration(seconds: 2),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Controlling Tong Sampah'),
        backgroundColor: const Color(0xFF4A90E2),
        actions: [
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Icon(
              isConnected ? Icons.cloud_done : Icons.cloud_off,
              color: isConnected ? Colors.green : Colors.red,
            ),
          )
        ],
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(
              isLidOpen ? Icons.delete_outline : Icons.delete,
              size: 100,
              color: isLidOpen ? Colors.green : Colors.red,
            ),
            const SizedBox(height: 20),
            Text(
              isLidOpen ? 'Tong Sampah Terbuka' : 'Tong Sampah Tertutup',
              style: const TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 10),
            Text(
              isConnected ? 'Terkoneksi' : 'Tidak Terkoneksi',
              style: TextStyle(
                fontSize: 16,
                color: isConnected ? Colors.green : Colors.red,
              ),
            ),
            const SizedBox(height: 30),
            ElevatedButton.icon(
              onPressed: isConnected ? toggleLid : null,
              icon: Icon(isLidOpen ? Icons.close : Icons.open_in_full),
              label: Text(isLidOpen ? 'Tutup Lid' : 'Buka Lid'),
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFF50E3C2),
                padding: const EdgeInsets.symmetric(horizontal: 40, vertical: 15),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(30),
                ),
                disabledBackgroundColor: Colors.grey,
              ),
            ),
          ],
        ),
      ),
    );
  }
}