from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# Initialize LED statuses
led_status = {'led1': 'OFF2', 'led2': 'OFF4', 'led3': 'OFF6', 'led4': 'OFF8'}

@app.route('/')
def index():
    return render_template('index.html', status1=led_status['led1'], status2=led_status['led2'], status3=led_status['led3'], status4=led_status['led4'])

@app.route('/status', methods=['GET'])
def get_status():
    global led_status
    # Return the current LED statuses
    return jsonify(led_status)

@app.route('/toggle', methods=['POST'])
def toggle_led():
    global led_status
    data = request.get_json()

    # Update all LED statuses
    if 'led1' in data:
        led_status['led1'] = data['led1']
    if 'led2' in data:
        led_status['led2'] = data['led2']
    if 'led3' in data:
        led_status['led3'] = data['led3']
    if 'led4' in data:
        led_status['led4'] = data['led4']
    
    return jsonify({'status': 'success', 'message': 'LEDs updated'}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)