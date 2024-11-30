const express = require('express');
const mongoose = require('mongoose');
const mqtt = require('mqtt');
const app = express();

app.use(express.json());

mongoose.connect('mongodb+srv://root:Agista0605.@trashbin.ydrv7.mongodb.net/TrashBin')

const mqttOptions = {
    keepalive: 10,
    reconnectPeriod: 1000,
    connectTimeout: 20000
};

const mqttClient = mqtt.connect('mqtt://broker.hivemq.com:1883', mqttOptions);
const topic = 'trashbin/data';

const dataSchema = new mongoose.Schema({
    kategori: Number,
    jarak: Number,
    timestamp: { 
        type: String, 
        default: () => {
            const date = new Date();
            return date.toLocaleString("id-ID", { timeZone: "Asia/Jakarta" })
                .replace(/\//g, '-')
                .replace(/\./g, ':');
        }
    }
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
        const existingData = await dataModel.findOne({
            kategori: data.kategori,
            jarak: data.jarak,
            timestamp: new Date().toLocaleString("id-ID", { timeZone: "Asia/Jakarta" })
        });
        if (existingData) {
            console.log('Data duplikasi terdeteksi:', data);
            return;
        }
        const newData = new dataModel({
            kategori: data.kategori,
            jarak: data.jarak
        });
        await newData.save();
        console.log('Berhasil menyimpan data:', data);
    } catch (error) {
        console.error('Gagal menyimpan data:', error);
    }
});

app.get('/api/getdata', async (req, res) => {
    try {
        const data = await dataModel.find().sort({ timestamp: -1 }).select('-_id');
        res.json(data);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil data' });
    }
});

// Helper function to parse date range
function getDateRange(start, end) {
    const startDate = start ? new Date(start) : new Date();
    const endDate = end ? new Date(end) : new Date();
    return {
        startDate: startDate.toLocaleString("id-ID", { timeZone: "Asia/Jakarta" }).replace(/\//g, '-').replace(/\./g, ':'),
        endDate: endDate.toLocaleString("id-ID", { timeZone: "Asia/Jakarta" }).replace(/\//g, '-').replace(/\./g, ':')
    };
}

// Get counts by date range
app.get('/api/count', async (req, res) => {
    try {
        const { start, end } = req.query;
        const { startDate, endDate } = getDateRange(start, end);

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
                $project: {
                    kategori: "$_id",
                    count: 1,
                    _id: 0
                }
            }
        ]);

        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil statistik' });
    }
});

// Get monthly statistics
app.get('/api/bulan', async (req, res) => {
    try {
        const { year, month } = req.query;
        const startDate = new Date(year, month - 1, 1);
        const endDate = new Date(year, month, 0);
        const { startDate: start, endDate: end } = getDateRange(startDate, endDate);

        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: {
                        $gte: start,
                        $lte: end
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
                $project: {
                    kategori: "$_id",
                    count: 1,
                    _id: 0
                }
            }
        ]);

        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil statistik bulanan' });
    }
});

// Get yearly statistics
app.get('/api/tahun', async (req, res) => {
    try {
        const { year } = req.query;
        const startDate = new Date(year, 0, 1);
        const endDate = new Date(year, 11, 31);
        const { startDate: start, endDate: end } = getDateRange(startDate, endDate);

        const result = await dataModel.aggregate([
            {
                $match: {
                    timestamp: {
                        $gte: start,
                        $lte: end
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
                $project: {
                    kategori: "$_id",
                    count: 1,
                    _id: 0
                }
            }
        ]);

        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Gagal mengambil statistik tahunan' });
    }
});

app.listen(3000, () => {
    console.log('Server is running on port 3000');
});

