const express = require('express');
const mongoose = require('mongoose');
const mqtt = require('mqtt');
const app = express();

app.use(express.json());

mongoose.connect('mongodb+srv://root:Agista0605.@trashbin.ydrv7.mongodb.net/TrashBin')

const mqttClient = mqtt.connect('mqtt://broker.hivemq.com:1883');
const topic = 'trashbin/data';

const dataSchema = new mongoose.Schema({
    kategori: Number,
    jarak: Number,
    timestamp: { type: Date, default: Date.now }
});
const dataModel = mongoose.model('data', dataSchema);

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe(topic);
});

mqttClient.on('message', async (topic, message) => {
    try {
        const data = JSON.parse(message.toString());
        const newData = new dataModel({
            kategori: data.kategori,
            jarak: data.jarak
        });
        await newData.save();
        console.log('Data saved from MQTT:', data);
    } catch (error) {
        console.error('Error saving data:', error);
    }
});

app.get('/api/', async (req, res) => {
    try {
        const data = await dataModel.find().sort({ timestamp: -1 });
        res.json(data);
    } catch (error) {
        res.status(500).json({ error: 'Error fetching data' });
    }
});

app.listen(3000, () => {
    console.log('Server is running on port 3000');
});

