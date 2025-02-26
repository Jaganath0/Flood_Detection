const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const mongoose = require('mongoose');
const express = require('express');
const cors = require('cors');

// MongoDB Connection
mongoose
  .connect('mongodb://localhost:27017/floodDetection', {
    useNewUrlParser: true,
    useUnifiedTopology: true,
  })
  .then(() => console.log('Connected to MongoDB'))
  .catch((err) => console.error('MongoDB connection error:', err));

// Define Schemas
const waterLevelSchema = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  waterLevel: Number,
});

const alertSchema = new mongoose.Schema({
  timestamp: { type: Date, default: Date.now },
  alertMessage: String,
});

const WaterLevel = mongoose.model('WaterLevel', waterLevelSchema);
const Alert = mongoose.model('Alert', alertSchema);

// Serial Port Setup
const port = new SerialPort({
  path: 'COM11', // Replace with your Arduino's COM port
  baudRate: 9600,
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

parser.on('data', async (data) => {
  console.log(`Received from Arduino: ${data}`);

  try {
    const jsonData = JSON.parse(data); // Parse JSON from Arduino

    if (jsonData.waterLevel !== undefined) {
      // Save water level data
      const newEntry = new WaterLevel({ waterLevel: jsonData.waterLevel });
      await newEntry.save();
      console.log('Saved to MongoDB:', newEntry);
    } else if (jsonData.alert) {
      // Save alert data
      const newAlert = new Alert({ alertMessage: jsonData.alert });
      await newAlert.save();
      console.log('Alert saved to MongoDB:', newAlert);
    }
  } catch (err) {
    console.error('Error processing data:', err.message);
  }
});

// REST API Setup
const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());

// API Endpoints
app.get('/api/water-levels', async (req, res) => {
  try {
    const data = await WaterLevel.find().sort({ timestamp: -1 }).limit(10);
    res.json(data);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/api/alerts', async (req, res) => {
  try {
    const data = await Alert.find().sort({ timestamp: -1 }).limit(10);
    res.json(data);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Start the server
app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
