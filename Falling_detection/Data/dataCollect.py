from flask import Flask, request
import csv
import datetime
import os

app = Flask(__name__)
csv_filename = "falling.csv"

# Initialize CSV with headers if it doesn't exist
if not os.path.exists(csv_filename):
    with open(csv_filename, mode="w", newline='') as f:
        writer = csv.writer(f)
        writer.writerow([
            "timestamp",
            "accel_x", "accel_y", "accel_z",
            "gyro_x", "gyro_y", "gyro_z",
            "label"
        ])

@app.route("/data", methods=["POST"])
def log_data():
    data = request.get_json(force=True)

    if not isinstance(data, list):
        return "Expected a list of sensor samples", 400

    with open(csv_filename, mode="a", newline='') as csv_file:
        writer = csv.writer(csv_file)

        for sample in data:
            timestamp = datetime.datetime.now().isoformat()
            row = [
                timestamp,
                sample['accel_x'],
                sample['accel_y'],
                sample['accel_z'],
                sample['gyro_x'],
                sample['gyro_y'],
                sample['gyro_z'],
                sample['label']
            ]
            writer.writerow(row)
            print("Logged:", row)

    return "OK", 200

if __name__ == "__main__":
    app.run(host="192.168.1.214", port=5000)
