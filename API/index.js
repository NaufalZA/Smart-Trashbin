const express = require('express');
const mongoose = require('mongoose');
const mqtt = require('mqtt');
const app = express();

app.use(express.json());

mongoose.connect('mongodb+srv://root:Agista0605.@trashbin.ydrv7.mongodb.net/TrashBin')

const mqttClient = mqtt.connect('mqtt://test.mosquitto.org:1883');
// const mqttClient = mqtt.connect('mqtt://broker.hivemq.com:1883');
const topic = 'trashbin/data';

const dataSchema = new mongoose.Schema({
    kategori: Number,
    jarak: Number,
    timestamp: { type: Date, default: Date.now }
}, { versionKey: false });
const dataModel = mongoose.model('data', dataSchema);

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe(topic);
});

mqttClient.on('error', (error) => {
    console.log('MQTT Error:', error);
});

mqttClient.on('close', () => {
    console.log('MQTT connection closed. Attempting to reconnect...');
});

mqttClient.on('reconnect', () => {
    console.log('Attempting to reconnect to MQTT broker...');
});

mqttClient.on('message', async (topic, message) => {
    try {
        const data = JSON.parse(message.toString());
        const newData = new dataModel({
            kategori: data.kategori,
            jarak: data.jarak,
            timestamp: new Date()
        });
        await newData.save();
        console.log('Berhasil menyimpan data:', data);
    } catch (error) {
        console.error('Gagal menyimpan data:', error);
    }
});

app.get('/api/getdata', async (req, res) => {
    try {
        const data = await dataModel.find().sort({ timestamp: -1 });
        const formattedData = data.map(item => ({
            kategori: item.kategori,
            jarak: item.jarak,
            timestamp: new Date(item.timestamp).toLocaleString("id-ID", { 
                timeZone: "Asia/Jakarta" 
            }).replace(/\//g, '-').replace(/\./g, ':')
        }));
        res.json(formattedData);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil data' });
    }
});


app.listen(3000, () => {
    console.log('Server is running on port 3000');
});

