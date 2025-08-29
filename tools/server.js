/**
 * Pokemon Card Expositor - Web Server Backend
 * 
 * This Express.js server provides a web API for the Pokemon card conversion
 * system, enabling browser-based interaction with the Python processing pipeline.
 * It handles card searches, batch exports, and real-time conversion monitoring.
 * 
 * 
 * API Endpoints:
 * - GET /api/search - Search Pokemon cards via TCG API
 * - POST /api/export - Batch export and convert cards
 * - POST /api/convert - Convert single card by ID
 * - GET /api/status/:id - Check conversion status
 * - GET /images/* - Serve processed images
 * 
 * Author: mrheltic
 * Date: August 2025
 */

const express = require('express');
const axios = require('axios');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const cors = require('cors');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware setup
app.use(cors());
app.use(express.json({ limit: '50mb' })); // Support large JSON payloads
app.use(express.static(path.join(__dirname)));

// Directory configuration for image processing pipeline
const imagesDir = path.join(__dirname, '..', 'images');
const exportedDir = path.join(imagesDir, 'downloaded');
const rawDir = path.join(imagesDir, 'raw');
const convertedDir = path.join(imagesDir, 'converted');

/**
 * Ensure required directories exist for the processing pipeline
 * Creates the complete directory structure needed for image processing
 * 
 * @param {string[]} dirs - Array of directory paths to create
 */
function ensureDirs(dirs) {
    dirs.forEach(dir => {
        try {
            if (!fs.existsSync(dir)) {
                fs.mkdirSync(dir, { recursive: true });
                console.log(`📁 Created directory: ${path.relative(process.cwd(), dir)}`);
            }
        } catch (err) {
            console.error(`❌ Failed to create directory ${dir}:`, err.message);
        }
    });
}

// Initialize directory structure
ensureDirs([imagesDir, exportedDir, rawDir, convertedDir]);

/**
 * Serve the main web interface
 * GET / - Returns the main HTML page for the web interface
 */
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

/**
 * Search Pokemon cards using the official Pokemon TCG API
 * GET /api/search?q=<query> - Search for cards matching the query
 * 
 * Query examples:
 * - name:pikachu
 * - set.name:"Base Set"
 * - types:electric
 * - id:sv10-193
 */
app.get('/api/search', async (req, res) => {
    try {
        const { q } = req.query;
        
        if (!q) {
            return res.status(400).json({ error: 'Query parameter "q" is required' });
        }
        
        console.log(`🔍 Searching Pokemon cards: ${q}`);
        
        // Query the Pokemon TCG API
        const response = await axios.get(`https://api.pokemontcg.io/v2/cards?q=${encodeURIComponent(q)}`, {
            timeout: 10000,
            headers: {
                'User-Agent': 'Pokemon-Expositor/1.0'
            }
        });
        
        console.log(`📊 Found ${response.data.data.length} cards`);
        res.json(response.data);
        
    } catch (error) {
        console.error('❌ Search API error:', error.message);
        res.status(500).json({ 
            error: 'Failed to search cards',
            details: error.message 
        });
    }
});

/**
 * Convert single Pokemon card by ID
 * POST /api/convert - Convert a single card using Pokemon TCG API
 * 
 * Request body: { cardId: "sv10-193" }
 * Response: { success, convertedImage, rawFile, metadata }
 */
app.post('/api/convert', async (req, res) => {
    try {
        const { cardId } = req.body;
        
        if (!cardId) {
            return res.status(400).json({ error: 'cardId is required' });
        }
        
        console.log(`🎴 Converting card: ${cardId}`);
        
        // Use Python script to handle the complete conversion pipeline
        const convertedPath = path.join(convertedDir, `${cardId}_converted.png`);
        const rawPath = path.join(rawDir, `${cardId}_1024x600.raw`);
        
        // Execute conversion using pokemon_converter.py
        const result = await executeConversion(cardId, convertedPath, rawPath);
        
        if (result.success) {
            res.json({
                success: true,
                convertedImage: `/images/converted/${path.basename(convertedPath)}`,
                rawFile: `/images/raw/${path.basename(rawPath)}`,
                metadata: result.metadata
            });
        } else {
            res.status(500).json({
                success: false,
                error: result.error
            });
        }
        
    } catch (error) {
        console.error('❌ Conversion error:', error.message);
        res.status(500).json({
            success: false,
            error: 'Conversion failed',
            details: error.message
        });
    }
});

/**
 * Batch export and convert multiple Pokemon cards
 * POST /api/export - Process multiple cards from array
 * 
 * Request body: { cards: [{ id, name, image, ... }, ...] }
 * Response: { success, exportedCards, convertedImages, jsonFile, convertedFiles }
 */
app.post('/api/export', async (req, res) => {
    try {
        const { cards } = req.body;

        if (!cards || !Array.isArray(cards) || cards.length === 0) {
            return res.status(400).json({ error: 'Cards array is required and must not be empty' });
        }

        console.log(`📦 Starting batch export of ${cards.length} cards...`);

        // Save card metadata to JSON file for record keeping
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        const jsonFileName = `exported_cards_${timestamp}.json`;
        const jsonPath = path.join(imagesDir, jsonFileName);

        fs.writeFileSync(jsonPath, JSON.stringify(cards, null, 2));
        console.log(`💾 Saved card data to: ${jsonFileName}`);

        // Process each card through the conversion pipeline
        const convertedFiles = [];
        const errors = [];
        
        for (let i = 0; i < cards.length; i++) {
            const card = cards[i];
            
            try {
                console.log(`🔄 Processing card ${i + 1}/${cards.length}: ${card.name}`);
                
                const result = await processCardForExport(card);
                if (result.success) {
                    convertedFiles.push(result.data);
                    console.log(`✅ Completed: ${card.name}`);
                } else {
                    errors.push({ card: card.name, error: result.error });
                    console.log(`❌ Failed: ${card.name} - ${result.error}`);
                }
                
            } catch (error) {
                errors.push({ card: card.name, error: error.message });
                console.error(`❌ Error processing card ${card.name}:`, error.message);
            }
        }

        // Prepare response with detailed results
        const result = {
            success: true,
            exportedCards: cards.length,
            convertedImages: convertedFiles.length,
            errors: errors.length,
            jsonFile: jsonFileName,
            convertedFiles: convertedFiles,
            errorDetails: errors
        };

        console.log(`📊 Export complete! ${convertedFiles.length}/${cards.length} successful`);
        res.json(result);

    } catch (error) {
        console.error('❌ Export error:', error.message);
        res.status(500).json({ 
            error: 'Export failed',
            details: error.message 
        });
    }
});

/**
 * Process a single card for export workflow
 * Handles both direct image URLs and Pokemon TCG API card IDs
 * 
 * @param {Object} card - Card object with id, name, image properties
 * @returns {Object} - Result object with success status and data
 */
async function processCardForExport(card) {
    try {
        const safeCardName = card.name.replace(/[^a-zA-Z0-9]/g, '_');
        let imagePath;
        
        // Handle different image source types
        if (card.image && card.image.startsWith('http')) {
            // Download image from URL
            const imageResponse = await axios.get(card.image, { 
                responseType: 'arraybuffer',
                timeout: 30000
            });
            
            const imageFileName = `${card.id}_${safeCardName}.png`;
            imagePath = path.join(exportedDir, imageFileName);
            
            fs.writeFileSync(imagePath, imageResponse.data);
            console.log(`📥 Downloaded: ${imageFileName}`);
            
        } else if (card.image && card.image.startsWith('/')) {
            // Handle local file path
            if (fs.existsSync(card.image)) {
                const fileName = path.basename(card.image);
                imagePath = path.join(exportedDir, fileName);
                fs.copyFileSync(card.image, imagePath);
                console.log(`📋 Copied local file: ${fileName}`);
            } else {
                throw new Error(`Local file not found: ${card.image}`);
            }
        } else {
            // Use card ID for Pokemon TCG API download (handled by Python script)
            imagePath = card.id;
        }

        // Define output paths
        const convertedFileName = `${card.id}_${safeCardName}_converted.png`;
        const convertedPath = path.join(convertedDir, convertedFileName);
        const rawFileName = `${card.id}_${safeCardName}_1024x600.raw`;
        const rawPath = path.join(rawDir, rawFileName);

        // Prepare card metadata for text overlay
        const cardMetadata = {
            id: card.id,
            name: card.name,
            set_name: card.set || '',
            release_date: card.year || '',
            rarity: card.rarity || ''
        };

        // Execute conversion using Python pipeline
        await convertImageWithMetadata(imagePath, convertedPath, rawPath, cardMetadata);

        return {
            success: true,
            data: {
                cardId: card.id,
                cardName: card.name,
                downloadedPath: imagePath,
                convertedPath: convertedPath,
                rawPath: rawPath
            }
        };
        
    } catch (error) {
        return {
            success: false,
            error: error.message
        };
    }
}

/**
 * Execute Python conversion script with comprehensive error handling
 * Converts images through the complete processing pipeline
 * 
 * @param {string} cardIdOrPath - Pokemon card ID or local image path
 * @param {string} convertedPath - Output PNG path
 * @param {string} rawPath - Output RGB565 binary path
 * @returns {Object} - Conversion result with success status
 */
async function executeConversion(cardIdOrPath, convertedPath, rawPath) {
    return new Promise((resolve) => {
        console.log(`🐍 Starting Python conversion: ${cardIdOrPath}`);

        const args = [
            path.join(__dirname, 'pokemon_converter.py'),
            cardIdOrPath
        ];

        // Add output paths if provided
        if (convertedPath && convertedPath !== 'None') {
            args.push(convertedPath);
        }
        if (rawPath && rawPath !== 'None') {
            args.push(rawPath);
        }

        console.log(`📋 Python command: python3 ${args.join(' ')}`);

        const pythonProcess = spawn('python3', args, {
            cwd: __dirname,
            stdio: ['pipe', 'pipe', 'pipe']
        });

        let stdout = '';
        let stderr = '';

        pythonProcess.stdout.on('data', (data) => {
            const output = data.toString().trim();
            if (output) {
                stdout += output + '\n';
                console.log(`🐍 ${output}`);
            }
        });

        pythonProcess.stderr.on('data', (data) => {
            const output = data.toString().trim();
            if (output) {
                stderr += output + '\n';
                console.log(`🐍 ERR: ${output}`);
            }
        });

        pythonProcess.on('close', (code) => {
            console.log(`🐍 Python process completed with exit code: ${code}`);
            
            if (code === 0) {
                resolve({
                    success: true,
                    stdout: stdout,
                    metadata: null // Could parse metadata from stdout if needed
                });
            } else {
                resolve({
                    success: false,
                    error: stderr || `Python script failed with exit code ${code}`
                });
            }
        });

        pythonProcess.on('error', (error) => {
            console.log(`🐍 Python process error: ${error.message}`);
            resolve({
                success: false,
                error: `Failed to start Python process: ${error.message}`
            });
        });
    });
}

/**
 * Convert image with metadata overlay using Python pipeline
 * Wrapper function for conversion with metadata support
 * 
 * @param {string} inputPath - Input image path or card ID
 * @param {string} convertedPath - Output PNG path
 * @param {string} rawPath - Output RGB565 binary path
 * @param {Object} cardMetadata - Card metadata for text overlay
 * @returns {Promise} - Resolves when conversion completes
 */
function convertImageWithMetadata(inputPath, convertedPath, rawPath, cardMetadata = null) {
    return new Promise((resolve, reject) => {
        console.log(`🔄 Converting with metadata: ${inputPath}`);

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

        const pythonProcess = spawn('python3', args, {
            cwd: __dirname
        });

        let stdout = '';
        let stderr = '';

        pythonProcess.stdout.on('data', (data) => {
            const output = data.toString().trim();
            if (output) {
                stdout += output + '\n';
                console.log(`🐍 ${output}`);
            }
        });

        pythonProcess.stderr.on('data', (data) => {
            const output = data.toString().trim();
            if (output) {
                stderr += output + '\n';
                console.log(`🐍 ERR: ${output}`);
            }
        });

        pythonProcess.on('close', (code) => {
            if (code === 0) {
                resolve(stdout);
            } else {
                reject(new Error(`Python conversion failed: ${stderr || `Exit code ${code}`}`));
            }
        });

        pythonProcess.on('error', (error) => {
            reject(new Error(`Python process error: ${error.message}`));
        });
    });
}

/**
 * Serve static image files
 * GET /images/* - Serve processed images from the images directory
 */
app.use('/images', express.static(imagesDir));

/**
 * Health check endpoint
 * GET /api/health - Check server status and dependencies
 */
app.get('/api/health', (req, res) => {
    const health = {
        status: 'healthy',
        timestamp: new Date().toISOString(),
        directories: {
            images: fs.existsSync(imagesDir),
            downloaded: fs.existsSync(exportedDir),
            converted: fs.existsSync(convertedDir),
            raw: fs.existsSync(rawDir)
        }
    };
    
    res.json(health);
});

/**
 * Error handling middleware
 */
app.use((error, req, res, next) => {
    console.error('❌ Server error:', error.message);
    res.status(500).json({
        error: 'Internal server error',
        details: process.env.NODE_ENV === 'development' ? error.message : undefined
    });
});

/**
 * Start the Express server
 */
app.listen(PORT, () => {
    console.log(`🚀 Pokemon Card Expositor Server running at http://localhost:${PORT}`);
    console.log(`📁 Image directory: ${path.relative(process.cwd(), imagesDir)}`);
    console.log(`🌐 Web interface: http://localhost:${PORT}`);
    console.log(`📊 API health check: http://localhost:${PORT}/api/health`);
});
