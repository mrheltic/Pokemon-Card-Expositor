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
const imagesDir = path.join(__dirname, '..', 'images');
const exportedDir = path.join(imagesDir, 'downloaded');
const rawDir = path.join(imagesDir, 'raw');
const convertedDir = path.join(imagesDir, 'converted');

if (!fs.existsSync(exportedDir)) {
    fs.mkdirSync(exportedDir, { recursive: true });
}
if (!fs.existsSync(imagesDir)) {
    fs.mkdirSync(imagesDir, { recursive: true });
}
if (!fs.existsSync(rawDir)) {
    fs.mkdirSync(rawDir, { recursive: true });
}
if (!fs.existsSync(convertedDir)) {
    fs.mkdirSync(convertedDir, { recursive: true });
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
        const jsonPath = path.join(imagesDir, jsonFileName);

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

                // Handle local file paths or remote URLs
                let imagePath;
                if (imageUrl.startsWith('/home/mrheltic/Documents/PokemonExpositor/')) {
                    // Local file path - copy to exported directory for processing
                    if (fs.existsSync(imageUrl)) {
                        const fileName = path.basename(imageUrl);
                        imagePath = path.join(exportedDir, fileName);
                        fs.copyFileSync(imageUrl, imagePath);
                        console.log(`📁 Copied local file: ${fileName}`);
                    } else {
                        console.log(`⚠️  Local file not found: ${imageUrl}`);
                        continue;
                    }
                } else {
                    // Download image from URL
                    const imageResponse = await axios.get(imageUrl, { responseType: 'arraybuffer' });
                    const imageFileName = `${card.id}_${card.name.replace(/[^a-zA-Z0-9]/g, '_')}.png`;
                    imagePath = path.join(exportedDir, imageFileName);

                    fs.writeFileSync(imagePath, imageResponse.data);
                    console.log(`📥 Downloaded: ${imageFileName}`);
                }

                // Convert using Python script
                const convertedFileName = `${card.id}_${card.name.replace(/[^a-zA-Z0-9]/g, '_')}_converted.png`;
                const convertedPath = path.join(convertedDir, convertedFileName);
                const rawFileName = `${card.id}_${card.name.replace(/[^a-zA-Z0-9]/g, '_')}_1024x600.raw`;
                const rawPath = path.join(rawDir, rawFileName);

                // Extract card metadata for professional overlay
                const cardMetadata = {
                    id: card.id,
                    name: card.name,
                    set: card.set,
                    year: card.year,
                    rarity: card.rarity
                };

                console.log(`🔄 Starting conversion for ${card.name}...`);
                await convertImage(imagePath, convertedPath, rawPath, cardMetadata);
                console.log(`✅ Conversion completed for ${card.name}`);

                convertedFiles.push({
                    cardId: card.id,
                    cardName: card.name,
                    downloadedPath: imagePath,
                    convertedPath: convertedPath,
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
function convertImage(inputPath, convertedPath, rawPath, cardMetadata = null) {
    return new Promise((resolve, reject) => {
        console.log(`🐍 Calling Python script: ${inputPath} -> ${rawPath}`);

        const args = [
            path.join(__dirname, 'pokemon_converter.py'),
            inputPath,
            convertedPath,
            rawPath
        ];

        // Add metadata as JSON string if provided
        if (cardMetadata) {
            args.push(JSON.stringify(cardMetadata));
        }

        console.log(`📋 Python args: ${args.join(' ')}`);

        const pythonProcess = spawn('python3', args, {
            cwd: __dirname
        });

        let stdout = '';
        let stderr = '';

        pythonProcess.stdout.on('data', (data) => {
            stdout += data.toString();
            console.log(`🐍 STDOUT: ${data.toString().trim()}`);
        });

        pythonProcess.stderr.on('data', (data) => {
            stderr += data.toString();
            console.log(`🐍 STDERR: ${data.toString().trim()}`);
        });

        pythonProcess.on('close', (code) => {
            console.log(`🐍 Python process exited with code: ${code}`);
            if (code === 0) {
                resolve(stdout);
            } else {
                reject(new Error(`Python script failed: ${stderr}`));
            }
        });

        pythonProcess.on('error', (error) => {
            console.log(`🐍 Python process error: ${error.message}`);
            reject(error);
        });
    });
}

// Start server
app.listen(PORT, () => {
    console.log(`🚀 Pokemon Card Exporter running at http://localhost:${PORT}`);
    console.log(`📁 Exported files will be saved to: ${imagesDir}`);
});
