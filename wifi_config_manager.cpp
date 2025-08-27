#include "wifi_config_manager.h"
#include "system_manager.h"

// Global WiFi Config Manager instance
WiFiConfigManager wifiConfigManager;

// Static file paths - saved on SD card in config folder
const char* WiFiConfigManager::WIFI_CONFIG_FILE = "/config/wifi_config.json";
const char* WiFiConfigManager::SLIDESHOW_CONFIG_FILE = "/config/slideshow_config.json";
const char* WiFiConfigManager::ADVANCED_CONFIG_FILE = "/config/advanced_config.json";
const char* WiFiConfigManager::DISPLAY_CONFIG_FILE = "/config/display_config.json";

// Web interface HTML template - Professional Dark Theme
const char* html_header = R"HTMLEND(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pokemon Card Expositor - Control Panel</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background: #0b1220; /* darker for higher contrast */
            color: #ffffff;
            min-height: 100vh;
            padding: 8px;
            margin: 0;
            overflow-x: hidden;
            -webkit-font-smoothing:antialiased;
            -moz-osx-font-smoothing:grayscale;
        }
        
        .container { 
            max-width: 1100px; 
            margin: 0 auto; 
            background: rgba(18, 22, 30, 0.98);
            border-radius: 12px;
            box-shadow: 0 8px 20px rgba(0,0,0,0.5);
            border: 1px solid rgba(255,255,255,0.06);
            overflow: hidden;
            width: calc(100% - 16px);
        }
        
        .header { 
            background: linear-gradient(90deg, #2b2f45 0%, #172033 100%);
            padding: 16px 12px;
            text-align: center;
            position: relative;
            border-bottom: 1px solid rgba(255,255,255,0.04);
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: url('data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><circle cx="20" cy="20" r="2" fill="rgba(255,255,255,0.1)"/><circle cx="80" cy="80" r="1" fill="rgba(255,255,255,0.1)"/><circle cx="40" cy="70" r="1.5" fill="rgba(255,255,255,0.1)"/></svg>');
        }
        
        .header h1 { 
            font-size: 1.6rem; 
            font-weight: 700; 
            color: #ffffff;
            position: relative;
            z-index: 1;
            margin-bottom: 4px;
            letter-spacing: 0.5px;
        }
        
        .header p { 
            font-size: 1em; 
            opacity: 0.9;
            margin-top: 5px;
            position: relative;
            z-index: 1;
        }
        
        .main-content {
            display: grid;
            grid-template-columns: 1fr;
            gap: 12px;
            padding: 16px 12px;
        }
        
        /* Responsive design */
        @media (min-width: 768px) {
            body { padding: 20px; }
            .container { border-radius: 20px; }
            .header { padding: 30px; }
            .header h1 { font-size: 2.5em; }
            .header p { font-size: 1.2em; margin-top: 10px; }
            .main-content { 
                grid-template-columns: 1fr 1fr; 
                gap: 20px; 
                padding: 30px; 
            }
        }
        
        @media (min-width: 1024px) {
            .main-content { 
                grid-template-columns: repeat(2, 1fr); 
            }
        }
        
        .section { 
            background: rgba(26, 30, 40, 0.95);
            border-radius: 12px;
            padding: 14px;
            border: 1px solid rgba(255,255,255,0.06);
            transition: all 0.18s ease;
            position: relative;
            overflow: hidden;
            box-sizing: border-box;
            width: 100%;
            max-width: 100%;
        }
        
        .section:hover {
            transform: translateY(-1px);
            box-shadow: 0 5px 20px rgba(0,0,0,0.2);
            border-color: rgba(255,255,255,0.2);
        }
        
        @media (min-width: 768px) {
            .section { 
                padding: 25px; 
            }
            .section:hover {
                transform: translateY(-2px);
                box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            }
        }
        
        .section::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, #ff6b6b, #4ecdc4, #45b7d1, #96ceb4);
        }
        
        .section h3 { 
            color: #a8ffd6; 
            margin-bottom: 14px;
            font-size: 1.1rem;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .section h3::before {
            content: '';
            width: 8px;
            height: 8px;
            background: #64ffda;
            border-radius: 50%;
            box-shadow: 0 0 10px #64ffda;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        label { 
            display: block;
            margin-bottom: 8px;
            color: #b8c5d1;
            font-weight: 500;
            font-size: 0.95em;
        }
        
        input, select, textarea { 
            width: 100%; 
            padding: 12px 12px;
            border: 1px solid rgba(255,255,255,0.08);
            border-radius: 8px;
            background: rgba(18, 22, 30, 0.9);
            color: #ffffff;
            font-size: 15px; /* Prevents zoom on iOS */
            transition: all 0.18s ease;
            touch-action: manipulation;
            box-sizing: border-box;
        }
        
        @media (min-width: 768px) {
            input, select, textarea { 
                padding: 12px 16px;
                font-size: 1em;
            }
        }
        
        input:focus, select:focus, textarea:focus { 
            outline: none;
            border-color: #64ffda;
            box-shadow: 0 0 15px rgba(100, 255, 218, 0.3);
            background: rgba(30, 30, 46, 1);
        }
        
        input[type="range"] {
            padding: 0;
            height: 8px;
            background: rgba(255,255,255,0.06);
            border-radius: 4px;
            appearance: none;
            touch-action: manipulation;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: linear-gradient(135deg, #ff6b6b, #4ecdc4);
            cursor: pointer;
            box-shadow: 0 2px 10px rgba(0,0,0,0.3);
        }
        
        .range-value {
            display: inline-block;
            background: #2e3b4e;
            color: #fff;
            padding: 3px 10px;
            border-radius: 12px;
            font-weight: 600;
            margin-left: 8px;
            min-width: 40px;
            text-align: center;
            font-size: 0.9em;
        }

        /* Mobile tweaks to avoid overflow */
        @media (max-width: 480px) {
            .container { padding: 8px; }
            .header h1 { font-size: 1.2rem; }
            .section { padding: 12px; border-radius: 8px; }
            input, select, textarea { padding: 10px; font-size: 14px; }
            .range-value { min-width: 36px; padding: 2px 8px; }
            .tabs { flex-direction: row; overflow-x: auto; }
            .tab { padding: 10px 12px; font-size: 0.95em; }
        }
        
        button { 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white; 
            padding: 15px 20px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            font-size: 16px; /* Prevents zoom on iOS */
            font-weight: 600;
            transition: all 0.3s ease;
            position: relative;
            touch-action: manipulation;
            min-height: 44px; /* iOS recommended touch target */
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        @media (min-width: 768px) {
            button { 
                padding: 12px 24px;
                font-size: 1em;
                min-height: auto;
            }
        }
            overflow: hidden;
        }
        
        button:hover { 
            transform: translateY(-2px);
            box-shadow: 0 10px 25px rgba(102, 126, 234, 0.4);
        }
        
        button:active { 
            transform: translateY(0);
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        
        .btn-success {
            background: linear-gradient(135deg, #4ecdc4 0%, #44a08d 100%);
        }
        
        .btn-warning {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
        }
        
        .btn-danger {
            background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 100%);
        }
        
        .control-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 10px;
            margin-top: 16px;
            justify-items: center; /* make buttons symmetric */
        }

        /* Compact action buttons */
        .btn-compact {
            padding: 6px 10px;
            min-width: 100px;
            font-size: 14px;
        }

        /* Square small icon button (used for show-password) */
        .icon-btn {
            width: 32px;
            height: 32px;
            padding: 0;
            line-height: 32px;
            text-align: center;
            border-radius: 6px;
            font-size: 16px;
        }

        /* Vertical control column for Start/Pause/Stop to be symmetric */
        .vertical-controls {
            display:flex; flex-direction:column; gap:8px; width:100%; max-width:180px; align-items:center;
        }
        
        @media (min-width: 768px) {
            .control-grid {
                grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
                gap: 15px;
            }
        }
        
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
        }
        
        @media (min-width: 768px) {
            .status-grid {
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 15px;
            }
        }
        
        .status-card {
            background: rgba(30, 30, 46, 0.8);
            border-radius: 10px;
            padding: 12px;
            border: 1px solid rgba(255,255,255,0.1);
            text-align: center;
        }

        /* Storage usage progress bar */
        .storage-progress {
            background: rgba(255,255,255,0.06);
            border-radius: 8px;
            height: 12px;
            width: 100%;
            overflow: hidden;
            margin-top: 8px;
            border: 1px solid rgba(255,255,255,0.04);
        }

        .storage-progress .progress-bar {
            height: 100%;
            background: linear-gradient(90deg,#4ecdc4,#44a08d);
            width: 0%;
            transition: width 300ms ease;
        }
        
        @media (min-width: 768px) {
            .status-card {
                padding: 15px;
            }
        }
        
        .status-value {
            font-size: 1.3em;
            font-weight: bold;
            color: #64ffda;
            margin-bottom: 5px;
        }
        
        @media (min-width: 768px) {
            .status-value {
                font-size: 1.5em;
            }
        }
        
        .status-label {
            font-size: 0.9em;
            color: #b8c5d1;
            opacity: 0.8;
        }
        
        .alert { 
            padding: 15px 20px;
            margin: 15px 0;
            border-radius: 10px;
            border-left: 4px solid;
            font-weight: 500;
        }
        
        .alert-success { 
            background: rgba(76, 175, 80, 0.1);
            color: #4caf50;
            border-left-color: #4caf50;
        }
        
        .alert-error { 
            background: rgba(244, 67, 54, 0.1);
            color: #f44336;
            border-left-color: #f44336;
        }
        
        .alert-info { 
            background: rgba(33, 150, 243, 0.1);
            color: #2196f3;
            border-left-color: #2196f3;
        }
        
        .loading {
            opacity: 0.6;
            pointer-events: none;
        }
        
        .loading::after {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 20px;
            height: 20px;
            margin: -10px 0 0 -10px;
            border: 2px solid transparent;
            border-top: 2px solid #64ffda;
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .tab-container {
            grid-column: 1 / -1;
        }
        
        .tabs {
            display: flex;
            margin-bottom: 20px;
            background: rgba(30, 30, 46, 0.8);
            border-radius: 10px;
            padding: 5px;
        }
        
        .tab {
            flex: 1;
            padding: 12px 20px;
            text-align: center;
            cursor: pointer;
            border-radius: 8px;
            transition: all 0.3s ease;
            color: #b8c5d1;
        }

    /* ensure buttons don't overflow on small screens */
    button { max-width: 220px; width: 100%; }
        
        .tab.active {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        
        .tab-content {
            display: none;
            animation: fadeIn 0.3s ease;
        }
        
        .tab-content.active {
            display: block;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎴 Pokemon Card Expositor</h1>
            <p>Professional Display Control System</p>
        </div>
        <div class="main-content">
)HTMLEND";

const char* html_wifi_form = R"HTMLEND(
            <div class="section">
                <h3>🌐 Network Configuration</h3>
                <form id="wifiForm">
                    <div class="form-group">
                        <label for="ssid">WiFi Network (SSID):</label>
                        <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi network name" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="password">WiFi Password:</label>
                        <div style="display:flex; gap:8px; align-items:center;">
                            <input type="password" id="password" name="password" placeholder="Enter WiFi password" style="flex:1;">
                            <button type="button" onclick="togglePasswordVisibility()" title="Show/Hide password" class="icon-btn">👁️</button>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="deviceName">Device Name:</label>
                        <input type="text" id="deviceName" name="deviceName" placeholder="Pokemon-Expositor" value="Pokemon-Expositor">
                    </div>
                    
                    <div class="form-group">
                        <label for="enableAP">Enable Access Point Mode:</label>
                        <select id="enableAP" name="enableAP">
                            <option value="true">Yes - Create hotspot for configuration</option>
                            <option value="false">No - WiFi client mode only</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="apPassword">Access Point Password:</label>
                        <input type="password" id="apPassword" name="apPassword" placeholder="Minimum 8 characters" value="pokemon123">
                    </div>
                    
                    <div class="form-group">
                        <label for="hostname">Network Hostname:</label>
                        <input type="text" id="hostname" name="hostname" placeholder="pokemon-expositor" value="pokemon-expositor">
                    </div>
                    
                    <button type="button" class="btn-primary" onclick="saveWiFiConfig()">💾 Save Network Settings</button>
                </form>
            </div>
)HTMLEND";

const char* html_slideshow_form = R"HTMLEND(
            <div class="section">
                <h3>🎬 Slideshow Configuration</h3>
                <form id="slideshowForm">
                    <div class="form-group">
                        <label for="enabled">Enable Slideshow:</label>
                        <select id="enabled" name="enabled">
                            <option value="true">✅ Enabled - Auto display images</option>
                            <option value="false">❌ Disabled - Manual control only</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="interval">Display Interval:</label>
                        <input type="range" id="interval" name="interval" min="0" max="100" value="20" oninput="updateIntervalValue(this.value)">
                        <span class="range-value" id="intervalValue">10s</span>
                        <div style="margin-top:8px; display:flex; gap:8px; flex-wrap:wrap;">
                            <!-- Quick important steps -->
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(1)">1s</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(5)">5s</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(10)">10s</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(30)">30s</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(60)">1m</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(300)">5m</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(900)">15m</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(3600)">1h</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(21600)">6h</button>
                            <button type="button" class="btn-primary" onclick="setIntervalPreset(86400)">1d</button>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="randomOrder">Playback Order:</label>
                        <select id="randomOrder" name="randomOrder">
                            <option value="false">📁 Sequential - Alphabetical order</option>
                            <option value="true">🎲 Random - Shuffle mode</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="loop">Loop Mode:</label>
                        <select id="loop" name="loop">
                            <option value="true">🔄 Loop - Restart from beginning</option>
                            <option value="false">⏹️ Stop - End after last image</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="disableBrightness">Disable Brightness (keep display off):</label>
                        <input type="checkbox" id="disableBrightness" name="disableBrightness" checked onchange="onDisableBrightnessChange(this.checked)"> (default: disabled)
                        <div style="margin-top:8px;">
                            <label for="brightness">Display Brightness:</label>
                            <input type="range" id="brightness" name="brightness" min="0" max="255" value="0" oninput="updateBrightnessValue(this.value)" disabled>
                            <span class="range-value" id="brightnessValue">0%</span>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="imageFilter">Image Format Filter:</label>
                        <select id="imageFilter" name="imageFilter">
                            <option value="">🖼️ All Formats - Show all supported images</option>
                            <option value=".raw">⚡ RAW Only - Fastest display (recommended)</option>
                            <option value=".png">🎨 PNG Only - Best quality</option>
                            <option value=".jpg">📷 JPG Only - Standard format</option>
                            <option value=".bmp">🖥️ BMP Only - Uncompressed</option>
                        </select>
                    </div>
                    
                    <!-- Transition removed as per UI simplification -->
                    
                    <div class="form-group">
                        <label for="autoStart">Auto Start on Boot:</label>
                        <select id="autoStart" name="autoStart">
                            <option value="true">🚀 Auto Start - Begin slideshow at startup</option>
                            <option value="false">⏸️ Manual Start - Wait for user command</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="scalingMode">Image Scaling Mode:</label>
                        <select id="scalingMode" name="scalingMode">
                            <option value="fit">📐 Fit Screen - Scale to fit display</option>
                            <option value="fill">🔲 Fill Screen - Crop to fill display</option>
                            <option value="center">📍 Center - Original size, centered</option>
                            <option value="stretch">↔️ Stretch - Fill entire screen</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="rotationAngle">Image Rotation:</label>
                        <select id="rotationAngle" name="rotationAngle">
                            <option value="0">⬆️ Normal - 0°</option>
                            <option value="90">➡️ Right - 90°</option>
                            <option value="180">⬇️ Upside Down - 180°</option>
                            <option value="270">⬅️ Left - 270°</option>
                        </select>
                    </div>
                    
                    <button type="button" class="btn-success" onclick="saveSlideshowConfig()">🎯 Save Slideshow Settings</button>
                </form>
            </div>
)HTMLEND";

const char* html_controls = R"HTMLEND(
            <div class="section">
                <h3>🎮 Slideshow Controls</h3>
                <div class="control-grid">
                    <!-- Ordered for symmetry: Prev | Start/Pause/Stop | Next -->
                    <button class="btn-primary" onclick="controlSlideshow('prev')">⏮️ Previous</button>
                    <div class="vertical-controls">
                        <button class="btn-success btn-compact" onclick="controlSlideshow('start')">▶️ Start</button>
                        <button class="btn-warning btn-compact" onclick="controlSlideshow('pause')">⏸️ Pause</button>
                        <button class="btn-danger btn-compact" onclick="controlSlideshow('stop')">⏹️ Stop</button>
                    </div>
                    <button class="btn-primary" onclick="controlSlideshow('next')">⏭️ Next</button>
                </div>
            </div>
            
            <div class="section">
                <h3>📊 System Status</h3>
                <div class="status-grid">
                    <div class="status-card">
                        <div class="status-value" id="wifiStatusValue">-</div>
                        <div class="status-label">WiFi Status</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="ipAddressValue">-</div>
                        <div class="status-label">IP Address</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="slideshowStatusValue">-</div>
                        <div class="status-label">Slideshow</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="uptimeValue">-</div>
                        <div class="status-label">Uptime</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="memoryValue">-</div>
                        <div class="status-label">Free Memory</div>
                    </div>
                    <div class="status-card">
                        <div class="status-value" id="imagesCountValue">-</div>
                        <div class="status-label">Images Found</div>
                    </div>
                </div>
                <div style="margin-top: 20px; display:flex; gap:8px; flex-wrap:wrap; justify-content:center;">
                    <button class="btn-primary btn-compact" onclick="loadConfig()">🔄 Refresh</button>
                    <button class="btn-warning btn-compact" onclick="restartDevice()">🔁 Restart</button>
                    <button class="btn-danger btn-compact" onclick="factoryReset()">⚠️ Reset</button>
                </div>
            </div>
            
            <div class="tab-container">
                <div class="tabs">
                    <div class="tab active" onclick="showTab('advanced-tab')">⚙️ Advanced</div>
                    <div class="tab" onclick="showTab('display-tab')">🖥️ Display</div>
                    <div class="tab" onclick="showTab('logs-tab')">📝 Logs</div>
                </div>
                
                <div id="advanced-tab" class="tab-content active">
                    <div class="section">
                        <h3>⚙️ Advanced Settings</h3>
                        <form id="advancedForm">
                            <div class="form-group">
                                <label for="webPort">Web Server Port:</label>
                                <input type="number" id="webPort" name="webPort" min="80" max="65535" value="80">
                            </div>
                            
                            <div class="form-group">
                                <label for="maxFileSize">Max File Size (MB):</label>
                                <input type="number" id="maxFileSize" name="maxFileSize" min="1" max="50" value="10">
                            </div>
                            
                            <div class="form-group">
                                <label for="cacheEnabled">Enable Image Caching:</label>
                                <select id="cacheEnabled" name="cacheEnabled">
                                    <option value="true">✅ Enabled - Faster loading</option>
                                    <option value="false">❌ Disabled - Save memory</option>
                                </select>
                            </div>
                            
                            <div class="form-group">
                                <label for="debugMode">Debug Mode:</label>
                                <select id="debugMode" name="debugMode">
                                    <option value="false">❌ Disabled - Normal operation</option>
                                    <option value="true">✅ Enabled - Verbose logging</option>
                                </select>
                            </div>
                            
                            <button type="button" class="btn-primary" onclick="saveAdvancedConfig()">💾 Save Advanced Settings</button>
                        </form>
                    </div>
                </div>
                
                <div id="display-tab" class="tab-content">
                    <div class="section">
                        <h3>🖥️ Display Settings</h3>
                        <form id="displayForm">
                            <!-- Contrast and Saturation controls removed for simplicity on mobile and to reduce confusion -->
                            
                            <div class="form-group">
                                <label for="screensaverTime">Screensaver Timeout (minutes):</label>
                                <input type="number" id="screensaverTime" name="screensaverTime" min="0" max="120" value="0" placeholder="0 = disabled">
                            </div>
                            
                            <div class="form-group">
                                <label for="powerSaveMode">Power Save Mode:</label>
                                <select id="powerSaveMode" name="powerSaveMode">
                                    <option value="none">❌ Disabled - Full performance</option>
                                    <option value="low">🔋 Low - Reduce brightness after timeout</option>
                                    <option value="high">🌙 High - Sleep mode after timeout</option>
                                </select>
                            </div>
                            
                            <button type="button" class="btn-primary" onclick="saveDisplayConfig()">💾 Save Display Settings</button>
                        </form>
                    </div>
                </div>
                
                <!-- Storage tab removed as requested -->
                
                <div id="logs-tab" class="tab-content">
                    <div class="section">
                        <h3>📝 System Logs</h3>
                        <div style="margin-bottom: 15px;">
                            <button class="btn-primary" onclick="refreshLogs()">🔄 Refresh</button>
                            <div style="display:inline-flex; gap:8px;">
                                <button class="btn-warning btn-compact" onclick="clearLogs()">🗑️ Clear Logs</button>
                                <button class="btn-primary btn-compact" onclick="downloadLogs()">💾 Download</button>
                            </div>
                        </div>
                        <textarea id="systemLogs" readonly style="height: 300px; font-family: monospace; font-size: 12px;">
Loading system logs...
                        </textarea>
                    </div>
                </div>
            </div>
        </div>
    </div>
)HTMLEND";

const char* html_script = R"HTMLEND(
    <script>
        // Global variables
        let currentConfig = {};
        let statusInterval;
        let alertTimeout;
        
        // Initialize page
        document.addEventListener('DOMContentLoaded', function() {
            loadConfig();
            // Do not start automatic status updates to avoid interrupting user input
            // startStatusUpdates();
            setupEventListeners();
        });
        
        function setupEventListeners() {
            // Auto-save on form changes with debounce
            const forms = ['wifiForm', 'slideshowForm', 'advancedForm', 'displayForm'];
            forms.forEach(formId => {
                const form = document.getElementById(formId);
                if (form) {
                            const inputs = form.querySelectorAll('input, select');
                            inputs.forEach(input => {
                                // Inform user only on focus loss, not on every change
                                input.addEventListener('blur', () => {
                                    if (formId === 'wifiForm') showAlert('Network settings will be saved when you click Save button', 'info');
                                });
                            });
                }
            });
        }
        
        function debounce(func, wait) {
            let timeout;
            return function executedFunction(...args) {
                const later = () => {
                    clearTimeout(timeout);
                    func(...args);
                };
                clearTimeout(timeout);
                timeout = setTimeout(later, wait);
            };
        }
        
        function startStatusUpdates() {
            // Disabled by default. Use manual Refresh button to avoid interfering with input fields.
            // To enable automatic updates set a longer interval (eg. 60000 ms).
            // statusInterval = setInterval(loadConfig, 60000);
        }
        
        function showTab(tabId) {
            // Hide all tab contents
            document.querySelectorAll('.tab-content').forEach(content => {
                content.classList.remove('active');
            });
            
            // Remove active class from all tabs
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.remove('active');
            });
            
            // Show selected tab
            document.getElementById(tabId).classList.add('active');
            event.target.classList.add('active');
        }
        
        function updateIntervalValue(value) {
            // If value is a slider position (0-100) -> map exponentially to 1s..86400s
            // If value is already seconds ( > 100 ) we display it directly
            const min = 1;
            const max = 86400;
            let seconds = Number(value);
            if (seconds >= 0 && seconds <= 100) {
                const exponent = seconds / 100; // 0..1
                seconds = Math.round(min * Math.pow(max / min, exponent));
                // store mapped value on slider for save
                const el = document.getElementById('interval'); if (el) el.dataset.mapped = seconds;
            }
            document.getElementById('intervalValue').textContent = formatSeconds(seconds);
        }

        function setIntervalPreset(seconds) {
            const min = 1; const max = 86400;
            const ratio = Math.log(Math.max(seconds,1) / min) / Math.log(max / min);
            const pos = Math.max(0, Math.min(100, Math.round(ratio * 100)));
            const el = document.getElementById('interval'); if (el) { el.value = pos; updateIntervalValue(pos); }
        }

        function formatSeconds(s) {
            if (s < 60) return s + 's';
            if (s < 3600) return Math.round(s/60) + 'm';
            if (s < 86400) return Math.round(s/3600) + 'h';
            return Math.round(s/86400) + 'd';
        }

        function togglePasswordVisibility() {
            const p = document.getElementById('password'); if (!p) return; p.type = p.type === 'password' ? 'text' : 'password';
        }

        function onDisableBrightnessChange(checked) {
            const b = document.getElementById('brightness');
            if (b) {
                b.disabled = checked;
                if (checked) { b.value = 0; updateBrightnessValue(0); }
                else { b.value = 128; updateBrightnessValue(128); }
            }
        }
        
        function updateBrightnessValue(value) {
            const percentage = Math.round((value / 255) * 100);
            document.getElementById('brightnessValue').textContent = percentage + '%';
        }
        
        function updateTransitionValue(value) {
            document.getElementById('transitionValue').textContent = value + 'ms';
        }
        
    /* Contrast and saturation controls removed */
        
        function showAlert(message, type = 'info', duration = 5000) {
            clearTimeout(alertTimeout);
            
            // Remove existing alerts
            document.querySelectorAll('.alert').forEach(alert => alert.remove());
            
            const alertDiv = document.createElement('div');
            alertDiv.className = `alert alert-${type}`;
            alertDiv.innerHTML = message;
            
            // Insert after header
            const header = document.querySelector('.header');
            header.parentNode.insertBefore(alertDiv, header.nextSibling);
            
            // Auto remove after duration
            if (duration > 0) {
                alertTimeout = setTimeout(() => alertDiv.remove(), duration);
            }
        }
        
        function setLoading(element, loading = true) {
            if (loading) {
                element.classList.add('loading');
                element.disabled = true;
            } else {
                element.classList.remove('loading');
                element.disabled = false;
            }
        }
        
        async function loadConfig() {
            try {
                const response = await fetch('/get-config');
                if (!response.ok) throw new Error('Failed to load configuration');
                
                const data = await response.json();
                currentConfig = data;
                
                // Update WiFi form
                updateFormField('ssid', data.wifi?.ssid || '');
                updateFormField('deviceName', data.wifi?.deviceName || 'Pokemon-Expositor');
                updateFormField('enableAP', data.wifi?.enableAP ? 'true' : 'false');
                updateFormField('hostname', data.wifi?.hostname || 'pokemon-expositor');
                
                // Update slideshow form
                updateFormField('enabled', data.slideshow?.enabled ? 'true' : 'false');
                // Set interval slider position using preset mapping helper
                const intervalSeconds = (data.slideshow?.intervalMs || 10000) / 1000;
                setIntervalPreset(intervalSeconds);
                updateFormField('randomOrder', data.slideshow?.randomOrder ? 'true' : 'false');
                updateFormField('loop', data.slideshow?.loop ? 'true' : 'false');
                updateFormField('brightness', data.slideshow?.brightness || 128);
                updateFormField('imageFilter', data.slideshow?.imageFilter || '');
                // transition removed from UI
                updateFormField('autoStart', data.slideshow?.autoStart ? 'true' : 'false');
                updateFormField('scalingMode', data.slideshow?.scalingMode || 'fit');
                updateFormField('rotationAngle', data.slideshow?.rotationAngle || '0');
                
                // Update advanced form
                updateFormField('webPort', data.advanced?.webPort || 80);
                updateFormField('maxFileSize', data.advanced?.maxFileSize || 10);
                updateFormField('cacheEnabled', data.advanced?.cacheEnabled ? 'true' : 'false');
                updateFormField('debugMode', data.advanced?.debugMode ? 'true' : 'false');
                
                // Update display form (contrast and saturation controls removed)
                updateFormField('screensaverTime', data.display?.screensaverTime || 0);
                updateFormField('powerSaveMode', data.display?.powerSaveMode || 'none');
                
                // Update range value displays
                updateBrightnessValue(data.slideshow?.brightness || 128);
                // intervalValue already updated by setIntervalPreset
                // transition removed from UI
                // initialize disableBrightness checkbox
                const disableEl = document.getElementById('disableBrightness');
                if (disableEl) disableEl.checked = !!data.slideshow?.disableBrightness;
                // Contrast and saturation controls removed from UI
                
                // Update status cards
                updateStatusCards(data);
                
            } catch (error) {
                // Suppress verbose console output on client; show user-friendly alert
                showAlert('Failed to load configuration', 'error');
            }
        }
        
        function updateFormField(fieldId, value) {
            const field = document.getElementById(fieldId);
            if (field) {
                field.value = value;
            }
        }
        
        function updateStatusCards(data) {
            const status = data.status || {};
            
            // WiFi Status
            const wifiStatus = status.wifiConnected ? '🟢 Connected' : '🔴 Disconnected';
            updateStatusCard('wifiStatusValue', wifiStatus);
            
            // IP Address
            const ipAddress = status.wifiConnected ? status.localIP : status.apIP || 'N/A';
            updateStatusCard('ipAddressValue', ipAddress);
            
            // Slideshow Status
            const slideshowStatus = data.slideshow?.enabled ? '▶️ Running' : '⏹️ Stopped';
            updateStatusCard('slideshowStatusValue', slideshowStatus);
            
            // Uptime
            const uptime = formatUptime(status.uptime || 0);
            updateStatusCard('uptimeValue', uptime);
            
            // Memory
            const memory = formatBytes(status.freeHeap || 0);
            updateStatusCard('memoryValue', memory);
            
            // Images count
            updateStatusCard('imagesCountValue', status.imagesCount || '0');
            
            // Storage info
            updateStatusCard('totalSpaceValue', formatBytes(status.totalSpace || 0));
            updateStatusCard('usedSpaceValue', formatBytes(status.usedSpace || 0));
            updateStatusCard('freeSpaceValue', formatBytes(status.freeSpace || 0));
            // Storage fields removed from UI
        }
        
        function updateStatusCard(cardId, value) {
            const card = document.getElementById(cardId);
            if (card) card.textContent = value;
        }
        
        function formatUptime(milliseconds) {
            const seconds = Math.floor(milliseconds / 1000);
            const days = Math.floor(seconds / 86400);
            const hours = Math.floor((seconds % 86400) / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            
            if (days > 0) return `${days}d ${hours}h`;
            if (hours > 0) return `${hours}h ${minutes}m`;
            return `${minutes}m`;
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 B';
            const k = 1024;
            const sizes = ['B', 'KB', 'MB', 'GB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
        }
        
        async function saveWiFiConfig() {
            const button = event.target;
            setLoading(button);
            
            try {
                const formData = new FormData(document.getElementById('wifiForm'));
                const data = Object.fromEntries(formData);
                data.enableAP = data.enableAP === 'true';
                
                const response = await fetch('/save-config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ wifi: data })
                });
                
                if (!response.ok) throw new Error('Failed to save configuration');
                
                showAlert('📡 WiFi configuration saved! Device will restart in 3 seconds...', 'success');
                // Do not auto-reload the page to allow user to remain on the form
                
            } catch (error) {
                showAlert('❌ Failed to save WiFi configuration', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function saveSlideshowConfig() {
            const button = event.target;
            setLoading(button);
            
            try {
                const formData = new FormData(document.getElementById('slideshowForm'));
                const data = Object.fromEntries(formData);
                
                // Convert form data to proper types
                data.enabled = data.enabled === 'true';
                data.randomOrder = data.randomOrder === 'true';
                data.loop = data.loop === 'true';
                data.autoStart = data.autoStart === 'true';
                // interval slider stores mapped seconds in dataset.mapped
                const slider = document.getElementById('interval');
                if (slider && slider.dataset && slider.dataset.mapped) {
                    data.intervalMs = parseInt(slider.dataset.mapped) * 1000;
                } else {
                    data.intervalMs = parseInt(data.interval) * 1000;
                }
                data.brightness = parseInt(data.brightness);
                // include disableBrightness flag
                data.disableBrightness = !!document.getElementById('disableBrightness') && document.getElementById('disableBrightness').checked;
                // transition removed from UI; no transitionMs sent
                data.rotationAngle = parseInt(data.rotationAngle);
                
                const response = await fetch('/save-config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ slideshow: data })
                });
                
                if (!response.ok) throw new Error('Failed to save configuration');
                
                showAlert('🎬 Slideshow configuration saved successfully!', 'success');
                loadConfig();
                
            } catch (error) {
                showAlert('❌ Failed to save slideshow configuration', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function saveAdvancedConfig() {
            const button = event.target;
            setLoading(button);
            
            try {
                const formData = new FormData(document.getElementById('advancedForm'));
                const data = Object.fromEntries(formData);
                
                data.cacheEnabled = data.cacheEnabled === 'true';
                data.debugMode = data.debugMode === 'true';
                data.webPort = parseInt(data.webPort);
                data.maxFileSize = parseInt(data.maxFileSize);
                
                const response = await fetch('/save-config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ advanced: data })
                });
                
                if (!response.ok) throw new Error('Failed to save configuration');
                
                showAlert('⚙️ Advanced settings saved successfully!', 'success');
                
            } catch (error) {
                showAlert('❌ Failed to save advanced settings', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function saveDisplayConfig() {
            const button = event.target;
            setLoading(button);
            
            try {
                const formData = new FormData(document.getElementById('displayForm'));
                const data = Object.fromEntries(formData);
                
                // Contrast and saturation removed from UI; only keep screensaver
                data.screensaverTime = parseInt(data.screensaverTime);
                
                const response = await fetch('/save-config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ display: data })
                });
                
                if (!response.ok) throw new Error('Failed to save configuration');
                
                showAlert('🖥️ Display settings saved successfully!', 'success');
                
            } catch (error) {
                showAlert('❌ Failed to save display settings', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function controlSlideshow(action) {
            const button = event.target;
            setLoading(button);
            
            try {
                const response = await fetch('/slideshow-control', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ action: action })
                });
                
                if (!response.ok) throw new Error('Command failed');
                
                const actionNames = {
                    start: '▶️ started',
                    stop: '⏹️ stopped',
                    pause: '⏸️ paused',
                    next: '⏭️ next image',
                    prev: '⏮️ previous image'
                };
                
                showAlert(`Slideshow ${actionNames[action] || action}!`, 'success', 3000);
                
            } catch (error) {
                showAlert('❌ Failed to control slideshow', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function restartDevice() {
            if (!confirm('🔄 Are you sure you want to restart the device?\\n\\nThis will interrupt the current slideshow.')) {
                return;
            }
            
            try {
                const response = await fetch('/restart', { method: 'POST' });
                showAlert('🔄 Device is restarting... Please wait 30 seconds before reconnecting.', 'info', 0);
                
                // Clear status updates
                clearInterval(statusInterval);
                
                // Device is restarting on server side; do not force page reload here
                // User can manually refresh after reconnecting
                
            } catch (error) {
                showAlert('Device restart initiated. Please refresh the page after device comes back online.', 'info', 0);
            }
        }
        
        async function factoryReset() {
            if (!confirm('⚠️ Factory Reset Warning!\\n\\nThis will delete ALL settings and return to default configuration.\\nThis action cannot be undone.\\n\\nAre you absolutely sure?')) {
                return;
            }
            
            if (!confirm('🚨 FINAL WARNING!\\n\\nAll WiFi settings, slideshow configurations, and preferences will be lost!\\n\\nType YES in the next dialog to confirm.')) {
                return;
            }
            
            const confirmation = prompt('Type "YES" in capital letters to confirm factory reset:');
            if (confirmation !== 'YES') {
                showAlert('Factory reset cancelled.', 'info');
                return;
            }
            
            try {
                const response = await fetch('/factory-reset', { method: 'POST' });
                showAlert('🏭 Factory reset completed. Device will restart with default settings.', 'success', 0);
                
                clearInterval(statusInterval);
                // Device will restart; user may manually refresh after reboot
                
            } catch (error) {
                showAlert('Factory reset initiated. Device will restart with default settings.', 'info', 0);
            }
        }
        
        async function reloadImages() {
            const button = event.target;
            setLoading(button);
            
            try {
                const response = await fetch('/reload-images', { method: 'POST' });
                if (!response.ok) throw new Error('Failed to reload images');
                
                showAlert('🔄 Image library reloaded successfully!', 'success');
                loadConfig();
                
            } catch (error) {
                showAlert('❌ Failed to reload images', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function scanImages() {
            const button = event.target;
            setLoading(button);
            
            try {
                const response = await fetch('/scan-images', { method: 'POST' });
                if (!response.ok) throw new Error('Failed to scan images');
                
                const result = await response.json();
                showAlert(`🔍 Scan complete! Found ${result.count || 0} images.`, 'success');
                loadConfig();
                
            } catch (error) {
                showAlert('❌ Failed to scan images', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function clearCache() {
            if (!confirm('🗑️ Clear image cache?\\n\\nThis will free up memory but images may load slower initially.')) {
                return;
            }
            
            const button = event.target;
            setLoading(button);
            
            try {
                const response = await fetch('/clear-cache', { method: 'POST' });
                if (!response.ok) throw new Error('Failed to clear cache');
                
                showAlert('🗑️ Image cache cleared successfully!', 'success');
                loadConfig();
                
            } catch (error) {
                showAlert('❌ Failed to clear cache', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function formatStorage() {
            if (!confirm('⚠️ FORMAT STORAGE WARNING!\\n\\nThis will DELETE ALL IMAGES and data on the SD card!\\nThis action cannot be undone!\\n\\nAre you absolutely sure?')) {
                return;
            }
            
            const confirmation = prompt('Type "FORMAT" in capital letters to confirm:');
            if (confirmation !== 'FORMAT') {
                showAlert('Storage format cancelled.', 'info');
                return;
            }
            
            const button = event.target;
            setLoading(button);
            
            try {
                const response = await fetch('/format-storage', { method: 'POST' });
                if (!response.ok) throw new Error('Failed to format storage');
                
                showAlert('💾 Storage formatted successfully! Please upload new images.', 'success');
                loadConfig();
                
            } catch (error) {
                showAlert('❌ Failed to format storage', 'error');
            } finally {
                setLoading(button, false);
            }
        }
        
        async function refreshLogs() {
            try {
                const response = await fetch('/get-logs');
                if (!response.ok) throw new Error('Failed to get logs');
                
                const logs = await response.text();
                document.getElementById('systemLogs').value = logs;
                
            } catch (error) {
                document.getElementById('systemLogs').value = 'Error loading logs';
            }
        }
        
        async function clearLogs() {
            if (!confirm('Clear all system logs?')) return;
            
            try {
                const response = await fetch('/clear-logs', { method: 'POST' });
                if (!response.ok) throw new Error('Failed to clear logs');
                
                document.getElementById('systemLogs').value = 'Logs cleared.';
                showAlert('📝 System logs cleared successfully!', 'success');
                
            } catch (error) {
                showAlert('❌ Failed to clear logs', 'error');
            }
        }
        
        function downloadLogs() {
            const logs = document.getElementById('systemLogs').value;
            const blob = new Blob([logs], { type: 'text/plain' });
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `pokemon-expositor-logs-${new Date().toISOString().split('T')[0]}.txt`;
            a.click();
            window.URL.revokeObjectURL(url);
        }
        
        // Cleanup on page unload
        window.addEventListener('beforeunload', function() {
            clearInterval(statusInterval);
            clearTimeout(alertTimeout);
        });
    </script>
</body>
</html>
)HTMLEND";

// Function to build complete HTML
String buildCompleteHTML() {
    String html = String(html_header) + 
                  String(html_wifi_form) + 
                  String(html_slideshow_form) + 
                  String(html_controls) + 
                  String(html_script);
    return html;
}

WiFiConfigManager::WiFiConfigManager() 
    : webServer(nullptr), wifiConnected(false), webServerStarted(false), configLoaded(false) {
    setDefaultConfigs();
}

WiFiConfigManager::~WiFiConfigManager() {
    shutdown();
}

bool WiFiConfigManager::initialize() {
    Serial.println("[WiFi] Initializing WiFi Config Manager...");

    // If we've already started the web server, initialization is complete
    if (webServerStarted && webServer) {
        Serial.println("[WiFi] Already initialized - web server running");
        return true;
    }

    // SD card may be initialized by SD Manager earlier in startup.
    // Prefer not to call SD.begin() here (hardware-specific), just check availability.
    if (!SD.exists("/")) {
        // keep only a single warning if SD not available
        Serial.println("[WiFi] WARNING: SD not available, using defaults");
    }
    
    // Create config directory on SD card if it doesn't exist
    if (SD.exists("/")) {
        if (!SD.exists("/config")) {
            if (SD.mkdir("/config")) {
                Serial.println("[WiFi] /config directory created on SD card");
            } else {
                Serial.println("[WiFi] ERROR: Failed to create /config directory");
                // continue without persistent config
            }
        }
    }
    
    // Load configurations
    loadWiFiConfig();
    loadSlideshowConfig();
    loadAdvancedConfig();
    loadDisplayConfig();
    configLoaded = true;
    
    // Set WiFi hostname
    WiFi.setHostname(wifiConfig.deviceName.c_str());
    
    // Try to connect to WiFi
    if (!wifiConfig.ssid.isEmpty()) {
        if (connectToWiFi()) {
            Serial.println("[WiFi] Connected to WiFi");
        } else {
            Serial.println("[WiFi] WiFi connect failed - will start AP");
        }
    }
    
    // Start Access Point if enabled or WiFi failed
    if (wifiConfig.enableAP || !wifiConnected) {
        startAccessPoint();
    }
    
    // Initialize web server (create once)
    if (!webServer) {
        webServer = new WebServer(WEB_SERVER_PORT);
        if (!webServer) {
            Serial.println("[WiFi] ERROR: Failed to create web server instance");
            return false;
        }
    }
    
    // Setup web server routes
    Serial.println("[WiFi] Configuring web server routes...");
    // Main page route
    webServer->on("/", [this]() { handleRoot(); });
    
    webServer->on("/test", [this]() {
        webServer->send(200, "text/plain", "Hello from ESP32!");
    });

    // Simple health endpoint for quick checks
    webServer->on("/ping", [this]() {
        webServer->send(200, "text/plain", "pong");
    });
    
    webServer->on("/config", [this]() { handleConfig(); });
    webServer->on("/save-config", HTTP_POST, [this]() { handleSaveConfig(); });
    webServer->on("/get-config", [this]() { handleGetConfig(); });
    webServer->on("/slideshow-control", HTTP_POST, [this]() { handleSlideshowControl(); });
    webServer->on("/restart", HTTP_POST, [this]() { handleRestart(); });
    webServer->on("/advanced-config", [this]() { handleAdvancedConfig(); });
    webServer->on("/save-advanced", HTTP_POST, [this]() { handleSaveAdvanced(); });
    webServer->on("/display-config", [this]() { handleDisplayConfig(); });
    webServer->on("/save-display", HTTP_POST, [this]() { handleSaveDisplay(); });
    webServer->on("/get-logs", [this]() { handleGetLogs(); });
    webServer->on("/clear-logs", HTTP_POST, [this]() { handleClearLogs(); });
    webServer->on("/system-status", [this]() { handleGetSystemStatus(); });
    webServer->on("/storage-operation", HTTP_POST, [this]() { handleStorageOperation(); });
    webServer->on("/brightness-control", HTTP_POST, [this]() { handleBrightnessControl(); });
    webServer->on("/apply-settings", HTTP_POST, [this]() { handleApplySettings(); });
    webServer->onNotFound([this]() { handleNotFound(); });
    
    // minimal log
    Serial.println("[WiFi] Starting web server");
    // begin() is safe to call multiple times on the same instance
    webServer->begin();
    webServerStarted = true;

    Serial.printf("[WiFi] Web server on port %d\n", WEB_SERVER_PORT);
    printNetworkInfo();
    
    // Apply initial settings if system is ready
    if (isSystemReady()) {
        applyBrightnessSettings();
        applyDisplaySettings();
        Serial.println("[WiFi] Initial settings applied");
    } else {
    Serial.println("[WiFi] System not ready - settings deferred");
    }
    
    return true;
}

void WiFiConfigManager::shutdown() {
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
        webServerStarted = false;
    }
    
    WiFi.disconnect();
    wifiConnected = false;
    
    Serial.println("[WiFi] WiFi Config Manager shutdown complete");
}

bool WiFiConfigManager::connectToWiFi() {
    if (wifiConfig.ssid.isEmpty()) {
        Serial.println("[WiFi] No SSID configured");
        return false;
    }
    
    Serial.printf("[WiFi] Connecting to WiFi: %s\n", wifiConfig.ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        wifiConnected = false;
        Serial.println("[WiFi] Connection failed");
        return false;
    }
}

void WiFiConfigManager::startAccessPoint() {
    Serial.println("[WiFi] Starting Access Point...");
    
    // Configure AP IP address
    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    if (!WiFi.softAPConfig(apIP, gateway, subnet)) {
        Serial.println("[WiFi] WARNING: Failed to configure AP IP");
    } else {
        Serial.println("[WiFi] AP IP configured to 192.168.4.1");
    }
    
    WiFi.mode(wifiConnected ? WIFI_AP_STA : WIFI_AP);
    
    String apSSID = wifiConfig.deviceName + "_Config";
    bool success;
    
    if (wifiConfig.apPassword.length() >= 8) {
        success = WiFi.softAP(apSSID.c_str(), wifiConfig.apPassword.c_str());
    } else {
        success = WiFi.softAP(apSSID.c_str());
    }
    
    if (success) {
        Serial.printf("[WiFi] AP started: %s\n", apSSID.c_str());
        Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("[WiFi] Failed to start AP");
    }
}

void WiFiConfigManager::printNetworkInfo() {
    Serial.println("\n[WiFi] ===== NETWORK INFO =====");
    
    if (wifiConnected) {
        Serial.printf("WiFi SSID: %s\n", wifiConfig.ssid.c_str());
        Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    }
    
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        Serial.printf("AP SSID: %s_Config\n", wifiConfig.deviceName.c_str());
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    }
    
    Serial.printf("Device Name: %s\n", wifiConfig.deviceName.c_str());
    Serial.printf("Web Interface: http://%s/\n", 
        wifiConnected ? WiFi.localIP().toString().c_str() : WiFi.softAPIP().toString().c_str());
    Serial.println("============================\n");
}

bool WiFiConfigManager::loadWiFiConfig() {
    if (!SD.exists(WIFI_CONFIG_FILE)) {
        Serial.println("[WiFi] No WiFi config file found on SD card, using defaults");
        return false;
    }
    
    File file = SD.open(WIFI_CONFIG_FILE, "r");
    if (!file) {
        Serial.println("[WiFi] Failed to open WiFi config file on SD card");
        return false;
    }
    
    String content = file.readString();
    file.close();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    
    if (error) {
        Serial.printf("[WiFi] Failed to parse WiFi config: %s\n", error.c_str());
        return false;
    }
    
    wifiConfig.ssid = doc["ssid"].as<String>();
    wifiConfig.password = doc["password"].as<String>();
    wifiConfig.deviceName = doc["deviceName"] | "Pokemon-Expositor";
    wifiConfig.enableAP = doc["enableAP"] | true;
    wifiConfig.apPassword = doc["apPassword"] | "pokemon123";
    
    Serial.println("[WiFi] WiFi configuration loaded");
    return true;
}

bool WiFiConfigManager::loadSlideshowConfig() {
    if (!SD.exists(SLIDESHOW_CONFIG_FILE)) {
        Serial.println("[WiFi] No slideshow config file found on SD card, using defaults");
        return false;
    }
    
    File file = SD.open(SLIDESHOW_CONFIG_FILE, "r");
    if (!file) {
        Serial.println("[WiFi] Failed to open slideshow config file");
        return false;
    }
    
    String content = file.readString();
    file.close();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    
    if (error) {
        Serial.printf("[WiFi] Failed to parse slideshow config: %s\n", error.c_str());
        return false;
    }
    
    slideshowConfig.enabled = doc["enabled"] | false;
    slideshowConfig.intervalMs = doc["intervalMs"] | 10000;
    slideshowConfig.randomOrder = doc["randomOrder"] | false;
    slideshowConfig.loop = doc["loop"] | true;
    // Default to display off by default for slideshow (user requested)
    slideshowConfig.brightness = doc["brightness"] | 0;
    slideshowConfig.imageFilter = doc["imageFilter"] | ".raw";
    // transitionMs removed from UI; ignore any incoming value
    slideshowConfig.autoStart = doc["autoStart"] | false;
    // New flag: disableBrightness (if true, keep display off regardless of brightness)
    slideshowConfig.disableBrightness = doc.containsKey("disableBrightness") ? doc["disableBrightness"] | true : true;
    
    Serial.println("[WiFi] Slideshow configuration loaded");
    return true;
}

bool WiFiConfigManager::saveWiFiConfig() {
    DynamicJsonDocument doc(1024);
    
    doc["ssid"] = wifiConfig.ssid;
    doc["password"] = wifiConfig.password;
    doc["deviceName"] = wifiConfig.deviceName;
    doc["enableAP"] = wifiConfig.enableAP;
    doc["apPassword"] = wifiConfig.apPassword;
    
    File file = SD.open(WIFI_CONFIG_FILE, "w");
    if (!file) {
        Serial.println("[WiFi] Failed to create WiFi config file on SD card");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("[WiFi] WiFi configuration saved");
    return true;
}

bool WiFiConfigManager::saveSlideshowConfig() {
    DynamicJsonDocument doc(1024);
    
    doc["enabled"] = slideshowConfig.enabled;
    doc["intervalMs"] = slideshowConfig.intervalMs;
    doc["randomOrder"] = slideshowConfig.randomOrder;
    doc["loop"] = slideshowConfig.loop;
    doc["brightness"] = slideshowConfig.brightness;
    doc["imageFilter"] = slideshowConfig.imageFilter;
    // transitionMs intentionally omitted (removed from UI)
    doc["autoStart"] = slideshowConfig.autoStart;
    // Persist disableBrightness so UI and device agree
    doc["disableBrightness"] = slideshowConfig.disableBrightness;
    
    File file = SD.open(SLIDESHOW_CONFIG_FILE, "w");
    if (!file) {
        Serial.println("[WiFi] Failed to create slideshow config file on SD card");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("[WiFi] Slideshow configuration saved");
    return true;
}

void WiFiConfigManager::setDefaultConfigs() {
    // Default WiFi config
    wifiConfig.ssid = "";
    wifiConfig.password = "";
    wifiConfig.deviceName = "Pokemon-Expositor";
    wifiConfig.enableAP = true;
    wifiConfig.apPassword = "pokemon123";
    
    // Default slideshow config
    slideshowConfig.enabled = false;
    slideshowConfig.intervalMs = 10000;  // 10 seconds
    slideshowConfig.randomOrder = false;
    slideshowConfig.loop = true;
    // default slideshow brightness set to 0 and disabled by default (checkbox checked)
    slideshowConfig.brightness = 0;
    slideshowConfig.imageFilter = ".raw";
    // transition disabled by default
    slideshowConfig.autoStart = false;
    slideshowConfig.disableBrightness = true;
    
    // Default advanced config
    advancedConfig.enablePreloading = true;
    advancedConfig.cacheSize = 4;
    advancedConfig.maxFileSize = 5242880; // 5MB
    advancedConfig.enableDeepSleep = false;
    advancedConfig.sleepTimeout = 300000; // 5 minutes
    advancedConfig.logLevel = "INFO";
    advancedConfig.enableSerial = true;
    advancedConfig.enableDebug = false;
    advancedConfig.enableErrorRecovery = true;
    advancedConfig.watchdogTimeout = 30000; // 30 seconds
    
    // Default display config
    displayConfig.brightness = 128;
    displayConfig.scalingMode = "fit";
    displayConfig.rotation = 0;
    displayConfig.flipHorizontal = false;
    displayConfig.flipVertical = false;
    displayConfig.colorProfile = "default";
    displayConfig.enableDithering = false;
    displayConfig.enableAntiAliasing = true;
    displayConfig.screensaverTimeout = 0; // Disabled
}

bool WiFiConfigManager::loadAdvancedConfig() {
    if (!SD.exists(ADVANCED_CONFIG_FILE)) {
        Serial.println("[WiFi] Advanced config file not found on SD card, using defaults");
        return false;
    }
    
    File file = SD.open(ADVANCED_CONFIG_FILE, "r");
    if (!file) {
        Serial.println("[WiFi] ERROR: Failed to open advanced config file");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[WiFi] ERROR: Failed to parse advanced config: %s\n", error.c_str());
        return false;
    }
    
    advancedConfig.enablePreloading = doc["enablePreloading"] | true;
    advancedConfig.cacheSize = doc["cacheSize"] | 4;
    advancedConfig.maxFileSize = doc["maxFileSize"] | 5242880;
    advancedConfig.enableDeepSleep = doc["enableDeepSleep"] | false;
    advancedConfig.sleepTimeout = doc["sleepTimeout"] | 300000;
    advancedConfig.logLevel = doc["logLevel"] | "INFO";
    advancedConfig.enableSerial = doc["enableSerial"] | true;
    advancedConfig.enableDebug = doc["enableDebug"] | false;
    advancedConfig.enableErrorRecovery = doc["enableErrorRecovery"] | true;
    advancedConfig.watchdogTimeout = doc["watchdogTimeout"] | 30000;
    
    Serial.println("[WiFi] Advanced configuration loaded");
    return true;
}

bool WiFiConfigManager::loadDisplayConfig() {
    if (!SD.exists(DISPLAY_CONFIG_FILE)) {
        Serial.println("[WiFi] Display config file not found on SD card, using defaults");
        return false;
    }
    
    File file = SD.open(DISPLAY_CONFIG_FILE, "r");
    if (!file) {
        Serial.println("[WiFi] ERROR: Failed to open display config file");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[WiFi] ERROR: Failed to parse display config: %s\n", error.c_str());
        return false;
    }
    
    displayConfig.brightness = doc["brightness"] | 128;
    displayConfig.scalingMode = doc["scalingMode"] | "fit";
    displayConfig.rotation = doc["rotation"] | 0;
    displayConfig.flipHorizontal = doc["flipHorizontal"] | false;
    displayConfig.flipVertical = doc["flipVertical"] | false;
    displayConfig.colorProfile = doc["colorProfile"] | "default";
    displayConfig.enableDithering = doc["enableDithering"] | false;
    displayConfig.enableAntiAliasing = doc["enableAntiAliasing"] | true;
    displayConfig.screensaverTimeout = doc["screensaverTimeout"] | 0;
    
    Serial.println("[WiFi] Display configuration loaded");
    return true;
}

bool WiFiConfigManager::saveAdvancedConfig() {
    JsonDocument doc;
    
    doc["enablePreloading"] = advancedConfig.enablePreloading;
    doc["cacheSize"] = advancedConfig.cacheSize;
    doc["maxFileSize"] = advancedConfig.maxFileSize;
    doc["enableDeepSleep"] = advancedConfig.enableDeepSleep;
    doc["sleepTimeout"] = advancedConfig.sleepTimeout;
    doc["logLevel"] = advancedConfig.logLevel;
    doc["enableSerial"] = advancedConfig.enableSerial;
    doc["enableDebug"] = advancedConfig.enableDebug;
    doc["enableErrorRecovery"] = advancedConfig.enableErrorRecovery;
    doc["watchdogTimeout"] = advancedConfig.watchdogTimeout;
    
    File file = SD.open(ADVANCED_CONFIG_FILE, "w");
    if (!file) {
        Serial.println("[WiFi] ERROR: Failed to create advanced config file on SD card");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("[WiFi] Advanced configuration saved");
    return true;
}

bool WiFiConfigManager::saveDisplayConfig() {
    JsonDocument doc;
    
    doc["brightness"] = displayConfig.brightness;
    doc["scalingMode"] = displayConfig.scalingMode;
    doc["rotation"] = displayConfig.rotation;
    doc["flipHorizontal"] = displayConfig.flipHorizontal;
    doc["flipVertical"] = displayConfig.flipVertical;
    doc["colorProfile"] = displayConfig.colorProfile;
    doc["enableDithering"] = displayConfig.enableDithering;
    doc["enableAntiAliasing"] = displayConfig.enableAntiAliasing;
    doc["screensaverTimeout"] = displayConfig.screensaverTimeout;
    
    File file = SD.open(DISPLAY_CONFIG_FILE, "w");
    if (!file) {
        Serial.println("[WiFi] ERROR: Failed to create display config file on SD card");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("[WiFi] Display configuration saved");
    return true;
}

void WiFiConfigManager::handleWebRequests() {
    static unsigned long lastActivityTime = 0;
    static int requestCount = 0;
    static unsigned long lastDetailedDebug = 0;
    
    if (webServer && webServerStarted) {
        // Minimal periodic debug - only every 60s
        if (millis() - lastDetailedDebug > 60000) {
            Serial.printf("[WiFi] Web server running (ptr=%p)\n", webServer);
            lastDetailedDebug = millis();
        }
        
        // Rimuoviamo il controllo del client e chiamiamo sempre handleClient()
        webServer->handleClient();
        requestCount++;
        
        // Debug summary occasionally
        if (millis() - lastActivityTime > 60000) {
            Serial.printf("[WiFi] handleClient() calls in last minute: %d\n", requestCount);
            requestCount = 0;
            lastActivityTime = millis();
        }
    } else {
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 60000) { // Debug ogni 60 secondi
            Serial.printf("[WiFi] DEBUG: webServer=%p, webServerStarted=%s\n", 
                         webServer, webServerStarted ? "true" : "false");
            lastDebugTime = millis();
        }
    }
}

String WiFiConfigManager::getLocalIP() const {
    return wifiConnected ? WiFi.localIP().toString() : "";
}

String WiFiConfigManager::getAPIP() const {
    return WiFi.softAPIP().toString();
}

// Web server handlers
void WiFiConfigManager::handleRoot() {
    Serial.println("[WiFi] 🌐 handleRoot() called - serving main page");
    String html = buildCompleteHTML();
    Serial.printf("[WiFi] 📄 HTML size: %d bytes\n", html.length());
    webServer->send(200, "text/html", html);
    Serial.println("[WiFi] ✅ Response sent to client");
}

void WiFiConfigManager::handleConfig() {
    handleRoot();  // Same as root for now
}

void WiFiConfigManager::handleSaveConfig() {
    if (webServer->method() != HTTP_POST) {
        webServer->send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    String body = webServer->arg("plain");
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        webServer->send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    bool needsRestart = false;
    
    // Update WiFi config if provided
    if (doc.containsKey("wifi")) {
        WiFiConfig newWifiConfig = wifiConfig;
        
        if (doc["wifi"].containsKey("ssid")) {
            newWifiConfig.ssid = doc["wifi"]["ssid"].as<String>();
        }
        if (doc["wifi"].containsKey("password")) {
            newWifiConfig.password = doc["wifi"]["password"].as<String>();
        }
        if (doc["wifi"].containsKey("deviceName")) {
            newWifiConfig.deviceName = doc["wifi"]["deviceName"].as<String>();
        }
        if (doc["wifi"].containsKey("enableAP")) {
            newWifiConfig.enableAP = doc["wifi"]["enableAP"].as<bool>();
        }
        if (doc["wifi"].containsKey("apPassword")) {
            newWifiConfig.apPassword = doc["wifi"]["apPassword"].as<String>();
        }
        
        // Check if WiFi settings changed
        if (newWifiConfig.ssid != wifiConfig.ssid || 
            newWifiConfig.password != wifiConfig.password ||
            newWifiConfig.deviceName != wifiConfig.deviceName ||
            newWifiConfig.enableAP != wifiConfig.enableAP) {
            needsRestart = true;
        }
        
        wifiConfig = newWifiConfig;
        saveWiFiConfig();
    }
    
    // Update slideshow config if provided
    if (doc.containsKey("slideshow")) {
        if (doc["slideshow"].containsKey("enabled")) {
            slideshowConfig.enabled = doc["slideshow"]["enabled"].as<bool>();
        }
        if (doc["slideshow"].containsKey("intervalMs")) {
            slideshowConfig.intervalMs = doc["slideshow"]["intervalMs"].as<uint32_t>();
        }
        if (doc["slideshow"].containsKey("randomOrder")) {
            slideshowConfig.randomOrder = doc["slideshow"]["randomOrder"].as<bool>();
        }
        if (doc["slideshow"].containsKey("loop")) {
            slideshowConfig.loop = doc["slideshow"]["loop"].as<bool>();
        }
        if (doc["slideshow"].containsKey("brightness")) {
            slideshowConfig.brightness = doc["slideshow"]["brightness"].as<uint8_t>();
        }
        if (doc["slideshow"].containsKey("disableBrightness")) {
            slideshowConfig.disableBrightness = doc["slideshow"]["disableBrightness"].as<bool>();
        }
        if (doc["slideshow"].containsKey("imageFilter")) {
            slideshowConfig.imageFilter = doc["slideshow"]["imageFilter"].as<String>();
        }
        // transitionMs intentionally ignored (removed from UI)
        if (doc["slideshow"].containsKey("autoStart")) {
            slideshowConfig.autoStart = doc["slideshow"]["autoStart"].as<bool>();
        }
        
        saveSlideshowConfig();
    }
    
    if (needsRestart) {
        webServer->send(200, "text/plain", "Configuration saved. Device will restart in 3 seconds.");
        delay(1000);
        ESP.restart();
    } else {
        webServer->send(200, "text/plain", "Configuration saved successfully.");
    }
}

void WiFiConfigManager::handleGetConfig() {
    DynamicJsonDocument doc(2048);
    
    // WiFi config (don't send passwords)
    doc["wifi"]["ssid"] = wifiConfig.ssid;
    doc["wifi"]["deviceName"] = wifiConfig.deviceName;
    doc["wifi"]["enableAP"] = wifiConfig.enableAP;
    
    // Slideshow config
    doc["slideshow"]["enabled"] = slideshowConfig.enabled;
    doc["slideshow"]["intervalMs"] = slideshowConfig.intervalMs;
    doc["slideshow"]["randomOrder"] = slideshowConfig.randomOrder;
    doc["slideshow"]["loop"] = slideshowConfig.loop;
    doc["slideshow"]["brightness"] = slideshowConfig.brightness;
    doc["slideshow"]["imageFilter"] = slideshowConfig.imageFilter;
    // transitionMs removed from response
    doc["slideshow"]["autoStart"] = slideshowConfig.autoStart;
    doc["slideshow"]["disableBrightness"] = slideshowConfig.disableBrightness;
    
    // Status info
    // Status info (include system status snapshot)
    SystemStatus status = getSystemStatus();
    doc["status"]["wifiConnected"] = status.wifiConnected;
    doc["status"]["localIP"] = status.localIP;
    doc["status"]["apIP"] = status.apIP;
    doc["status"]["uptime"] = status.uptime;
    doc["status"]["freeHeap"] = status.freeHeap;
    doc["status"]["chipModel"] = status.chipModel;
    doc["status"]["imagesCount"] = status.imagesCount;
    doc["status"]["totalSpace"] = status.totalSpace;
    doc["status"]["usedSpace"] = status.usedSpace;
    doc["status"]["freeSpace"] = status.freeSpace;
    // SD diagnostics
    uint8_t ct = SD.cardType();
    const char* ctStr = "UNKNOWN";
    if (ct == CARD_NONE) ctStr = "NONE";
    else if (ct == CARD_MMC) ctStr = "MMC";
    else if (ct == CARD_SD) ctStr = "SDSC";
    else if (ct == CARD_SDHC) ctStr = "SDHC";
    doc["status"]["cardType"] = ctStr;
    uint64_t cardSizeMB = 0;
    if (SD.totalBytes() > 0) cardSizeMB = (SD.totalBytes() + (1024ULL*1024ULL - 1)) / (1024ULL*1024ULL);
    doc["status"]["cardSizeMB"] = cardSizeMB;
    
    String response;
    serializeJson(doc, response);
    webServer->send(200, "application/json", response);
}

void WiFiConfigManager::handleSlideshowControl() {
    if (webServer->method() != HTTP_POST) {
        webServer->send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    String body = webServer->arg("plain");
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        webServer->send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    String action = doc["action"].as<String>();
    bool success = false;
    
    if (action == "start") {
        success = startSlideshow();
    } else if (action == "stop") {
        success = stopSlideshow();
    } else if (action == "pause") {
        success = pauseSlideshow();
    } else if (action == "next") {
        success = nextImage();
    } else if (action == "prev") {
        success = previousImage();
    }
    
    if (success) {
        webServer->send(200, "text/plain", "Command executed successfully");
    } else {
        webServer->send(500, "text/plain", "Command failed");
    }
}

void WiFiConfigManager::handleRestart() {
    webServer->send(200, "text/plain", "Device restarting...");
    delay(1000);
    ESP.restart();
}

void WiFiConfigManager::handleNotFound() {
    Serial.printf("[WiFi] ❌ 404 - Requested URL: %s\n", webServer->uri().c_str());
    Serial.printf("[WiFi] ❌ Client IP: %s\n", webServer->client().remoteIP().toString().c_str());
    webServer->send(404, "text/plain", "File not found");
}

void WiFiConfigManager::handleAdvancedConfig() {
    JsonDocument doc;
    
    doc["enablePreloading"] = advancedConfig.enablePreloading;
    doc["cacheSize"] = advancedConfig.cacheSize;
    doc["maxFileSize"] = advancedConfig.maxFileSize;
    doc["enableDeepSleep"] = advancedConfig.enableDeepSleep;
    doc["sleepTimeout"] = advancedConfig.sleepTimeout;
    doc["logLevel"] = advancedConfig.logLevel;
    doc["enableSerial"] = advancedConfig.enableSerial;
    doc["enableDebug"] = advancedConfig.enableDebug;
    doc["enableErrorRecovery"] = advancedConfig.enableErrorRecovery;
    doc["watchdogTimeout"] = advancedConfig.watchdogTimeout;
    
    String response;
    serializeJson(doc, response);
    webServer->send(200, "application/json", response);
}

void WiFiConfigManager::handleSaveAdvanced() {
    if (!webServer->hasArg("plain")) {
        webServer->send(400, "text/plain", "Missing request body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, webServer->arg("plain"));
    
    if (error) {
        webServer->send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    advancedConfig.enablePreloading = doc["enablePreloading"] | true;
    advancedConfig.cacheSize = doc["cacheSize"] | 4;
    advancedConfig.maxFileSize = doc["maxFileSize"] | 5242880;
    advancedConfig.enableDeepSleep = doc["enableDeepSleep"] | false;
    advancedConfig.sleepTimeout = doc["sleepTimeout"] | 300000;
    advancedConfig.logLevel = doc["logLevel"] | "INFO";
    advancedConfig.enableSerial = doc["enableSerial"] | true;
    advancedConfig.enableDebug = doc["enableDebug"] | false;
    advancedConfig.enableErrorRecovery = doc["enableErrorRecovery"] | true;
    advancedConfig.watchdogTimeout = doc["watchdogTimeout"] | 30000;
    
    if (saveAdvancedConfig()) {
        webServer->send(200, "text/plain", "Advanced configuration saved successfully");
    } else {
        webServer->send(500, "text/plain", "Failed to save advanced configuration");
    }
}

void WiFiConfigManager::handleDisplayConfig() {
    JsonDocument doc;
    
    doc["brightness"] = displayConfig.brightness;
    doc["scalingMode"] = displayConfig.scalingMode;
    doc["rotation"] = displayConfig.rotation;
    doc["flipHorizontal"] = displayConfig.flipHorizontal;
    doc["flipVertical"] = displayConfig.flipVertical;
    doc["colorProfile"] = displayConfig.colorProfile;
    doc["enableDithering"] = displayConfig.enableDithering;
    doc["enableAntiAliasing"] = displayConfig.enableAntiAliasing;
    doc["screensaverTimeout"] = displayConfig.screensaverTimeout;
    
    String response;
    serializeJson(doc, response);
    webServer->send(200, "application/json", response);
}

void WiFiConfigManager::handleSaveDisplay() {
    if (!webServer->hasArg("plain")) {
        webServer->send(400, "text/plain", "Missing request body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, webServer->arg("plain"));
    
    if (error) {
        webServer->send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    displayConfig.brightness = doc["brightness"] | 128;
    displayConfig.scalingMode = doc["scalingMode"] | "fit";
    displayConfig.rotation = doc["rotation"] | 0;
    displayConfig.flipHorizontal = doc["flipHorizontal"] | false;
    displayConfig.flipVertical = doc["flipVertical"] | false;
    displayConfig.colorProfile = doc["colorProfile"] | "default";
    displayConfig.enableDithering = doc["enableDithering"] | false;
    displayConfig.enableAntiAliasing = doc["enableAntiAliasing"] | true;
    displayConfig.screensaverTimeout = doc["screensaverTimeout"] | 0;
    
    if (saveDisplayConfig()) {
        webServer->send(200, "text/plain", "Display configuration saved successfully");
    } else {
        webServer->send(500, "text/plain", "Failed to save display configuration");
    }
}

void WiFiConfigManager::handleGetLogs() {
    String logs = getSystemLogs();
    webServer->send(200, "text/plain", logs);
}

void WiFiConfigManager::handleClearLogs() {
    clearSystemLogs();
    webServer->send(200, "text/plain", "System logs cleared successfully");
}

void WiFiConfigManager::handleGetSystemStatus() {
    SystemStatus status = getSystemStatus();
    
    JsonDocument doc;
    doc["wifiConnected"] = status.wifiConnected;
    doc["localIP"] = status.localIP;
    doc["apIP"] = status.apIP;
    doc["uptime"] = status.uptime;
    doc["freeHeap"] = status.freeHeap;
    doc["chipModel"] = status.chipModel;
    doc["imagesCount"] = status.imagesCount;
    doc["totalSpace"] = status.totalSpace;
    doc["usedSpace"] = status.usedSpace;
    doc["freeSpace"] = status.freeSpace;
    // Add SD card diagnostics
    uint8_t ct = SD.cardType();
    const char* ctStr = "UNKNOWN";
    if (ct == CARD_NONE) ctStr = "NONE";
    else if (ct == CARD_MMC) ctStr = "MMC";
    else if (ct == CARD_SD) ctStr = "SDSC";
    else if (ct == CARD_SDHC) ctStr = "SDHC";
    doc["cardType"] = ctStr;
    // cardSize in MB (rounded)
    uint64_t cardSizeMB = 0;
    if (SD.totalBytes() > 0) cardSizeMB = (SD.totalBytes() + (1024ULL*1024ULL - 1)) / (1024ULL*1024ULL);
    doc["cardSizeMB"] = cardSizeMB;
    
    String response;
    serializeJson(doc, response);
    webServer->send(200, "application/json", response);
}

void WiFiConfigManager::handleStorageOperation() {
    if (!webServer->hasArg("operation")) {
        webServer->send(400, "text/plain", "Missing operation parameter");
        return;
    }
    
    String operation = webServer->arg("operation");
    
    if (operation == "format") {
        if (formatStorage()) {
            webServer->send(200, "text/plain", "Storage formatted successfully");
        } else {
            webServer->send(500, "text/plain", "Failed to format storage");
        }
    } else if (operation == "cleanup") {
        if (cleanupTempFiles()) {
            webServer->send(200, "text/plain", "Temporary files cleaned up successfully");
        } else {
            webServer->send(500, "text/plain", "Failed to cleanup temporary files");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid operation");
    }
}

void WiFiConfigManager::handleBrightnessControl() {
    if (!webServer->hasArg("action")) {
        webServer->send(400, "text/plain", "Missing action parameter");
        return;
    }
    
    String action = webServer->arg("action");
    
    try {
        if (action == "increase") {
            systemManager.increaseBrightness();
            webServer->send(200, "text/plain", "Brightness increased");
        } else if (action == "decrease") {
            systemManager.decreaseBrightness();
            webServer->send(200, "text/plain", "Brightness decreased");
        } else if (action == "low") {
            systemManager.setBrightnessLow();
            webServer->send(200, "text/plain", "Brightness set to low");
        } else if (action == "medium") {
            systemManager.setBrightnessMedium();
            webServer->send(200, "text/plain", "Brightness set to medium");
        } else if (action == "high") {
            systemManager.setBrightnessHigh();
            webServer->send(200, "text/plain", "Brightness set to high");
        } else if (action == "max") {
            systemManager.setBrightnessMax();
            webServer->send(200, "text/plain", "Brightness set to maximum");
        } else if (action == "night") {
            systemManager.setBrightnessNight();
            webServer->send(200, "text/plain", "Brightness set to night mode");
        } else if (webServer->hasArg("value")) {
            int percentage = webServer->arg("value").toInt();
            if (percentage >= 0 && percentage <= 100) {
                systemManager.setBrightness(percentage);
                webServer->send(200, "text/plain", "Brightness set to " + String(percentage) + "%");
            } else {
                webServer->send(400, "text/plain", "Invalid brightness value (0-100)");
            }
        } else {
            webServer->send(400, "text/plain", "Invalid action or missing value");
        }
        addLogEntry("INFO", "Brightness control: " + action);
    } catch (...) {
        webServer->send(500, "text/plain", "Failed to control brightness");
        addLogEntry("ERROR", "Failed to control brightness: " + action);
    }
}

void WiFiConfigManager::handleApplySettings() {
    try {
        if (isSystemReady()) {
            applyBrightnessSettings();
            applyDisplaySettings();
            webServer->send(200, "text/plain", "Settings applied successfully");
            addLogEntry("INFO", "Settings applied via web interface");
        } else {
            webServer->send(500, "text/plain", "System not ready");
            addLogEntry("WARNING", "Attempted to apply settings but system not ready");
        }
    } catch (...) {
        webServer->send(500, "text/plain", "Failed to apply settings");
        addLogEntry("ERROR", "Failed to apply settings");
    }
}

// Configuration update methods
bool WiFiConfigManager::updateWiFiConfig(const WiFiConfig& newConfig) {
    wifiConfig = newConfig;
    return saveWiFiConfig();
}

bool WiFiConfigManager::updateSlideshowConfig(const SlideshowConfig& newConfig) {
    slideshowConfig = newConfig;
    return saveSlideshowConfig();
}

bool WiFiConfigManager::updateAdvancedConfig(const AdvancedConfig& newConfig) {
    advancedConfig = newConfig;
    return saveAdvancedConfig();
}

bool WiFiConfigManager::updateDisplayConfig(const DisplayConfig& newConfig) {
    displayConfig = newConfig;
    return saveDisplayConfig();
}

// Serial command interface
void WiFiConfigManager::handleSerialCommand(const String& command, const String& params) {
    if (command == "wifi") {
        if (params.startsWith("ssid ")) {
            wifiConfig.ssid = params.substring(5);
            saveWiFiConfig();
            Serial.printf("[WiFi] SSID set to: %s\n", wifiConfig.ssid.c_str());
        } else if (params.startsWith("password ")) {
            wifiConfig.password = params.substring(9);
            saveWiFiConfig();
            Serial.println("[WiFi] Password updated");
        } else if (params == "connect") {
            if (connectToWiFi()) {
                Serial.println("[WiFi] Connected successfully");
            } else {
                Serial.println("[WiFi] Connection failed");
            }
        } else if (params == "disconnect") {
            WiFi.disconnect();
            wifiConnected = false;
            Serial.println("[WiFi] Disconnected");
        } else if (params == "status") {
            printNetworkInfo();
        } else {
            printHelp();
        }
    } else if (command == "slideshow") {
        if (params == "start") {
            if (startSlideshow()) {
                Serial.println("[Slideshow] Started");
            } else {
                Serial.println("[Slideshow] Failed to start");
            }
        } else if (params == "stop") {
            if (stopSlideshow()) {
                Serial.println("[Slideshow] Stopped");
            } else {
                Serial.println("[Slideshow] Failed to stop");
            }
        } else if (params == "pause") {
            if (pauseSlideshow()) {
                Serial.println("[Slideshow] Paused");
            } else {
                Serial.println("[Slideshow] Failed to pause");
            }
        } else if (params == "next") {
            if (nextImage()) {
                Serial.println("[Slideshow] Next image");
            } else {
                Serial.println("[Slideshow] Failed to go to next image");
            }
        } else if (params == "prev") {
            if (previousImage()) {
                Serial.println("[Slideshow] Previous image");
            } else {
                Serial.println("[Slideshow] Failed to go to previous image");
            }
        } else if (params.startsWith("interval ")) {
            int interval = params.substring(9).toInt();
            if (interval > 0) {
                slideshowConfig.intervalMs = interval * 1000;
                saveSlideshowConfig();
                Serial.printf("[Slideshow] Interval set to %d seconds\n", interval);
            } else {
                Serial.println("[Slideshow] Invalid interval");
            }
        } else if (params.startsWith("brightness ")) {
            int brightness = params.substring(11).toInt();
            if (brightness >= 0 && brightness <= 255) {
                slideshowConfig.brightness = brightness;
                saveSlideshowConfig();
                Serial.printf("[Slideshow] Brightness set to %d\n", brightness);
            } else {
                Serial.println("[Slideshow] Invalid brightness (0-255)");
            }
        } else if (params == "random") {
            slideshowConfig.randomOrder = !slideshowConfig.randomOrder;
            saveSlideshowConfig();
            Serial.printf("[Slideshow] Random order: %s\n", slideshowConfig.randomOrder ? "ON" : "OFF");
        } else if (params == "loop") {
            slideshowConfig.loop = !slideshowConfig.loop;
            saveSlideshowConfig();
            Serial.printf("[Slideshow] Loop: %s\n", slideshowConfig.loop ? "ON" : "OFF");
        } else if (params == "status") {
            printStatus();
        } else {
            printHelp();
        }
    } else if (command == "config") {
        if (params == "save") {
            saveWiFiConfig();
            saveSlideshowConfig();
            Serial.println("[Config] All configurations saved");
        } else if (params == "load") {
            loadWiFiConfig();
            loadSlideshowConfig();
            Serial.println("[Config] All configurations loaded");
        } else if (params == "reset") {
            setDefaultConfigs();
            saveWiFiConfig();
            saveSlideshowConfig();
            Serial.println("[Config] Configurations reset to defaults");
        } else {
            printHelp();
        }
    }
}

void WiFiConfigManager::printStatus() {
    Serial.println("\n[WiFi] ===== CONFIGURATION STATUS =====");
    
    // WiFi Status
    Serial.printf("WiFi SSID: %s\n", wifiConfig.ssid.isEmpty() ? "Not configured" : wifiConfig.ssid.c_str());
    Serial.printf("WiFi Status: %s\n", wifiConnected ? "Connected" : "Disconnected");
    Serial.printf("Device Name: %s\n", wifiConfig.deviceName.c_str());
    Serial.printf("AP Mode: %s\n", wifiConfig.enableAP ? "Enabled" : "Disabled");
    
    if (wifiConnected) {
        Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
    }
    
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    }
    
    // Slideshow Status
    Serial.printf("\nSlideshow Enabled: %s\n", slideshowConfig.enabled ? "YES" : "NO");
    Serial.printf("Interval: %d seconds\n", slideshowConfig.intervalMs / 1000);
    Serial.printf("Random Order: %s\n", slideshowConfig.randomOrder ? "YES" : "NO");
    Serial.printf("Loop: %s\n", slideshowConfig.loop ? "YES" : "NO");
    Serial.printf("Brightness: %d\n", slideshowConfig.brightness);
    Serial.printf("Image Filter: %s\n", slideshowConfig.imageFilter.c_str());
    // Transition removed from UI and logs
    Serial.printf("Auto Start: %s\n", slideshowConfig.autoStart ? "YES" : "NO");
    
    Serial.printf("\nWeb Interface: http://%s/\n", 
        wifiConnected ? WiFi.localIP().toString().c_str() : WiFi.softAPIP().toString().c_str());
    Serial.println("=====================================\n");
}

void WiFiConfigManager::printHelp() {
    Serial.println("\n[WiFi] ===== AVAILABLE COMMANDS =====");
    Serial.println("WiFi Commands:");
    Serial.println("  wifi ssid <name>       - Set WiFi SSID");
    Serial.println("  wifi password <pass>   - Set WiFi password");
    Serial.println("  wifi connect           - Connect to WiFi");
    Serial.println("  wifi disconnect        - Disconnect from WiFi");
    Serial.println("  wifi status            - Show WiFi status");
    
    Serial.println("\nSlideshow Commands:");
    Serial.println("  slideshow start        - Start slideshow");
    Serial.println("  slideshow stop         - Stop slideshow");
    Serial.println("  slideshow pause        - Pause slideshow");
    Serial.println("  slideshow next         - Next image");
    Serial.println("  slideshow prev         - Previous image");
    Serial.println("  slideshow interval <s> - Set interval in seconds");
    Serial.println("  slideshow brightness <0-255> - Set brightness");
    Serial.println("  slideshow random       - Toggle random order");
    Serial.println("  slideshow loop         - Toggle loop mode");
    Serial.println("  slideshow status       - Show slideshow status");
    
    Serial.println("\nConfig Commands:");
    Serial.println("  config save            - Save all configurations");
    Serial.println("  config load            - Load all configurations");
    Serial.println("  config reset           - Reset to defaults");
    Serial.println("=====================================\n");
}

// Slideshow control methods - Integrated with System Manager
bool WiFiConfigManager::startSlideshow() {
    slideshowConfig.enabled = true;
    if (saveSlideshowConfig()) {
        Serial.println("[WiFi] Slideshow configuration saved, starting slideshow...");
        bool success = systemManager.startSlideshow();
        if (success) {
            Serial.println("[WiFi] Slideshow started successfully");
            addLogEntry("INFO", "Slideshow started via web interface");
        } else {
            Serial.println("[WiFi] Failed to start slideshow");
            addLogEntry("ERROR", "Failed to start slideshow");
        }
        return success;
    } else {
        Serial.println("[WiFi] Failed to save slideshow configuration");
        addLogEntry("ERROR", "Failed to save slideshow configuration");
        return false;
    }
}

bool WiFiConfigManager::stopSlideshow() {
    slideshowConfig.enabled = false;
    if (saveSlideshowConfig()) {
        Serial.println("[WiFi] Slideshow configuration saved, stopping slideshow...");
        bool success = systemManager.stopSlideshow();
        if (success) {
            Serial.println("[WiFi] Slideshow stopped successfully");
            addLogEntry("INFO", "Slideshow stopped via web interface");
        } else {
            Serial.println("[WiFi] Failed to stop slideshow");
            addLogEntry("ERROR", "Failed to stop slideshow");
        }
        return success;
    } else {
        Serial.println("[WiFi] Failed to save slideshow configuration");
        addLogEntry("ERROR", "Failed to save slideshow configuration");
        return false;
    }
}

bool WiFiConfigManager::pauseSlideshow() {
    Serial.println("[WiFi] Pausing slideshow...");
    bool success = systemManager.pauseSlideshow();
    if (success) {
        Serial.println("[WiFi] Slideshow paused successfully");
        addLogEntry("INFO", "Slideshow paused via web interface");
    } else {
        Serial.println("[WiFi] Failed to pause slideshow");
        addLogEntry("ERROR", "Failed to pause slideshow");
    }
    return success;
}

bool WiFiConfigManager::nextImage() {
    Serial.println("[WiFi] Going to next image...");
    try {
        systemManager.nextImage();
        Serial.println("[WiFi] Advanced to next image successfully");
        addLogEntry("INFO", "Advanced to next image via web interface");
        return true;
    } catch (...) {
        Serial.println("[WiFi] Failed to go to next image");
        addLogEntry("ERROR", "Failed to go to next image");
        return false;
    }
}

bool WiFiConfigManager::previousImage() {
    Serial.println("[WiFi] Going to previous image...");
    try {
        systemManager.previousImage();
        Serial.println("[WiFi] Advanced to previous image successfully");
        addLogEntry("INFO", "Advanced to previous image via web interface");
        return true;
    } catch (...) {
        Serial.println("[WiFi] Failed to go to previous image");
        addLogEntry("ERROR", "Failed to go to previous image");
        return false;
    }
}

// System management functions
SystemStatus WiFiConfigManager::getSystemStatus() {
    SystemStatus status;
    
    status.wifiConnected = wifiConnected;
    status.localIP = getLocalIP();
    status.apIP = getAPIP();
    status.uptime = millis();
    status.freeHeap = ESP.getFreeHeap();
    status.chipModel = ESP.getChipModel();
    status.imagesCount = getImageCount();
    
    // Get storage information
    status.totalSpace = SD.totalBytes();
    status.usedSpace = SD.usedBytes();
    status.freeSpace = status.totalSpace - status.usedSpace;

    // Card type and size (MB) for UI
    uint8_t ct = SD.cardType();
    String ctStr = "UNKNOWN";
    if (ct == CARD_NONE) ctStr = "NONE";
    else if (ct == CARD_MMC) ctStr = "MMC";
    else if (ct == CARD_SD) ctStr = "SDSC";
    else if (ct == CARD_SDHC) ctStr = "SDHC";
    else ctStr = "UNKNOWN";

    // extend SystemStatus through dynamic fields by writing to variables used by handlers
    // We'll add these to the JSON when requested
    // cardType stored temporarily in chipModel (not ideal) - instead we'll compute in handler
    
    return status;
}

String WiFiConfigManager::getSystemLogs() {
    String logs = "System Logs:\n";
    logs += "============\n";
    logs += "Uptime: " + String(millis() / 1000) + " seconds\n";
    logs += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    logs += "WiFi Status: " + String(wifiConnected ? "Connected" : "Disconnected") + "\n";
    logs += "Storage Used: " + String(SD.usedBytes()) + "/" + String(SD.totalBytes()) + " bytes\n";
    logs += "Image Count: " + String(getImageCount()) + "\n";
    
    // TODO: Add actual log entries from a log buffer/file
    logs += "\nRecent Events:\n";
    logs += "[INFO] WiFi Config Manager initialized\n";
    logs += "[INFO] Web server started on port 80\n";
    if (wifiConnected) {
        logs += "[INFO] Connected to WiFi: " + wifiConfig.ssid + "\n";
    }
    logs += "[INFO] Access Point: " + wifiConfig.deviceName + "-AP\n";
    
    return logs;
}

void WiFiConfigManager::clearSystemLogs() {
    // TODO: Implement actual log clearing
    Serial.println("[WiFi] System logs cleared");
    addLogEntry("INFO", "System logs cleared");
}

void WiFiConfigManager::addLogEntry(const String& level, const String& message) {
    String timestamp = String(millis() / 1000);
    String logEntry = "[" + timestamp + "] [" + level + "] " + message;
    Serial.println(logEntry);
    // TODO: Store in circular buffer or file for web interface
}

bool WiFiConfigManager::formatStorage() {
    Serial.println("[WiFi] Resetting configuration to defaults...");
    // Remove all config files from SD card
    SD.remove(WIFI_CONFIG_FILE);
    SD.remove(SLIDESHOW_CONFIG_FILE);
    SD.remove(ADVANCED_CONFIG_FILE);
    SD.remove(DISPLAY_CONFIG_FILE);
    
    // Recreate config directory and set defaults
    SD.mkdir("/config");
    setDefaultConfigs();
    saveWiFiConfig();
    saveSlideshowConfig();
    saveAdvancedConfig();
    saveDisplayConfig();
    addLogEntry("INFO", "Configuration reset to defaults");
    return true;
}

bool WiFiConfigManager::cleanupTempFiles() {
    // TODO: Implement cleanup of temporary files
    Serial.println("[WiFi] Cleaning up temporary files...");
    addLogEntry("INFO", "Temporary files cleaned up");
    return true;
}

uint32_t WiFiConfigManager::getImageCount() {
    // Count actual image files in SD card images directory
    File root = SD.open("/images");
    if (!root || !root.isDirectory()) {
        // Only search inside /images as requested
        return 0;
    }
    
    uint32_t count = 0;
    File file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        if (fileName.endsWith(".raw") || fileName.endsWith(".jpg") || 
            fileName.endsWith(".png") || fileName.endsWith(".bmp")) {
            count++;
        }
        file = root.openNextFile();
    }
    
    return count;
}

// Additional system integration functions
void WiFiConfigManager::applyBrightnessSettings() {
    try {
        if (slideshowConfig.disableBrightness) {
            systemManager.setBrightness(0);
            Serial.println("[WiFi] Slideshow brightness disabled by config (display off)");
        } else {
            int brightnessPercentage = map(slideshowConfig.brightness, 0, 255, 0, 100);
            systemManager.setBrightness(brightnessPercentage);
            Serial.printf("[WiFi] Applied brightness setting: %d%% (%d/255)\n", 
                         brightnessPercentage, slideshowConfig.brightness);
        }
        addLogEntry("INFO", "Brightness settings applied from configuration");
    } catch (...) {
        Serial.println("[WiFi] Failed to apply brightness settings");
        addLogEntry("ERROR", "Failed to apply brightness settings");
    }
}

void WiFiConfigManager::applyDisplaySettings() {
    try {
        // Apply brightness from display config if available
        if (displayConfig.brightness > 0) {
            int brightnessPercentage = map(displayConfig.brightness, 0, 255, 0, 100);
            systemManager.setBrightness(brightnessPercentage);
            Serial.printf("[WiFi] Applied display brightness: %d%% (%d/255)\n", 
                         brightnessPercentage, displayConfig.brightness);
        }
        addLogEntry("INFO", "Display settings applied from configuration");
    } catch (...) {
        Serial.println("[WiFi] Failed to apply display settings");
        addLogEntry("ERROR", "Failed to apply display settings");
    }
}

bool WiFiConfigManager::isSystemReady() {
    return systemManager.isSystemReady();
}

String WiFiConfigManager::getSlideshowStatus() {
    String status = "Slideshow Status:\n";
    status += "Enabled: " + String(slideshowConfig.enabled ? "Yes" : "No") + "\n";
    status += "Interval: " + String(slideshowConfig.intervalMs) + "ms\n";
    status += "Random: " + String(slideshowConfig.randomOrder ? "Yes" : "No") + "\n";
    status += "Loop: " + String(slideshowConfig.loop ? "Yes" : "No") + "\n";
    status += "Auto Start: " + String(slideshowConfig.autoStart ? "Yes" : "No") + "\n";
    status += "System Ready: " + String(systemManager.isSystemReady() ? "Yes" : "No") + "\n";
    status += "Image Count: " + String(getImageCount()) + "\n";
    return status;
}
