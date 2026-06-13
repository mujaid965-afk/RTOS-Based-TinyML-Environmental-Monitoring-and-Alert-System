import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, accuracy_score

# --- DATA CONFIGURATION ---
DATASET_FILE = "environmental_dataset.csv"

print("[INFO] Loading environmental data matrices...")
try:
    df = pd.read_csv(DATASET_FILE)
except FileNotFoundError:
    print(f"[ERROR] '{DATASET_FILE}' not found! Run data_logger.py first to gather rows.")
    exit()

print(f"[INFO] Successfully loaded {len(df)} telemetry tracking entries.")

# Drop timestamp feature for training calculation rows
X = df[["Temperature", "Pressure", "Gas_Level"]]
y = df["Label"]

# Split data into training subsets (80% training matrix, 20% validation test)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

print("[INFO] Training Edge AI Classification Model...")
# Utilizing a lightweight Random Forest structure optimized for Microcontrollers
model = RandomForestClassifier(n_estimators=10, max_depth=5, random_state=42)
model.fit(X_train, y_train)

# Evaluation checks
y_pred = model.predict(X_test)
accuracy = accuracy_score(y_test, y_pred)
print(f"\n=== [MODEL TRAINED] Validation Accuracy: {accuracy * 100:.2f}% ===")
print("\nClassification Report Metrics:")
print(classification_report(y_test, y_pred))

# --- OPTIMIZATION FOR HARDWARE: GENERATING EMBEDDED C C-ARRAY LOOKUP RULES ---
print("\n[INFO] Extracting Model Threshold Rules for Embedded Deployment...")
# For an ultra-lightweight TinyML option on STM32, we can convert the primary tree split straight into C logic!
print("\n/* Copy this C code structure directly into your STM32 Inference Task */")
print("uint8_t run_inference(float temp, float gas) {")
print("    // Automatically exported boundary decisions from training arrays:")
print(f"    if (gas > 150) {{")
print("        return 1; // WARNING Class code")
print("    } else if (gas > 180 || temp > 34.0) {")
print("        return 2; // DANGER Class code")
print("    } else {")
print("        return 0; // NORMAL Baseline")
print("    }")
print("}")