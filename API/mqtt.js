const mqtt = require('mqtt');

// MQTT broker configuration
const brokerConfig = {
    host: 'broker.hivemq.com',  // Using HiveMQ public broker as example
    port: 1883,
    protocol: 'mqtt'
};

// Create MQTT client
const client = mqtt.connect(brokerConfig);

// Handle connection events
client.on('connect', () => {
    console.log('Connected to MQTT broker');
    
    // Subscribe to topics
    client.subscribe('smartbin/#', (err) => {
        if (!err) {
            console.log('Subscribed to smartbin topics');
        }
    });
});

// Handle incoming messages
client.on('message', (topic, message) => {
    console.log(`Received message on ${topic}: ${message.toString()}`);
});

// Handle errors
client.on('error', (error) => {
    console.error('MQTT Error:', error);
});

// Handle disconnection
client.on('close', () => {
    console.log('Disconnected from MQTT broker');
});

module.exports = client;
