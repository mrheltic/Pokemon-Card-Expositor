const express = require('express');
const axios = require('axios');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const cors = require('cors');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname)));

// Ensure exported directory exists
const exportedDir = path.join(__dirname, '..', 'sample_images', 'exported');
const imagesDir = path.join(exportedDir, 'images');
const rawDir = path.join(exportedDir, 'raw');

if (!fs.existsSync(exportedDir)) {
    fs.mkdirSync(exportedDir, { recursive: true });
}
if (!fs.existsSync(imagesDir)) {
    fs.mkdirSync(imagesDir, { recursive: true });
}
if (!fs.existsSync(rawDir)) {
    fs.mkdirSync(rawDir, { recursive: true });
}

// Serve the main HTML file
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// API endpoint to search Pokemon cards
app.get('/api/search', async (req, res) => {
    try {
        const { q } = req.query;
        const response = await axios.get(`https://api.pokemontcg.io/v2/cards?q=${q}`);
        res.json(response.data);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// API endpoint to export cards
app.post('/api/export', async (req, res) => {
    try {
        const { cards } = req.body;

        if (!cards || cards.length === 0) {
            return res.status(400).json({ error: 'No cards to export' });
        }

        console.log(`📤 Exporting ${cards.length} cards...`);

        // Save cards data to JSON
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        const jsonFileName = `exported_cards_${timestamp}.json`;
        const jsonPath = path.join(exportedDir, jsonFileName);

        fs.writeFileSync(jsonPath, JSON.stringify(cards, null, 2));
        console.log(`💾 Saved card data to: ${jsonFileName}`);

        // Download and convert images
        const convertedFiles = [];
        for (let i = 0; i < cards.length; i++) {
            const card = cards[i];
            try {
                const imageUrl = card.imageHD || card.image;
                if (!imageUrl) {
                    console.log(`⚠️  No image URL for card: ${card.name}`);
                    continue;
                }

                // Download image
                const imageResponse = await axios.get(imageUrl, { responseType: 'arraybuffer' });
                const imageFileName = `${card.id}_${card.name.replace(/[^a-zA-Z0-9]/g, '_')}.png`;
                const imagePath = path.join(imagesDir, imageFileName);

                fs.writeFileSync(imagePath, imageResponse.data);
                console.log(`📥 Downloaded: ${imageFileName}`);

                // Convert using Python script
                const rawFileName = `${card.id}_${card.name.replace(/[^a-zA-Z0-9]/g, '_')}_1024x600.raw`;
                const rawPath = path.join(rawDir, rawFileName);

                await convertImage(imagePath, rawPath);
                convertedFiles.push({
                    cardId: card.id,
                    cardName: card.name,
                    imagePath: imagePath,
                    rawPath: rawPath
                });

            } catch (error) {
                console.error(`❌ Error processing card ${card.name}:`, error.message);
            }
        }

        const result = {
            success: true,
            exportedCards: cards.length,
            convertedImages: convertedFiles.length,
            jsonFile: jsonFileName,
            convertedFiles: convertedFiles
        };

        console.log(`✅ Export complete! Processed ${convertedFiles.length}/${cards.length} cards`);
        res.json(result);

    } catch (error) {
        console.error('Export error:', error);
        res.status(500).json({ error: error.message });
    }
});

// Function to convert image using Python script
function convertImage(inputPath, outputPath) {
    return new Promise((resolve, reject) => {
        const pythonProcess = spawn('python3', [
            path.join(__dirname, 'pokemon_converter.py'),
            inputPath
        ], {
            cwd: __dirname
        });

        let stdout = '';
        let stderr = '';

        pythonProcess.stdout.on('data', (data) => {
            stdout += data.toString();
        });

        pythonProcess.stderr.on('data', (data) => {
            stderr += data.toString();
        });

        pythonProcess.on('close', (code) => {
            if (code === 0) {
                // Move the generated .raw file to the desired location
                const generatedRawPath = inputPath.replace(/\.[^.]+$/, '_1024x600.raw');
                if (fs.existsSync(generatedRawPath)) {
                    fs.renameSync(generatedRawPath, outputPath);
                }
                resolve(stdout);
            } else {
                reject(new Error(`Python script failed: ${stderr}`));
            }
        });

        pythonProcess.on('error', (error) => {
            reject(error);
        });
    });
}

// Start server
app.listen(PORT, () => {
    console.log(`🚀 Pokemon Card Exporter running at http://localhost:${PORT}`);
    console.log(`📁 Exported files will be saved to: ${exportedDir}`);
});
