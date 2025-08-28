#!/usr/bin/env node

// Test script per la pipeline di export
const axios = require('axios');

async function testExport() {
    try {
        console.log('🧪 Testing Pokemon Card Export Pipeline...');

        // Prima testiamo la ricerca
        console.log('1. Testing search API...');
        const searchResponse = await axios.get('http://localhost:3000/api/search?q=name:pikachu');
        console.log(`✅ Search returned ${searchResponse.data.data.length} cards`);

        // Poi testiamo l'export con una carta
        if (searchResponse.data.data.length > 0) {
            console.log('2. Testing export API...');
            const testCard = searchResponse.data.data[0];

            // Formatta la carta come fa l'app
            const formattedCard = {
                id: testCard.id,
                name: testCard.name,
                set: testCard.set.name,
                year: testCard.set.releaseDate ? testCard.set.releaseDate.split('/')[0] : 'N/A',
                rarity: testCard.rarity || 'N/A',
                marketPrice: 'N/A', // Non disponibile nella risposta base
                artist: testCard.artist || 'N/A',
                image: testCard.images?.small,
                imageHD: testCard.images?.large,
                types: testCard.types || [],
                fullData: {
                    supertype: testCard.supertype,
                    subtypes: testCard.subtypes || [],
                    hp: testCard.hp || 'N/A',
                    rules: testCard.rules || [],
                    attacks: testCard.attacks || [],
                    weaknesses: testCard.weaknesses || [],
                    retreatCost: testCard.convertedRetreatCost || 'N/A',
                    set: {
                        name: testCard.set.name,
                        series: testCard.set.series,
                        printedTotal: testCard.set.printedTotal,
                        total: testCard.set.total
                    }
                }
            };

            const exportResponse = await axios.post('http://localhost:3000/api/export', {
                cards: [formattedCard]
            });

            console.log('✅ Export successful!');
            console.log(`📊 Cards exported: ${exportResponse.data.exportedCards}`);
            console.log(`🖼️ Images converted: ${exportResponse.data.convertedImages}`);
            console.log(`📁 JSON file: ${exportResponse.data.jsonFile}`);
        }

        console.log('🎉 All tests passed!');

    } catch (error) {
        console.error('❌ Test failed:', error.response?.data || error.message);
    }
}

testExport();
