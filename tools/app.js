/**
 * Pokemon Card Expositor - Frontend JavaScript Application
 * 
 * This script provides the interactive frontend for the Pokemon card conversion
 * system. It enables users to search for Pokemon cards, manage collections,
 * and trigger batch conversions through an intuitive web interface.
 * 
 * API Integration:
 * - GET /api/search - Search Pokemon cards
 * - POST /api/convert - Convert single card
 * - POST /api/export - Batch export cards
 * - GET /api/health - System health check
 * 
 * Author: mrheltic
 * Date: August 2025
 */

document.addEventListener('DOMContentLoaded', () => {
    // Small helper to reduce repetition when getting DOM elements
    const $ = id => document.getElementById(id);
    // DOM Elements (compact mapping)
    const {
        searchButton,
        searchTerm: searchTermInput,
        searchType: searchTypeSelect,
        pokemonType: pokemonTypeSelect,
        yearRange: yearRangeSelect,
        sortBy: sortBySelect,
        sortDirection: sortDirectionBtn,
        sortIcon,
        results: resultsDiv,
        resultsSection,
        resultsCount,
        loading: loadingDiv,
        savedCards,
        savedCount: savedCountSpan,
        clearSaved: clearSavedBtn,
        themeToggle: themeToggleBtn,
        exportBtn,
        exportSavedBtn,
        addAllBtn
    } = {
        searchButton: $('searchButton'),
        searchTerm: $('searchTerm'),
        searchType: $('searchType'),
        pokemonType: $('pokemonType'),
        yearRange: $('yearRange'),
        sortBy: $('sortBy'),
        sortDirection: $('sortDirection'),
        sortIcon: $('sortIcon'),
        results: $('results'),
        resultsSection: $('resultsSection'),
        resultsCount: $('resultsCount'),
        loading: $('loading'),
        savedCards: $('savedCards'),
        savedCount: $('savedCount'),
        clearSaved: $('clearSaved'),
        themeToggle: $('themeToggle'),
        exportBtn: $('exportBtn'),
        exportSavedBtn: $('exportSavedBtn'),
        addAllBtn: $('addAllBtn')
    };
    const tabButtons = document.querySelectorAll('.tab-btn');
    const searchTab = $('searchTab');
    const savedListTab = $('savedList');
    
    // Modals
    const imageModal = document.getElementById('imageModal');
    const detailsModal = document.getElementById('detailsModal');
    const highResImage = document.getElementById('highResImage');
    const cardDetails = document.getElementById('cardDetails');
    const closeModalBtns = document.querySelectorAll('.close-modal');
    
    // Application state
    let currentResults = [];
    let savedCardsList = loadSavedCards();
    let sortDirection = 'asc';
    let activeTab = 'search';
    
    // Initialize theme preference
    initializeTheme();
    
    // Event listeners
    searchButton.addEventListener('click', startSearch);
    searchTermInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') startSearch();
    });
    
    sortDirectionBtn.addEventListener('click', toggleSortDirection);
    sortBySelect.addEventListener('change', sortResults);
    
    themeToggleBtn.addEventListener('click', toggleTheme);
    
    // Export buttons
    exportBtn.addEventListener('click', (e) => exportCards(currentResults, e.target));
    exportSavedBtn.addEventListener('click', (e) => exportCards(savedCardsList, e.target));
    addAllBtn.addEventListener('click', addAllToSaved);
    
    closeModalBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            imageModal.style.display = 'none';
            detailsModal.style.display = 'none';
        });
    });
    
    clearSavedBtn.addEventListener('click', clearSavedCards);
    
    // Tab navigation
    tabButtons.forEach(button => {
        button.addEventListener('click', () => {
            const tabName = button.getAttribute('data-tab');
            activeTab = tabName;
            
            tabButtons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
            
            if (tabName === 'search') {
                searchTab.style.display = 'block';
                savedListTab.style.display = 'none';
            } else {
                searchTab.style.display = 'none';
                savedListTab.style.display = 'block';
                renderSavedCards();
            }
        });
    });
    
    // Close modals when clicking outside the content
    window.addEventListener('click', (e) => {
        if (e.target === imageModal) {
            imageModal.style.display = 'none';
        }
        if (e.target === detailsModal) {
            detailsModal.style.display = 'none';
        }
    });
    
    // Initial saved cards count
    updateSavedCount();
    
    // Theme functions
    function initializeTheme() {
        const savedTheme = localStorage.getItem('pokemonCardTheme') || 'light';
        document.documentElement.setAttribute('data-theme', savedTheme);
        themeToggleBtn.textContent = savedTheme === 'light' ? '☀️' : '🌙';
    }
    
    function toggleTheme() {
        const currentTheme = document.documentElement.getAttribute('data-theme');
        const newTheme = currentTheme === 'light' ? 'dark' : 'light';
        
        document.documentElement.setAttribute('data-theme', newTheme);
        localStorage.setItem('pokemonCardTheme', newTheme);
        themeToggleBtn.textContent = newTheme === 'light' ? '☀️' : '🌙';
    }

    async function startSearch() {
        const searchTerm = searchTermInput.value.trim();
        const searchType = searchTypeSelect.value;
        const pokemonType = pokemonTypeSelect.value;
        const yearRange = yearRangeSelect.value;
        
        if (!searchTerm && !pokemonType && !yearRange) {
            alert('Please enter at least one search criterion');
            return;
        }
        
        // Show loading indicator
        loadingDiv.style.display = 'block';
        resultsDiv.innerHTML = '';
        resultsSection.style.display = 'block';
        
        try {
            currentResults = await searchCards(searchTerm, searchType, pokemonType, yearRange);
            sortResults();
        } catch (error) {
            console.error('Search error:', error);
            resultsDiv.innerHTML = `<p class="empty-message">An error occurred: ${error.message}</p>`;
            resultsCount.textContent = 'Error occurred during search';
        } finally {
            loadingDiv.style.display = 'none';
        }
    }

    async function searchCards(searchTerm, searchType, pokemonType, yearRange) {
        // Build query for server API
        let queryParams = [];
        
        // Build query based on search type and term
        if (searchTerm) {
            switch (searchType) {
                case 'pokemon':
                    queryParams.push(`name:"${searchTerm}"`);
                    break;
                case 'artist':
                    queryParams.push(`artist:"${searchTerm}"`);
                    break;
                case 'set':
                    queryParams.push(`set.name:"${searchTerm}"`);
                    break;
                case 'rarity':
                    queryParams.push(`rarity:"${searchTerm}"`);
                    break;
                default:
                    queryParams.push(`name:"${searchTerm}"`);
            }
        }
        
        // Add Pokémon type filter
        if (pokemonType) {
            queryParams.push(`types:${pokemonType}`);
        }
        
        // Add year range filter
        if (yearRange) {
            const [minYear, maxYear] = yearRange.split('-');
            if (maxYear) {
                queryParams.push(`set.releaseDate:[${ minYear }-01-01 TO ${ maxYear }-12-31]`);
            } else if (yearRange.startsWith('-')) {
                // Before a specific year
                const year = yearRange.substring(1);
                queryParams.push(`set.releaseDate:[* TO ${ year }-12-31]`);
            } else {
                // After a specific year
                queryParams.push(`set.releaseDate:[${ minYear }-01-01 TO *]`);
            }
        }
        
        // Construct query string
        const query = queryParams.join(' ');
        
        // Make API request to our server
        const response = await fetch(`/api/search?q=${encodeURIComponent(query)}`);
        
        if (!response.ok) {
            throw new Error(`API request failed: ${response.status}`);
        }
        
        const data = await response.json();
        
        // Extract required information from each card
        return data.data.map(card => {
            // Extract year from release date (format "YYYY/MM/DD")
            const year = card.set.releaseDate ? card.set.releaseDate.split('/')[0] : 'N/A';
            
            // Get market price
            let marketPrice = 'N/A';
            if (card.tcgplayer && card.tcgplayer.prices) {
                const priceTypes = Object.keys(card.tcgplayer.prices);
                for (const type of priceTypes) {
                    if (card.tcgplayer.prices[type].market) {
                        marketPrice = card.tcgplayer.prices[type].market;
                        break;
                    }
                }
            }
            
            return {
                id: card.id,
                name: card.name,
                set: card.set.name,
                year: year,
                rarity: card.rarity || 'N/A',
                marketPrice: marketPrice,
                artist: card.artist || 'N/A',
                image: card.images?.small,
                imageHD: card.images?.large,
                types: card.types || [],
                // Additional data to store for details view
                fullData: {
                    supertype: card.supertype,
                    subtypes: card.subtypes || [],
                    hp: card.hp || 'N/A',
                    rules: card.rules || [],
                    attacks: card.attacks || [],
                    weaknesses: card.weaknesses || [],
                    retreatCost: card.convertedRetreatCost || 'N/A',
                    set: {
                        name: card.set.name,
                        series: card.set.series,
                        printedTotal: card.set.printedTotal,
                        total: card.set.total
                    }
                }
            };
        });
    }

    function sortResults() {
        const sortBy = sortBySelect.value;
        
        if (currentResults.length === 0) return;
        
        currentResults.sort((a, b) => {
            let valueA, valueB;
            
            switch (sortBy) {
                case 'price':
                    valueA = typeof a.marketPrice === 'number' ? a.marketPrice : -1;
                    valueB = typeof b.marketPrice === 'number' ? b.marketPrice : -1;
                    break;
                case 'year':
                    valueA = a.year !== 'N/A' ? parseInt(a.year) : 0;
                    valueB = b.year !== 'N/A' ? parseInt(b.year) : 0;
                    break;
                default: // name
                    valueA = a.name;
                    valueB = b.name;
            }
            
            if (sortDirection === 'asc') {
                return valueA > valueB ? 1 : -1;
            } else {
                return valueA < valueB ? 1 : -1;
            }
        });
        
        renderResults();
    }
    
    function toggleSortDirection() {
        sortDirection = sortDirection === 'asc' ? 'desc' : 'asc';
        sortIcon.textContent = sortDirection === 'asc' ? '↑' : '↓';
        sortDirectionBtn.setAttribute('data-direction', sortDirection);
        sortResults();
    }

    function renderResults() {
        resultsDiv.innerHTML = '';
        
        if (currentResults.length === 0) {
            resultsCount.textContent = 'No cards found';
            return;
        }
        
        resultsCount.textContent = `Found ${currentResults.length} cards`;
        
        currentResults.forEach(card => {
            const cardDiv = document.createElement('div');
            cardDiv.className = 'card-mini';
            cardDiv.setAttribute('data-card-id', card.id);
            
            // Format price display
            const priceDisplay = typeof card.marketPrice === 'number' 
                ? `$${card.marketPrice.toFixed(2)}`
                : card.marketPrice;
            
            // Check if card is saved
            const isSaved = isCardSaved(card.id);
            
            // Create card HTML
            cardDiv.innerHTML = `
                ${card.image ? `<img src="${card.image}" alt="${card.name}">` : ''}
                ${isSaved ? `<div class="saved-indicator">+</div>` : ''}
                <h3>${card.name}</h3>
                <p>${card.set} (${card.year})</p>
                <p>${priceDisplay}</p>
            `;
            
            // Add event listeners
            cardDiv.addEventListener('click', (e) => handleCardClick(e, card));
            
            resultsDiv.appendChild(cardDiv);
        });
    }
    
    function handleCardClick(e, card) {
        const cardElement = e.currentTarget;
        const isTargetImg = e.target.tagName.toLowerCase() === 'img';
        
        // If clicking on the image, show high-res image
        if (isTargetImg) {
            showHighResImage(card.imageHD, card.name);
            return;
        }
        
        // Otherwise toggle save state
        const isSaved = isCardSaved(card.id);
        
        if (isSaved) {
            removeFromSaved(card.id);
            // Remove saved indicator
            const indicator = cardElement.querySelector('.saved-indicator');
            if (indicator) {
                indicator.remove();
            }
        } else {
            addToSaved(card);
            // Add saved indicator if it doesn't exist
            if (!cardElement.querySelector('.saved-indicator')) {
                const indicator = document.createElement('div');
                indicator.className = 'saved-indicator';
                indicator.textContent = '+';
                cardElement.appendChild(indicator);
            }
        }
    }

    function showHighResImage(imageUrl, cardName) {
        if (!imageUrl) return;
        
        highResImage.src = imageUrl;
        highResImage.alt = cardName;
        imageModal.style.display = 'block';
    }
    
    function showCardDetails(card) {
        cardDetails.innerHTML = `
            <h2>${card.name}</h2>
            ${card.imageHD ? `<img src="${card.imageHD}" alt="${card.name}" style="max-width:100%;">` : ''}
            <table>
                <tr>
                    <td><strong>Set:</strong></td>
                    <td>${card.set}</td>
                </tr>
                <tr>
                    <td><strong>Year:</strong></td>
                    <td>${card.year}</td>
                </tr>
                <tr>
                    <td><strong>Rarity:</strong></td>
                    <td>${card.rarity}</td>
                </tr>
                <tr>
                    <td><strong>Artist:</strong></td>
                    <td>${card.artist}</td>
                </tr>
                <tr>
                    <td><strong>Market Price:</strong></td>
                    <td>${typeof card.marketPrice === 'number' ? `$${card.marketPrice.toFixed(2)}` : card.marketPrice}</td>
                </tr>
                <tr>
                    <td><strong>Types:</strong></td>
                    <td>${card.types.join(', ') || 'N/A'}</td>
                </tr>
                <tr>
                    <td><strong>HP:</strong></td>
                    <td>${card.fullData.hp}</td>
                </tr>
                <tr>
                    <td><strong>Subtypes:</strong></td>
                    <td>${card.fullData.subtypes.join(', ') || 'N/A'}</td>
                </tr>
            </table>
            
            ${card.fullData.attacks && card.fullData.attacks.length > 0 ? `
                <h3>Attacks</h3>
                <ul>
                    ${card.fullData.attacks.map(attack => `
                        <li>
                            <strong>${attack.name}</strong> - 
                            Damage: ${attack.damage || 'N/A'} 
                            ${attack.text ? `<br>${attack.text}` : ''}
                        </li>
                    `).join('')}
                </ul>
            ` : ''}
        `;
        
        detailsModal.style.display = 'block';
    }
    
    // Export functionality
    async function exportCards(cards, exportButton) {
        if (!cards || cards.length === 0) {
            alert('No cards to export');
            return;
        }
        
        try {
            // Show loading state
            const originalText = exportButton.textContent;
            exportButton.textContent = 'Exporting...';
            exportButton.disabled = true;
            
            // Send cards to server for processing
            const response = await fetch('/api/export', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ cards })
            });
            
            if (!response.ok) {
                throw new Error(`Export failed: ${response.status}`);
            }
            
            const result = await response.json();
            
            // Show success message
            alert(`✅ Export successful!\n\n📊 Cards exported: ${result.exportedCards}\n🖼️ Images converted: ${result.convertedImages}\n📁 Files saved to: sample_images/exported/`);
            
        } catch (error) {
            console.error('Export error:', error);
            alert(`❌ Export failed: ${error.message}`);
        } finally {
            // Reset button state
            exportButton.textContent = originalText;
            exportButton.disabled = false;
        }
    }
    
    // Saved cards functionality
    function addAllToSaved() {
        if (currentResults.length === 0) {
            alert('No cards to add');
            return;
        }
        
        let addedCount = 0;
        currentResults.forEach(card => {
            if (!isCardSaved(card.id)) {
                savedCardsList.push(card);
                addedCount++;
            }
        });
        
        if (addedCount > 0) {
            saveToBrowser();
            updateSavedCount();
            renderResults(); // Update UI to show saved indicators
            alert(`Added ${addedCount} cards to your list`);
        } else {
            alert('All cards are already in your list');
        }
    }
    
    function addToSaved(card) {
        if (!isCardSaved(card.id)) {
            savedCardsList.push(card);
            saveToBrowser();
            updateSavedCount();
        }
    }
    
    function removeFromSaved(cardId) {
        savedCardsList = savedCardsList.filter(card => card.id !== cardId);
        saveToBrowser();
        updateSavedCount();
        
        // Update UI in both tabs
        if (activeTab === 'saved') {
            renderSavedCards();
        } else {
            // Update saved indicators in search results
            const cardElement = resultsDiv.querySelector(`[data-card-id="${cardId}"]`);
            if (cardElement) {
                const indicator = cardElement.querySelector('.saved-indicator');
                if (indicator) {
                    indicator.remove();
                }
            }
        }
    }
    
    function isCardSaved(cardId) {
        return savedCardsList.some(card => card.id === cardId);
    }
    
    function saveToBrowser() {
        localStorage.setItem('savedPokemonCards', JSON.stringify(savedCardsList));
    }
    
    function loadSavedCards() {
        const saved = localStorage.getItem('savedPokemonCards');
        return saved ? JSON.parse(saved) : [];
    }
    
    function updateSavedCount() {
        savedCountSpan.textContent = `(${savedCardsList.length})`;
    }
    
    function renderSavedCards() {
        savedCards.innerHTML = '';
        
        if (savedCardsList.length === 0) {
            savedCards.innerHTML = '<p class="empty-message">No saved cards yet</p>';
            return;
        }
        
        savedCardsList.forEach(card => {
            const cardDiv = document.createElement('div');
            cardDiv.className = 'card-mini';
            cardDiv.setAttribute('data-card-id', card.id);
            
            const priceDisplay = typeof card.marketPrice === 'number' 
                ? `$${card.marketPrice.toFixed(2)}`
                : card.marketPrice;
            
            cardDiv.innerHTML = `
                ${card.image ? `<img src="${card.image}" alt="${card.name}">` : ''}
                <div class="saved-indicator">+</div>
                <h3>${card.name}</h3>
                <p>${card.set} (${card.year})</p>
                <p>${priceDisplay}</p>
            `;
            
            // Add event listeners
            cardDiv.addEventListener('click', (e) => {
                const isTargetImg = e.target.tagName.toLowerCase() === 'img';
                
                if (isTargetImg) {
                    showHighResImage(card.imageHD, card.name);
                } else {
                    removeFromSaved(card.id);
                }
            });
            
            savedCards.appendChild(cardDiv);
        });
    }
    
    function clearSavedCards() {
        if (confirm('Are you sure you want to clear all saved cards?')) {
            savedCardsList = [];
            saveToBrowser();
            updateSavedCount();
            renderSavedCards();
            
            // Update UI in search results if that tab is active
            if (activeTab === 'search') {
                renderResults();
            }
        }
    }
});