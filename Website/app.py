from flask import Flask, request, jsonify, render_template
from flask_cors import CORS
import datetime

app = Flask(__name__, static_folder='static', template_folder='templates')
CORS(app)

data_store = []

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/health')
def health():
    return render_template('health.html')

@app.route('/athlete')
def athlete():
    return render_template('athlete.html')

@app.route('/health_card')
def health_card():
    return render_template('health_card.html')

@app.route('/motion_intensity')
def tracker():
    return render_template('motion_intensity.html')

@app.route('/complete')
def complete():
    return render_template('complete.html')

@app.route('/api/upload', methods=['POST'])
def upload_data():
    try:
        content = request.get_json()
        content['timestamp'] = datetime.datetime.utcnow().isoformat()
        data_store.append(content)
        return jsonify({'status': 'success', 'data': content}), 200
    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)}), 400

@app.route('/api/data', methods=['GET'])
def get_data():
    return jsonify({'data': data_store})

@app.route('/api/clear', methods=['POST'])
def clear_data():
    data_store.clear()
    return jsonify({'status': 'cleared'})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
