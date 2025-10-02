#!/bin/bash

echo "============================================"
echo "  MusicBox - NFC Reader Test"
echo "============================================"
echo ""
echo "This will test your PN532 NFC reader"
echo "Press CTRL+C to exit"
echo ""

# Upload test firmware
echo "📤 Uploading test firmware..."
~/.platformio/penv/bin/platformio run -e nfc_test --target upload

echo ""
echo "✅ Upload complete!"
echo ""
echo "📺 Opening serial monitor..."
echo "   Press the RESET button on your ESP32 to see the test results"
echo ""

# Open monitor
~/.platformio/penv/bin/platformio device monitor -e nfc_test
