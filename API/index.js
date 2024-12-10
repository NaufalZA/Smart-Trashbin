const express = require('express');
const mongoose = require('mongoose');
const mqtt = require('mqtt');
const cors = require('cors'); 
const app = express();

app.use(cors()); 
app.use(express.json());

mongoose.connect('mongodb+srv://root:Agista0605.@trashbin.ydrv7.mongodb.net/TrashBin')

const mqttClient = mqtt.connect('mqtt://test.mosquitto.org:1883');
// const mqttClient = mqtt.connect('mqtt://broker.hivemq.com:1883');
const topic = 'trashbin/data';

const dataSchema = new mongoose.Schema({
    kategori: {
        type: Number,
        required: true,
        validate: {
            validator: function(v) {
                return v === 1 || v === 2 || v === 3;
            },
            message: 'Kategori harus 1, 2, atau 3'
        }
    },
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
        
        // Validasi kategori
        if (![1, 2, 3].includes(data.kategori)) {
            console.error('Invalid kategori:', data.kategori);
            return;
        }

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

app.get('/api/harian', async (req, res) => {
    try {
        const today = new Date();
        today.setHours(0, 0, 0, 0);
        
        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: { $gte: today }
                }
            },
            {
                $group: {
                    _id: "$kategori",
                    count: { $sum: 1 }
                }
            },
            {
                $sort: { _id: 1 }
            }
        ]);

        const formattedResult = result.map(item => ({
            kategori: item._id,
            jumlah: item.count
        }));

        res.json(formattedResult);
    } catch (error) {
        res.status(500).json({ error: 'Gagal menghitung data harian' });
    }
});

app.get('/api/bulanan', async (req, res) => {
    try {
        const startOfMonth = new Date();
        startOfMonth.setDate(1);
        startOfMonth.setHours(0, 0, 0, 0);

        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: { $gte: startOfMonth }
                }
            },
            {
                $group: {
                    _id: "$kategori",
                    count: { $sum: 1 }
                }
            },
            {
                $sort: { _id: 1 }
            }
        ]);

        const formattedResult = result.map(item => ({
            kategori: item._id,
            jumlah: item.count
        }));

        res.json(formattedResult);
    } catch (error) {
        res.status(500).json({ error: 'Gagal menghitung data bulanan' });
    }
});

app.get('/api/tahunan', async (req, res) => {
    try {
        const startOfYear = new Date();
        startOfYear.setMonth(0, 1);
        startOfYear.setHours(0, 0, 0, 0);

        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: { $gte: startOfYear }
                }
            },
            {
                $group: {
                    _id: "$kategori",
                    count: { $sum: 1 }
                }
            },
            {
                $sort: { _id: 1 }
            }
        ]);

        const formattedResult = result.map(item => ({
            kategori: item._id,
            jumlah: item.count
        }));

        res.json(formattedResult);
    } catch (error) {
        res.status(500).json({ error: 'Gagal menghitung data tahunan' });
    }
});

app.get('/api/range', async (req, res) => {
    try {
        const { start, end } = req.query;
        const startDate = new Date(start);
        const endDate = new Date(end);
        endDate.setHours(23, 59, 59, 999);

        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: { 
                        $gte: startDate,
                        $lte: endDate
                    }
                }
            },
            {
                $group: {
                    _id: "$kategori",
                    count: { $sum: 1 }
                }
            },
            {
                $sort: { _id: 1 }
            }
        ]);

        const formattedResult = result.map(item => ({
            kategori: item._id,
            jumlah: item.count
        }));

        res.json(formattedResult);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil data range' });
    }
});

app.get('/api/range/detail', async (req, res) => {
    try {
        const { start, end } = req.query;
        const startDate = new Date(start);
        const endDate = new Date(end);
        endDate.setHours(23, 59, 59, 999);

        const data = await dataModel.find({
            timestamp: {
                $gte: startDate,
                $lte: endDate
            }
        }).sort({ timestamp: -1 });

        const formattedData = data.map(item => ({
            kategori: item.kategori,
            jarak: item.jarak,
            timestamp: new Date(item.timestamp).toLocaleString("id-ID", { 
                timeZone: "Asia/Jakarta" 
            }).replace(/\//g, '-').replace(/\./g, ':')
        }));

        res.json(formattedData);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil data detail range' });
    }
});

app.listen(3000, () => {
    console.log('Server is running on port 3000');
});

