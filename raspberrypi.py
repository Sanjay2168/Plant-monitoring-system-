from flask import Flask, request, jsonify
from grove.display.jhd1802 import JHD1802

# Initialize Flask app
app = Flask(__name__)

# Initialize Grove 16x2 LCD on I2C (default address 0x3E)
lcd = JHD1802()

def display_data_on_lcd(temperature, humidity):
    """
    Displays temperature and humidity on the LCD in the correct format.
    """
    lcd.clear()
    lcd.setCursor(0, 0)
    lcd.write("Temperature: {:.1f}".format(temperature))
    lcd.setCursor(0, 1)
    lcd.write("Humidity: {:.1f}".format(humidity))

@app.route('/data', methods=['POST'])
def receive_data():
    """
    Endpoint to receive data from Arduino via HTTP POST.
    """
    try:
        data = request.json
        print(f"Received Data: {data}")

        # Extract temperature and humidity from received JSON
        temperature = data.get("temperature", 0)
        humidity = data.get("humidity", 0)

        # Display on LCD
        display_data_on_lcd(temperature, humidity)

        # Respond to Arduino
        return jsonify({"status": "success"}), 200
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == "__main__":
    try:
        # Run Flask server on all available interfaces and port 5000
        app.run(host="0.0.0.0", port=5000)
    except KeyboardInterrupt:
        lcd.clear()
        print("Server stopped.")
